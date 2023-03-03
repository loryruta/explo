#include <optional>

#include <GLFW/glfw3.h>

#include <vren/context.hpp>

#include "log.hpp"
#include "Game.hpp"

using namespace explo;

void glfw_error_callback(int error_code, const char* description)
{
	fprintf(stderr, "GLFW error (code %d): %s\n", error_code, description);
}

int main(int argc, char* argv[])
{
	setvbuf(stdout, NULL, _IONBF, 0);

	// Init GLFW
	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(1);
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	LOG_I("", "GLFW initialized");

	// Create GLFW window
	GLFWwindow* window = glfwCreateWindow(720, 720, "explo", nullptr, nullptr);
	if (window == nullptr)
	{
		fprintf(stderr, "Failed to create window\n");
		exit(1);
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	LOG_I("", "Window initialized");

	/* */

	explo::init(window);

	int last_framebuffer_width = -1,
		last_framebuffer_height = -1;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// Handle resize
		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

		if (framebuffer_width != last_framebuffer_width || framebuffer_height != last_framebuffer_height)
		{
			explo::game().on_window_resize(framebuffer_width, framebuffer_height);

			last_framebuffer_width = framebuffer_width;
			last_framebuffer_height = framebuffer_height;
		}

		explo::render();
	}

	explo::shutdown();

	/* */
	LOG_I("", "Bye bye!");

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
