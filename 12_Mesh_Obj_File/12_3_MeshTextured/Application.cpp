#include "Application.h"
#include <GL/gl3w.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

struct Texture
{
	glm::vec<3, char>* buff;
	int width;
	int height;
};

Texture loadtexture(const char* path) 
{
	Texture t;
	std::ifstream  tgafile(path, std::ios::binary);
	char buff[18];
	tgafile.read(buff, 18);
	t.width = *((short*)&buff[0xc]);
	t.height = *((short*)&buff[0xe]);
	int size = 3 * sizeof(char) * t.width * t.height;
	t.buff = new glm::vec<3, char>[t.width * t.height];
	tgafile.read((char*)t.buff, size);
	return t;
}

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
	glm::vec2 uv;
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
		attribute vec2 a_uv;

		uniform vec2 u_rotation;

		varying vec2 v_uv;

		void main()
		{
			vec3 pos = a_position;
			pos.xz = mat2(u_rotation, -u_rotation.y, u_rotation.x) * pos.xz;

			gl_Position = vec4(pos * 3.0, 1.0);

			v_uv = a_uv;
		}
	)";

	const char* fragment_shader_src = R"(
		uniform sampler2D u_texture;
		varying vec2 v_uv;

		void main()
		{
			vec3 color = texture2D(u_texture, v_uv).bgr;
			gl_FragColor = vec4(color, 1.0);
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
	m_attrib_uv = glGetAttribLocation(m_program, "a_uv");

	m_uniform_rotation = glGetUniformLocation(m_program, "u_rotation");
	m_uniform_texture = glGetUniformLocation(m_program, "u_texture");

	glGenBuffers(1, &m_vertexBufferObject);
	glGenBuffers(1, &m_indexBufferObject);

	std::map<std::pair<int, int>, int> vertex_cache;
	std::vector<Vertex> vertices;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> uv_coordinates;
	std::vector<int> indices;

	std::ifstream file("LeePerrySmith.obj");
	std::string str;
	Vertex v;
	glm::vec3 pos;
	glm::vec3 uv;
	int current_index = 0;
	while (std::getline(file, str))
	{
		if (strncmp(str.c_str(), "v ", 2) == 0)
		{
			sscanf(str.c_str(), "v %f %f %f", &pos.x, &pos.y, &pos.z);
			positions.push_back(pos);
		}
		else if (strncmp(str.c_str(), "vt ", 3) == 0)
		{
			sscanf(str.c_str(), "vt %f %f", &uv.x, &uv.y);
			uv_coordinates.push_back(uv);
		}
		else if (strncmp(str.c_str(), "f ", 2) == 0)
		{
			int pos[4] = {0, 0, 0 , 0 };
			int uv[4] = { 0, 0, 0, 0 };
			sscanf(str.c_str(), "f %d/%d %d/%d %d/%d %d/%d", pos, uv, pos + 1, uv + 1, pos + 2, uv + 2, pos + 3, uv + 3);
			int ind[4] = { 0, 0, 0, 0 };
			for (int i = 0; i < 4; ++i)
			{
				auto key = std::make_pair(pos[i], uv[i]);
				auto it = vertex_cache.find(key);
				if (it != vertex_cache.end())
				{
					ind[i] = it->second;
				}
				else
				{
					vertex_cache[key] = current_index;
					ind[i] = current_index;
					++current_index;
					Vertex v;
					v.pos = positions[pos[i] - 1];
					v.uv = uv_coordinates[uv[i] - 1];
					vertices.push_back(v);
				}
			}
			indices.push_back(ind[0]);
			indices.push_back(ind[1]);
			indices.push_back(ind[2]);
			indices.push_back(ind[0]);
			indices.push_back(ind[2]);
			indices.push_back(ind[3]);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_indexSize = indices.size();

	Texture t = loadtexture("albido.tga");

	glGenTextures(1, &m_textureHandle);
	glBindTexture(GL_TEXTURE_2D, m_textureHandle);

	int size = t.width * t.height * 3;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, t.width, t.height, 0, GL_RGB, GL_UNSIGNED_BYTE, t.buff);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glUseProgram(m_program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureHandle);

	float angle = 0.5 * time;
	glm::vec2 rotation = glm::vec2(cos(angle), sin(angle));

	glUniform2fv(m_uniform_rotation, 1, &rotation.x);

	glUniform1i(m_uniform_texture, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);

	glEnableVertexAttribArray(m_attrib_pos);
	glVertexAttribPointer(m_attrib_pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(0));

	glEnableVertexAttribArray(m_attrib_uv);
	glVertexAttribPointer(m_attrib_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(sizeof(glm::vec3)));

	glDrawElements(GL_TRIANGLES, m_indexSize, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(m_attrib_pos);
	glDisableVertexAttribArray(m_attrib_uv);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
