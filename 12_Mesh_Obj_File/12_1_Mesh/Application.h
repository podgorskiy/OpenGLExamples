#pragma once

class Application
{
public:
	Application();
	~Application();

	void Draw(float time);

private:
	unsigned int m_program;
	unsigned int m_attrib_pos;
	unsigned int m_attrib_color;
	unsigned int m_uniform_rotation;
	unsigned int m_vertexBufferObject;
	unsigned int m_indexBufferObject;
	unsigned int m_indexSize;
};
