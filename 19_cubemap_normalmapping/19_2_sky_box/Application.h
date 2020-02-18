#pragma once
#include "Object.h"
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
	unsigned int m_attrib_color;

	unsigned int u_projection;
	unsigned int u_projection_skybox;
	unsigned int m_uniform_cubemap;
	unsigned int m_uniform_cubemap_skybox;
	unsigned int m_vertexBufferObject;
	unsigned int m_indexBufferObject;
	unsigned int m_indexSize;
	unsigned int u_camera_pos;
	unsigned int u_camera_pos_skybox;

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
