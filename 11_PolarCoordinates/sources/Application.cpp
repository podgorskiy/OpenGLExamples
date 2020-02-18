#include "Application.h"
#include <GL/gl3w.h>
#include <stdio.h>
#include <glm/glm.hpp>

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
		attribute vec2 a_position;

		uniform float u_rotation;

		void main()
		{
			float phi = u_rotation + a_position.y;
			vec2 pos = vec2(cos(phi), sin(phi)) * a_position.x;

			gl_Position = vec4(pos * 0.4, 0.0, 1.0);
		}
	)";

	const char* fragment_shader_src = R"(

		void main()
		{
			gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
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

	glm::vec2 vertices[12];
	vertices[0] = glm::vec2(0.0, 0.0);
	vertices[1] = glm::vec2(1.0, 0.0);
	vertices[2] = glm::vec2(1.1, 0.1 * 6.28f);
	vertices[3] = glm::vec2(1.3, 0.2 * 6.28f);
	vertices[4] = glm::vec2(1.3, 0.3 * 6.28f);
	vertices[5] = glm::vec2(1.1, 0.4 * 6.28f);
	vertices[6] = glm::vec2(1.0, 0.5 * 6.28f);
	vertices[7] = glm::vec2(1.1, 0.6 * 6.28f);
	vertices[8] = glm::vec2(1.3, 0.7 * 6.28f);
	vertices[9] = glm::vec2(1.3, 0.8 * 6.28f);
	vertices[10] = glm::vec2(1.1, 0.9 * 6.28f);
	vertices[11] = glm::vec2(1.0, 1.0 * 6.28f);

	short indices[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(glm::vec2), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * sizeof(short), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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

	glUseProgram(m_program);

	float angle = 0.5 * time;

	glUniform1f(m_uniform_rotation, angle);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);

	glEnableVertexAttribArray(m_attrib_pos);
	glVertexAttribPointer(m_attrib_pos, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), ToVoidPointer(0));

	glDrawElements(GL_TRIANGLE_FAN, 12, GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(m_attrib_pos);
	glDisableVertexAttribArray(m_attrib_color);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
