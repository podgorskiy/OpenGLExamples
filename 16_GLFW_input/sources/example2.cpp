#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>

typedef glm::vec<3, char, glm::highp> pixel;

std::vector<pixel> canvas;
int width = 640;
int height = 480;


int main()
{
	GLFWwindow* window;

	/* Initialize the library */
	glfwInit();

	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);


	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "Hello World", NULL, NULL);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);


	canvas.resize(width * height);

	glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y)
	{
		int _x = static_cast<int>(x + 100 * width) % width;
		int _y = (static_cast<int>(height - y - 1 + 100 * height) % height);
		canvas[_x + _y * width] = pixel(255);
	});

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, canvas.data());

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
