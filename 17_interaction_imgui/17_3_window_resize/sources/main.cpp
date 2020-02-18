#include "Application.h"

#include <GLFW/glfw3.h>
#include <memory>
#include <chrono>

int main()
{
	GLFWwindow* window;

	/* Initialize the library */
	glfwInit();

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 640, "Hello World", NULL, NULL);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	
	{
		std::shared_ptr<Application> app = std::make_shared<Application>();
		
		app->Resize(640, 640);
		
		glfwSetWindowUserPointer(window, app.get());

		glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height)
		{
			Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
			app->Resize(width, height);
		});

		auto start = std::chrono::steady_clock::now();

		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			auto current_timestamp = std::chrono::steady_clock::now();

			std::chrono::duration<float> elapsed_time = (current_timestamp - start);

			app->Draw(elapsed_time.count());

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}

		glfwSetWindowSizeCallback(window, nullptr);
	}

	glfwTerminate();
	return 0;
}
