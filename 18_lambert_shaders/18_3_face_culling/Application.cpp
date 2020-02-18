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

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
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
		attribute vec3 a_normal;

		uniform mat4 u_transform;
		uniform mat4 u_viewProjection;

		varying vec3 v_normal;
		varying vec4 v_pos;

		void main()
		{
			v_normal = (u_transform * vec4(a_normal, 0.0)).xyz;			
			v_pos = u_transform * vec4(a_position, 1.0);
			gl_Position = u_viewProjection * v_pos;
		}
	)";

	const char* fragment_shader_src = R"(
		uniform vec3 u_color;
		uniform vec3 u_light_dir;
		varying vec2 v_uv;
		varying vec3 v_normal;

		void main()
		{
			float diffuse = dot(normalize(v_normal), u_light_dir);
			diffuse = max(diffuse, 0.0);
			gl_FragColor = vec4(u_color * diffuse, 1.0);
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

	m_uniform_transform = glGetUniformLocation(m_program, "u_transform");
	m_uniform_viewProjection = glGetUniformLocation(m_program, "u_viewProjection");

	m_uniform_color = glGetUniformLocation(m_program, "u_color");
	m_uniform_texture = glGetUniformLocation(m_program, "u_texture");
	m_uniform_u_light_dir = glGetUniformLocation(m_program, "u_light_dir");

	glGenBuffers(1, &m_vertexBufferObject);
	glGenBuffers(1, &m_indexBufferObject);

	std::map<int, int> vertex_cache;
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
			int pos[4] = { 0, 0, 0 , 0 };
			int uv[4] = { 0, 0, 0, 0 };
			sscanf(str.c_str(), "f %d/%d %d/%d %d/%d %d/%d", pos, uv, pos + 1, uv + 1, pos + 2, uv + 2, pos + 3, uv + 3);
			int ind[4] = { 0, 0, 0, 0 };
			for (int i = 0; i < 4; ++i)
			{
				auto key = pos[i];
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
					v.normal = glm::vec3(0);
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

	m_indexSize = indices.size();

	for (int f = 0; f < m_indexSize / 3; ++f)
	{
		int i = indices[3 * f + 0];
		int j = indices[3 * f + 1];
		int k = indices[3 * f + 2];
		glm::vec3 p1 = vertices[i].pos;
		glm::vec3 p2 = vertices[j].pos;
		glm::vec3 p3 = vertices[k].pos;
		glm::vec3 a = p1 - p3;
		glm::vec3 b = p2 - p3;
		glm::vec3 c = glm::cross(a, b);
		vertices[i].normal += c;
		vertices[j].normal += c;
		vertices[k].normal += c;
	}

	for (int i = 0; i < vertices.size(); ++i)
	{
		vertices[i].normal = glm::normalize(vertices[i].normal);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_rotation = 0.0f;

	m_color = glm::vec3(0.5f);
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

	ImGui::End();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, m_width, m_height);

	float aspect = m_width / (float)m_height;

	float view_height = 2.2f;
	float view_width = aspect * view_height;

	glUseProgram(m_program);

	glm::mat4 projection = glm::ortho(-0.3f * aspect, 0.3f * aspect, -0.3f, 0.3f, -3.0f, 3.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(1.0), glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 viewProjection = projection * view;

	glUniformMatrix4fv(m_uniform_viewProjection, 1, GL_FALSE, &viewProjection[0][0]);

	m_rotation = 0.1f * time;
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0), m_rotation, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(m_uniform_transform, 1, GL_FALSE, &rotation[0][0]);
	
	glm::vec3 light_dir = glm::vec3(1.0, 0.2, -1.0);
	light_dir = glm::normalize(light_dir);
	glUniform3fv(m_uniform_u_light_dir, 1, &light_dir.x);
	glUniform3fv(m_uniform_color, 1, &m_color[0]);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);

	glEnableVertexAttribArray(m_attrib_pos);
	glVertexAttribPointer(m_attrib_pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(0));

	glEnableVertexAttribArray(m_attrib_normal);
	glVertexAttribPointer(m_attrib_normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(sizeof(glm::vec3)));

	glEnableVertexAttribArray(m_attrib_uv);
	glVertexAttribPointer(m_attrib_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), ToVoidPointer(2 * sizeof(glm::vec3)));

	glDrawElements(GL_TRIANGLES, m_indexSize, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(m_attrib_pos);
	glDisableVertexAttribArray(m_attrib_normal);
	glDisableVertexAttribArray(m_attrib_uv);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Application::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
}
