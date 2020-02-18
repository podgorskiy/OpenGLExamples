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

		uniform mat4 u_modelView;
		uniform mat4 u_projection;
		uniform vec3 u_lightPos;

		varying vec2 v_uv;
		varying vec3 v_normal;

		varying vec3 v_L;
		varying vec3 v_E;
		varying vec3 v_N;

		void main()
		{	
			vec4 pos = u_modelView * vec4(a_position, 1.0);
			v_L = u_lightPos.xyz - pos.xyz;
			v_E = -pos.xyz;
			v_N = (u_modelView * vec4(a_normal, 0.0)).xyz;
			v_uv = a_uv;

			gl_Position = u_projection * pos;
		}
	)";

	const char* fragment_shader_src = R"(
		// light and material properties
		uniform float u_gamma;

		varying vec2 v_uv;
		varying vec3 v_L;
		varying vec3 v_E;
		varying vec3 v_N;

		uniform sampler2D u_texture;
		uniform samplerCube u_cubemap;

		uniform mat4 u_modelView;

		void main()
		{
			vec3 E = normalize(v_E);
			vec3 N = normalize(v_N);

			vec3 reflected = reflect(E, N);
			reflected = vec3(vec4(reflected, 0.0) * u_modelView);

			vec3 frag_colour = textureCube(u_cubemap, reflected).xyz;

			gl_FragColor = vec4(frag_colour, 1.0);
		}
	)";

	int vertex_shader_handle = CompileShader(vertex_shader_src, GL_VERTEX_SHADER);
	int fragment_shader_handle = CompileShader(fragment_shader_src, GL_FRAGMENT_SHADER);

	m_program = glCreateProgram();

	glAttachShader(m_program, vertex_shader_handle);
	glAttachShader(m_program, fragment_shader_handle);

	glLinkProgram(m_program);

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

	u_modelView = glGetUniformLocation(m_program, "u_modelView");
	u_projection = glGetUniformLocation(m_program, "u_projection");

	m_obj.Load("LeePerrySmith.obj");

	m_rotation = 0.0f;

	m_color = glm::vec3(0.5f);

	cubemap = Texture::LoadTexture("cubemap.pvr");
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

	float left = -view_width / 2.0f;
	float right = view_width / 2.0f;
	float top = view_height / 2.0f;
	float bottom = - view_height / 2.0f;

	float camera_rotation = -time * 0.1;

	glm::mat4 projection = glm::ortho(-0.3f * aspect, 0.3f * aspect, -0.3f, 0.3f, -3.0f, 3.0f);
	glm::vec3 camera_pos = glm::vec3(1.0, 0.5, 1.0);
	camera_pos = glm::rotate(glm::mat4(1.0), camera_rotation, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(camera_pos, 1.0);
	glm::mat4 view = glm::lookAt(camera_pos, glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 modelView = view;

	glUseProgram(m_program);
	cubemap->BindCube(0);

	glUniformMatrix4fv(u_projection, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(u_modelView, 1, GL_FALSE, &view[0][0]);
	
	m_lightPos = glm::rotate(glm::mat4(1.0), m_lightrotation, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(glm::vec3(1.0, 1.0, -1.0), 0);

	auto ap = glm::pow(m_ambientProduct, glm::vec3(m_gamma));
	glUniform3fv(u_ambientProduct, 1, (float*)&ap);
	auto dp = glm::pow(m_diffuseProduct, glm::vec3(m_gamma));
	glUniform3fv(u_diffuseProduct, 1, (float*)&dp);
	auto dp2 = glm::pow(m_diffuseProduct2, glm::vec3(m_gamma));
	glUniform3fv(u_diffuseProduct2, 1, (float*)&dp2);
	auto sp = glm::pow(m_specularProduct, glm::vec3(m_gamma));
	glUniform3fv(u_specularProduct, 1, (float*)&sp);
	glUniform1fv(u_shininess, 1, &m_shininess);
	glUniform3fv(u_lightPos, 1, &m_lightPos[0]);
	glUniform1fv(u_gamma, 1, &m_gamma);

	glUniform1i(m_uniform_cubemap, 0);

	m_obj.Bind();

	glEnableVertexAttribArray(m_attrib_pos);
	glVertexAttribPointer(m_attrib_pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(0));

	glEnableVertexAttribArray(m_attrib_normal);
	glVertexAttribPointer(m_attrib_normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(sizeof(glm::vec3)));

	glEnableVertexAttribArray(m_attrib_uv);
	glVertexAttribPointer(m_attrib_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(2 * sizeof(glm::vec3)));

	m_obj.Draw();

	glDisableVertexAttribArray(m_attrib_pos);
	glDisableVertexAttribArray(m_attrib_normal);
	glDisableVertexAttribArray(m_attrib_uv);

	m_obj.UnBind();
}

void Application::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
}
