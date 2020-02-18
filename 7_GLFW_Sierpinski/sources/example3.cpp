#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>

typedef glm::vec<3, char> pixel;

int main()
{
	GLFWwindow* window;
	glfwInit();
	
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);

	int width = 640;
	int height = 480;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "Hello World", NULL, NULL);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	std::vector<pixel> canvas(width * height);

	glm::vec2 vertices[3] = 
	{
		glm::vec2(0, 0),
		glm::vec2(width - 1, 0),
		glm::vec2(width / 2, height - 1)
	};

	glm::vec2 point(width / 2, height / 2);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		for (int i = 0; i < 10; ++i)
		{
			int j = rand() % 3;

			point = (point + vertices[j]) / 2.0f;
			
			int index = static_cast<int>(point.x) + static_cast<int>(point.y) * width;
			canvas[index] = pixel(255);
		}

		glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, canvas.data());

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
