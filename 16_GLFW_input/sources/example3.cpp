#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>

typedef glm::vec<3, char, glm::highp> pixel;

class Context
{
public:
	std::vector<pixel>* GetCanvas()
	{
		return &canvas;
	}
	std::vector<pixel> canvas;
	int width;
	int height;
};

int main()
{
	GLFWwindow* window;

	/* Initialize the library */
	glfwInit();

	Context ctx;
	ctx.width = 640;
	ctx.height = 480;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(ctx.width, ctx.height, "Hello World", NULL, NULL);

	glfwSetWindowUserPointer(window, &ctx);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	ctx.canvas.resize(ctx.width * ctx.height);

	glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y)
	{
		Context* ctx = (Context*)glfwGetWindowUserPointer(w);
		int _x = static_cast<int>(x + 100 * ctx->width) % ctx->width;
		int _y = (static_cast<int>(ctx->height - y - 1 + 100 * ctx->height) % ctx->height);
		ctx->canvas[_x + _y * ctx->width] = pixel(255);
	});
	
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawPixels(ctx.width, ctx.height, GL_RGB, GL_UNSIGNED_BYTE, ctx.canvas.data());

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
