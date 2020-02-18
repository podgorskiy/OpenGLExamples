#pragma once
#include <glm/glm.hpp>

class Application
{
public:
	Application();
	~Application();

	void Draw(float time);
	void Resize(int width, int height);

private:
	unsigned int m_program;

	unsigned int u_modelView;
	unsigned int m_uniform_color;
	
	unsigned int m_attrib_pos;
	unsigned int m_attrib_normal;
	unsigned int m_attrib_uv;
	unsigned int m_attrib_color;

	unsigned int u_projection;
	unsigned int m_uniform_texture;
	unsigned int m_uniform_u_light_dir;
	unsigned int m_vertexBufferObject;
	unsigned int m_indexBufferObject;
	unsigned int m_indexSize;

	unsigned int u_ambientProduct;
	unsigned int u_diffuseProduct;
	unsigned int u_diffuseProduct2;
	unsigned int u_specularProduct;
	unsigned int u_shininess;
	unsigned int u_lightPos;

	glm::vec3 m_ambientProduct = glm::vec3(0.0);
	glm::vec3 m_diffuseProduct = glm::vec3(0.2);
	glm::vec3 m_diffuseProduct2 = glm::vec3(0.3);
	glm::vec3 m_specularProduct = glm::vec3(0.3);
	float m_shininess = 30.0;

	glm::vec3 m_lightPos;
	float m_lightrotation = 0.0f;

	glm::vec3 m_color;

	unsigned int u_gamma;
	float m_gamma = 2.2f;

	int m_width;
	int m_height;
	float m_rotation;
};
