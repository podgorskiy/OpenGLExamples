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

		uniform mat4 u_modelView;
		uniform mat4 u_projection;
		uniform vec3 u_lightPos;

		varying vec3 v_L;
		varying vec3 v_E;
		varying vec3 v_N;

		void main()
		{	
			vec4 pos = u_modelView * vec4(a_position, 1.0);
			v_L = u_lightPos.xyz - pos.xyz;
			v_E = -pos.xyz;
			v_N = (u_modelView * vec4(a_normal, 0.0)).xyz;

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

		varying vec3 v_L;
		varying vec3 v_E;
		varying vec3 v_N;

		void main()
		{
			vec3 L = normalize(v_L);
			vec3 E = normalize(v_E);
			vec3 N = normalize(v_N);
			
			vec3 H = normalize(L + E);

			float Kd = max(dot(L, N), 0.0);
			vec3 diffuse = Kd * u_diffuseProduct;

			vec3 diffuse2 = (0.5 + 0.5 * N.y) * u_diffuseProduct2;

			float Ks = pow(max(dot(N, H), 0.0), u_shininess);
			vec3 specular = Ks * u_specularProduct;

			if (dot(L, N) < 0.0)
				specular = vec3(0.0, 0.0, 0.0);

			gl_FragColor = vec4(pow(u_ambientProduct + diffuse + diffuse2 + specular, vec3(1.0/u_gamma)), 1.0);
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

	u_ambientProduct = glGetUniformLocation(m_program, "u_ambientProduct");
	u_diffuseProduct = glGetUniformLocation(m_program, "u_diffuseProduct");
	u_diffuseProduct2 = glGetUniformLocation(m_program, "u_diffuseProduct2");
	u_specularProduct = glGetUniformLocation(m_program, "u_specularProduct");
	u_shininess = glGetUniformLocation(m_program, "u_shininess");
	u_lightPos = glGetUniformLocation(m_program, "u_lightPos");
	m_uniform_texture = glGetUniformLocation(m_program, "u_texture");
	u_gamma = glGetUniformLocation(m_program, "u_gamma");


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

	glUseProgram(m_program);

	glm::mat4 projection = glm::ortho(-0.3f * aspect, 0.3f * aspect, -0.3f, 0.3f, -3.0f, 3.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(1.0), glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));

	glUniformMatrix4fv(u_projection, 1, GL_FALSE, &projection[0][0]);

	m_rotation = 0.1f * time;
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0), m_rotation, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 modelView = view * rotation;
	glUniformMatrix4fv(u_modelView, 1, GL_FALSE, &modelView[0][0]);
	
	m_lightPos = glm::rotate(glm::mat4(1.0), m_lightrotation, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(glm::vec3(1.0, 1.0, -1.0), 0);

	auto ambient = glm::pow(m_ambientProduct, glm::vec3(m_gamma));
	auto diffuse = glm::pow(m_diffuseProduct, glm::vec3(m_gamma));
	auto diffuse2 = glm::pow(m_diffuseProduct2, glm::vec3(m_gamma));
	auto specular = glm::pow(m_specularProduct, glm::vec3(m_gamma));
	glUniform3fv(u_ambientProduct, 1, (float*)&ambient);
	glUniform3fv(u_diffuseProduct, 1, (float*)&diffuse);
	glUniform3fv(u_diffuseProduct2, 1, (float*)&diffuse2);
	glUniform3fv(u_specularProduct, 1, (float*)&specular);
	
	glUniform1fv(u_shininess, 1, &m_shininess);
	glUniform3fv(u_lightPos, 1, &m_lightPos[0]);
	glUniform1fv(u_gamma, 1, &m_gamma);

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
