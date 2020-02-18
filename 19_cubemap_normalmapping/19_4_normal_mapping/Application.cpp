#include "Application.h"
#include <GL/gl3w.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui.h"
#include "examples/imgui_impl_opengl3.h"

GLuint CompileShader(const char* src, GLint type)
{
	GLuint shader = glCreateShader(type);

	glShaderSource(shader, 1, &src, NULL);

	glCompileShader(shader);
	GLint compiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	GLint infoLen = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

	if (infoLen > 1)
	{
		printf("%s during shader compilation.\n ", compiled == GL_TRUE ? "Warning" : "Error");
		char* buf = new char[infoLen];
		glGetShaderInfoLog(shader, infoLen, NULL, buf);
		printf("Compilation log: %s\n", buf);
		delete[] buf;
	}
	
	return shader;
}

Application::Application()
{
	gl3wInit();

	const char* OpenGLversion = (const char*)glGetString(GL_VERSION);
	const char* GLSLversion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

	printf("OpenGL %s GLSL: %s", OpenGLversion, GLSLversion);


	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);


	const char* vertex_shader_src = R"(
		attribute vec3 a_position;
		attribute vec3 a_normal;
		attribute vec2 a_uv;
		attribute vec3 a_tangent;

		uniform mat4 u_modelView;
		uniform mat4 u_model;
		uniform vec3 u_camera_pos;
		uniform mat4 u_projection;
		uniform vec3 u_lightPos;

		varying vec2 v_uv;
		varying vec3 v_normal;

		varying vec3 v_L;
		varying vec3 v_E;
		varying vec3 v_E_world;
		varying vec3 v_N;
		varying vec3 v_T;

		void main()
		{	
			vec4 pos = u_modelView * vec4(a_position, 1.0);
			v_L = u_lightPos.xyz - pos.xyz;
			v_E = -pos.xyz;
			v_N = (u_modelView * vec4(a_normal, 0.0)).xyz;
			v_T = (u_modelView * vec4(a_tangent, 0.0)).xyz;
			v_uv = vec2(a_uv.x, 1.0 - a_uv.y);

			vec4 pos_world = u_model * vec4(a_position, 1.0);
			v_E_world = pos_world.xyz - u_camera_pos;

			gl_Position = u_projection * pos;
		}
	)";

	const char* fragment_shader_src = R"(
		// light and material properties
		uniform vec3 u_ambientProduct;
		uniform vec3 u_diffuseProduct;
		uniform vec3 u_diffuseProduct2;
		uniform vec3 u_specularProduct;
		uniform float u_shininess;

		uniform float u_gamma;

		varying vec2 v_uv;
		varying vec3 v_L;
		varying vec3 v_E;
		varying vec3 v_N;
		varying vec3 v_T;

		uniform sampler2D u_texture;
		uniform sampler2D u_normalmap;
		uniform samplerCube u_cubemap;

		uniform mat4 u_modelView;

		float fresnelReflectance(vec3 H, vec3 V, float F0)
		{
			float base = 1.0 - dot(V, H);
			float exponential = pow(base, 5.0);
			return exponential + F0 * (1.0 - exponential);
		}

		void main()
		{
			vec3 L = normalize(v_L);
			vec3 E = normalize(v_E);
			vec3 N = normalize(v_N);

			vec3 T1 = normalize(v_T);
			T1 = T1 - N * dot(T1, N);
			vec3 T2 = cross(N, T1);

			mat3 tangent_space = mat3(T2, T1, N);

			vec3 normal_tangent = 2.0 * (texture2D(u_normalmap, v_uv).xyz - vec3(0.5));
			vec3 normal = normalize(tangent_space * normal_tangent);
			N = normal;

			vec3 H = normalize(L + E);

			vec3 reflected = reflect(E, N);
			reflected = normalize(vec3(vec4(reflected, 0.0) * u_modelView));

			vec3 env_colour = pow(textureCube(u_cubemap, reflected).rgb, vec3(u_gamma));

			float Kd = max(dot(L, N), 0.0);
			vec3 diffuse = Kd * u_diffuseProduct;

			vec3 diffuse2 = (0.5 + 0.5 * N.y) * u_diffuseProduct2;

			float Ks = pow(max(dot(N, H), 0.0), u_shininess);
			vec3 specular = Ks * u_specularProduct;

			if (dot(L, N) < 0.0)
				specular = vec3(0.0, 0.0, 0.0);

			vec3 albido = pow(texture2D(u_texture, v_uv).rgb, vec3(u_gamma));

			float f = fresnelReflectance(N, E, 0.0028);

			vec3 color = (u_ambientProduct + diffuse + diffuse2) * albido + env_colour * f;

			gl_FragColor = vec4(pow(color, vec3(1.0/u_gamma)), 1.0);
		}
	)";

	const char* skybox_fragment_shader_src = R"(
		// light and material properties
		varying vec3 v_E_world;

		uniform samplerCube u_cubemap;

		void main()
		{
			vec3 E = normalize(v_E_world);
			vec3 frag_colour = textureCube(u_cubemap, -E).xyz;
			gl_FragColor = vec4(frag_colour, 1.0);
		}
	)";

	int vertex_shader_handle = CompileShader(vertex_shader_src, GL_VERTEX_SHADER);
	int fragment_shader_handle = CompileShader(fragment_shader_src, GL_FRAGMENT_SHADER);
	int skybox_fragment_shaderr_handle = CompileShader(skybox_fragment_shader_src, GL_FRAGMENT_SHADER);

	m_program = glCreateProgram();
	m_program_skybox = glCreateProgram();

	glAttachShader(m_program, vertex_shader_handle);
	glAttachShader(m_program, fragment_shader_handle);

	glLinkProgram(m_program);

	glAttachShader(m_program_skybox, vertex_shader_handle);
	glAttachShader(m_program_skybox, skybox_fragment_shaderr_handle);

	glLinkProgram(m_program_skybox);

	int linked;
	glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		GLint infoLen = 0;
		glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1)
		{
			char* buf = new char[infoLen];
			glGetProgramInfoLog(m_program, infoLen, NULL, buf);
			printf("Linking error: \n%s\n", buf);
			delete[] buf;
		}
	}

	glDetachShader(m_program, vertex_shader_handle);
	glDetachShader(m_program, fragment_shader_handle);

	glDeleteShader(vertex_shader_handle);
	glDeleteShader(fragment_shader_handle);

	m_attrib_pos = glGetAttribLocation(m_program, "a_position");
	m_attrib_normal = glGetAttribLocation(m_program, "a_normal");
	m_attrib_uv = glGetAttribLocation(m_program, "a_uv");
	m_attrib_tangent = glGetAttribLocation(m_program, "a_tangent");

	u_modelView = glGetUniformLocation(m_program, "u_modelView");
	u_projection = glGetUniformLocation(m_program, "u_projection");

	u_modelView = glGetUniformLocation(m_program, "u_modelView");
	u_projection = glGetUniformLocation(m_program, "u_projection");

	u_ambientProduct = glGetUniformLocation(m_program, "u_ambientProduct");
	u_diffuseProduct = glGetUniformLocation(m_program, "u_diffuseProduct");
	u_diffuseProduct2 = glGetUniformLocation(m_program, "u_diffuseProduct2");
	u_specularProduct = glGetUniformLocation(m_program, "u_specularProduct");
	u_shininess = glGetUniformLocation(m_program, "u_shininess");
	u_lightPos = glGetUniformLocation(m_program, "u_lightPos");

	m_uniform_cubemap = glGetUniformLocation(m_program, "u_cubemap");
	m_uniform_texture = glGetUniformLocation(m_program, "u_texture");
	m_uniform_normalmap = glGetUniformLocation(m_program, "u_normalmap");

	u_gamma = glGetUniformLocation(m_program, "u_gamma");

	u_modelView_skybox = glGetUniformLocation(m_program_skybox, "u_modelView");
	u_projection_skybox = glGetUniformLocation(m_program_skybox, "u_projection");
	u_model_skybox = glGetUniformLocation(m_program_skybox, "u_model");
	u_camera_pos_skybox = glGetUniformLocation(m_program_skybox, "u_camera_pos");
	m_uniform_cubemap_skybox = glGetUniformLocation(m_program_skybox, "u_cubemap");

	m_obj.Load("LeePerrySmith.obj");

	m_rotation = 0.0f;

	cubemap = Texture::LoadTexture("cubemap.pvr");
	texture = Texture::LoadTexture("albido.pvr");
	normal_map = Texture::LoadTexture("normal.pvr");

	m_skybox.MakeBox();
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}


