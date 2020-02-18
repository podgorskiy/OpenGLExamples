#include "Application.h"
#include <GL/gl3w.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <vector>

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

struct Vertex
{
	glm::vec3 pos;
};

Application::Application()
{
	gl3wInit();

	const char* OpenGLversion = (const char*)glGetString(GL_VERSION);
	const char* GLSLversion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

	printf("OpenGL %s GLSL: %s", OpenGLversion, GLSLversion);


	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	const char* vertex_shader_src = R"(
		attribute vec3 a_position;

		uniform vec2 u_rotation;

		void main()
		{
			vec3 pos = a_position;
			pos.xz = mat2(u_rotation, -u_rotation.y, u_rotation.x) * pos.xz;

			gl_Position = vec4(pos * 3.0, 1.0);
		}
	)";

	const char* fragment_shader_src = R"(
		void main()
		{
			gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
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
	m_attrib_color = glGetAttribLocation(m_program, "a_color");

	m_uniform_rotation = glGetUniformLocation(m_program, "u_rotation");

	glGenBuffers(1, &m_vertexBufferObject);
	glGenBuffers(1, &m_indexBufferObject);

	std::vector<Vertex> vertices;
	std::vector<int> indices;
	std::ifstream file("LeePerrySmith.obj");
	std::string str;
	Vertex v;
	v.pos = glm::vec3();
	while (std::getline(file, str))
	{
		if (strncmp(str.c_str(), "v ", 2) == 0)
		{
			sscanf(str.c_str(), "v %f %f %f", &v.pos.x, &v.pos.y, &v.pos.z);
			vertices.push_back(v);
		}
		else if (strncmp(str.c_str(), "f ", 2) == 0)
		{
			int tmp;
			int a, b, c, d;
			sscanf(str.c_str(), "f %d/%d %d/%d %d/%d %d/%d", &a, &tmp, &b, &tmp, &c, &tmp, &d, &tmp);
			indices.push_back(a - 1);
			indices.push_back(b - 1);
			indices.push_back(c - 1);
			indices.push_back(a - 1);
			indices.push_back(c - 1);
			indices.push_back(d - 1);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_indexSize = indices.size();
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
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_CULL_FACE);

	glUseProgram(m_program);

	float angle = 0.5 * time;
	glm::vec2 rotation = glm::vec2(cos(angle), sin(angle));

	// As two floats
	glUniform2f(m_uniform_rotation, rotation.x, rotation.y);

	// Or by pointer
	glUniform2fv(m_uniform_rotation, 1, &rotation.x);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);

	glEnableVertexAttribArray(m_attrib_pos);
	glVertexAttribPointer(m_attrib_pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(0));
	
	glDrawElements(GL_TRIANGLES, m_indexSize, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(m_attrib_pos);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//glEnableVertexAttribArray(m_attrib_color);
	//glVertexAttribPointer(m_attrib_color, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) + sizeof(glm::vec3), ToVoidPointer(sizeof(glm::vec2)));
	//glDisableVertexAttribArray(m_attrib_color);
}
