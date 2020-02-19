#pragma once
#include "Object.h"
#include "Texture.h"
#include <glm/glm.hpp>

class Application
{
public:
	Application();
	~Application();

	void Draw(float time);
	void Resize(int width, int height);

	void OnScroll(float y)
	{
		if (y > 0)
		{
			m_camera_distance *= 1.1;
		}
		else
		{
			m_camera_distance /= 1.1;
		}
	}
private:
	unsigned int m_program;
	unsigned int m_program_skybox;

	unsigned int u_modelView;
	unsigned int u_modelView_skybox;
	unsigned int u_model;
	unsigned int u_model_skybox;
	
	unsigned int m_attrib_pos;
	unsigned int m_attrib_normal;
	unsigned int m_attrib_uv;
	unsigned int m_attrib_tangent;
	unsigned int m_attrib_color;

	unsigned int u_mode;
	unsigned int u_projection;
	unsigned int u_projection_skybox;
	unsigned int m_uniform_cubemap;
	unsigned int m_uniform_texture;
	unsigned int m_uniform_normalmap;
	unsigned int m_uniform_specmap;
	unsigned int m_uniform_ibl_brdf_map;
	unsigned int m_uniform_cubemap_skybox;
	unsigned int m_vertexBufferObject;
	unsigned int m_indexBufferObject;
	unsigned int m_indexSize;
	unsigned int u_camera_pos;
	unsigned int u_camera_pos_skybox;

	unsigned int u_ambientProduct;
	unsigned int u_diffuseProduct;
	unsigned int u_diffuseProduct2;
	unsigned int u_specularProduct;
	unsigned int u_shininess;
	unsigned int u_roughness;
	unsigned int u_lightPos;

	TexturePtr cubemap;
	TexturePtr normal_map;
	TexturePtr specular_map;
	TexturePtr texture;
	TexturePtr ibl_brdf_map;

	glm::vec3 m_ambientProduct = glm::vec3(0.1);
	glm::vec3 m_diffuseProduct = glm::vec3(0.5);
	glm::vec3 m_diffuseProduct2 = glm::vec3(0.3);
	glm::vec3 m_specularProduct = glm::vec3(0.8);
	float m_shininess = 50.0;
	float m_roughness = 0.5;
	glm::vec3 m_lightPos;
	int m_mode;
	float m_lightrotation = 0.0f;

	glm::vec3 m_camera_direction = -glm::vec3(1.0, 1.0, 1.0);
	float m_camera_distance = 500.0;

	unsigned int u_gamma;
	float m_gamma = 2.2f;

	int m_width;
	int m_height;
	float m_rotation;

	Object m_obj;
	Object m_skybox;
};
