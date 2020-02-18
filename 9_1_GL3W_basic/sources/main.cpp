
#include <GL/gl3w.h>

#include <GLFW/glfw3.h>
#include <stdio.h>

int main()
{
	GLFWwindow* window;

	/* Initialize the library */
	glfwInit();

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	gl3wInit();

	const char* OpenGLversion = (const char*)glGetString(GL_VERSION);
	const char* GLSLversion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	
	printf("OpenGL %s\n GLSL: %s\n", OpenGLversion, GLSLversion);
	
	glfwTerminate();
	return 0;
}
