#include "Application.h"

#include <GLFW/glfw3.h>
#include <memory>

int main()
{
	GLFWwindow* window;

	/* Initialize the library */
	glfwInit();

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	{
		std::shared_ptr<Application> app = std::make_shared<Application>();

		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			app->Draw();

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}
	}

	glfwTerminate();
	return 0;
}
