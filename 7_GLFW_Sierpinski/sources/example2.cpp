#include <GLFW/glfw3.h>
#include <cmath>

int main(void)
{
	GLFWwindow* window;
	glfwInit(); /* Initialize the library */
	
	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	
	glClearColor(0.3, 0.0, 0.3, 1.0);
	
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Poll for and process events */
		glfwPollEvents();
		
		glClear(GL_COLOR_BUFFER_BIT);
		
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
