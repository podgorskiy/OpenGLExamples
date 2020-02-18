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
#include "examples/opengl3_example/imgui_impl_glfw_gl3.h"
#include "Texture.h"

TexturePtr cubemap;
TexturePtr texture;
TexturePtr normal_map;

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

		varying vec3 v_E;
		varying vec3 v_E_world;
		varying vec3 v_N;
		varying vec3 v_T;

		void main()
		{	
			vec4 pos = u_modelView * vec4(a_position, 1.0);
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

		varying vec2 v_uv;
		varying vec3 v_E;
		varying vec3 v_N;
		varying vec3 v_T;

		uniform sampler2D u_texture;
		uniform sampler2D u_normalmap;
		uniform samplerCube u_cubemap;

		uniform mat4 u_modelView;

		void main()
		{
			vec3 E = normalize(v_E);
			vec3 N = normalize(v_N);
			vec3 T1 = normalize(v_T);
			T1 = T1 - N * dot(T1, N);
			vec3 T2 = cross(N, T1);

			mat3 tangent_space = mat3(T2, T1, N);
			
			vec3 normal_tangent = 2.0 * (texture2D(u_normalmap, v_uv).xyz - vec3(0.5));
			vec3 normal = normalize(tangent_space * normal_tangent);

			vec3 reflected = reflect(E, normal);
			reflected = vec3(vec4(reflected, 0.0) * u_modelView);

			vec3 frag_colour = textureCube(u_cubemap, reflected).xyz;

			gl_FragColor = vec4(frag_colour, 1.0);
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
	u_model = glGetUniformLocation(m_program, "u_model");
	u_camera_pos = glGetUniformLocation(m_program, "u_camera_pos");
	m_uniform_cubemap = glGetUniformLocation(m_program, "u_cubemap");
	m_uniform_normalmap = glGetUniformLocation(m_program, "u_normalmap");

	u_modelView_skybox = glGetUniformLocation(m_program_skybox, "u_modelView");
	u_projection_skybox = glGetUniformLocation(m_program_skybox, "u_projection");
	u_model_skybox = glGetUniformLocation(m_program_skybox, "u_model");
	u_camera_pos_skybox = glGetUniformLocation(m_program_skybox, "u_camera_pos");
	m_uniform_cubemap_skybox = glGetUniformLocation(m_program_skybox, "u_cubemap");
	m_obj.Load("LeePerrySmith.obj");

	m_rotation = 0.0f;

	cubemap = Texture::LoadTexture("cubemap.pvr");
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
	//texture->Bind(0);
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
	cubemap->BindCube(0);
	normal_map->Bind(1);

	glUniformMatrix4fv(u_projection, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(u_modelView, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);
	glUniform3fv(u_camera_pos, 1, &camera_pos[0]);

	glUniform1i(m_uniform_cubemap, 0);
	glUniform1i(m_uniform_normalmap, 1);
	
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
}

void Application::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
}