Application::~Application()
{
	glDeleteProgram(m_program);
}

inline void* ToVoidPointer(int offset)
{
	size_t offset_ = static_cast<size_t>(offset);
	return reinterpret_cast<void*>(offset_);
}

void Application::Draw(float time)
{
	ImGui::Begin("IntroductionToComputerGraphics");
	ImGui::Text("Face culling.");

	static bool cull = false;
	ImGui::Checkbox("Do culling", &cull);
	if (cull)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	static bool front = false;
	ImGui::Checkbox("Cull front/back", &front);
	if (front)
	{
		glCullFace(GL_FRONT);
	}
	else
	{
		glCullFace(GL_BACK);
	}

	ImGui::ColorEdit3("Ambient", (float*)&m_ambientProduct);
	ImGui::ColorEdit3("Diffuse", (float*)&m_diffuseProduct);
	ImGui::ColorEdit3("Diffuse2", (float*)&m_diffuseProduct2);
	ImGui::ColorEdit3("Specular", (float*)&m_specularProduct);
	ImGui::SliderFloat("Shininess", (float*)&m_shininess, 0.0f, 500.0f);

	ImGui::SliderFloat("LightRotation", (float*)&m_lightrotation, 0.0f, 6.28f);
	ImGui::SliderFloat("Gamma", (float*)&m_gamma, 1.0f, 3.0f);

	ImGui::End();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, m_width, m_height);

	float aspect = m_width / (float)m_height;

	float view_height = 2.2f;
	float view_width = aspect * view_height;

	float camera_rotation = -time * 0.1;

	glm::mat4 projection = glm::perspectiveFov(1.0f, view_width, view_height, 0.01f, 100.0f);
	glm::vec3 camera_pos = glm::vec3(1.0, 0.5, 1.0) * 0.3f;
	camera_pos = glm::rotate(glm::mat4(1.0), camera_rotation, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(camera_pos, 1.0);
	glm::mat4 view = glm::lookAt(camera_pos, glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 modelView = view * model;

	glUseProgram(m_program);
	texture->Bind(0);
	normal_map->Bind(1);
	cubemap->BindCube(2);

	glUniformMatrix4fv(u_projection, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(u_modelView, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);
	glUniform3fv(u_camera_pos, 1, &camera_pos[0]);
	
	m_lightPos = glm::rotate(glm::mat4(1.0), m_lightrotation, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(glm::vec3(1.0, 1.0, -1.0), 0);

	auto ap = glm::pow(m_ambientProduct, glm::vec3(m_gamma));
	glUniform3fv(u_ambientProduct, 1, (float*)&ap);
	auto dp = glm::pow(m_diffuseProduct, glm::vec3(m_gamma));
	glUniform3fv(u_diffuseProduct, 1, (float*)&dp);
	auto dp2 = glm::pow(m_diffuseProduct2, glm::vec3(m_gamma));
	glUniform3fv(u_diffuseProduct2, 1, (float*)&dp2);
	auto sp = glm::pow(m_specularProduct, glm::vec3(m_gamma));
	// glUniform3fv(u_specularProduct, 1, (float*)&sp);
	// glUniform1fv(u_shininess, 1, &m_shininess);
	glUniform3fv(u_lightPos, 1, &m_lightPos[0]);
	glUniform1fv(u_gamma, 1, &m_gamma);

	assert(m_uniform_cubemap < 1000);
	assert(m_uniform_texture < 1000);
	assert(m_uniform_normalmap < 1000);

	glUniform1i(m_uniform_texture, 0);
	glUniform1i(m_uniform_normalmap, 1);
	glUniform1i(m_uniform_cubemap, 2);

	m_obj.Bind();

	glEnableVertexAttribArray(m_attrib_pos);
	glVertexAttribPointer(m_attrib_pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(0));

	glEnableVertexAttribArray(m_attrib_normal);
	glVertexAttribPointer(m_attrib_normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(sizeof(glm::vec3)));

	glEnableVertexAttribArray(m_attrib_uv);
	glVertexAttribPointer(m_attrib_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(2 * sizeof(glm::vec3)));

	glEnableVertexAttribArray(m_attrib_tangent);
	glVertexAttribPointer(m_attrib_tangent, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));

	m_obj.Draw();

	glDisableVertexAttribArray(m_attrib_pos);
	glDisableVertexAttribArray(m_attrib_normal);
	glDisableVertexAttribArray(m_attrib_uv);
	glDisableVertexAttribArray(m_attrib_tangent);

	m_obj.UnBind();

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, 0);

	//19_2_sky_box

	model = glm::scale(glm::mat4(1.0), glm::vec3(2.0));
	modelView = view * model;

	glUseProgram(m_program_skybox);
	cubemap->BindCube(0);

	glUniformMatrix4fv(u_projection_skybox, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(u_modelView_skybox, 1, GL_FALSE, &modelView[0][0]);
	glUniformMatrix4fv(u_model_skybox, 1, GL_FALSE, &model[0][0]);
	glUniform3fv(u_camera_pos_skybox, 1, &camera_pos[0]);

	glUniform1i(m_uniform_cubemap_skybox, 0);

	m_skybox.Bind();

	glEnableVertexAttribArray(m_attrib_pos);
	glVertexAttribPointer(m_attrib_pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(0));

	m_skybox.Draw();

	glDisableVertexAttribArray(m_attrib_pos);

	m_skybox.UnBind();

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
}

void Application::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
}
