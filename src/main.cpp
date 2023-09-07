#include <optional>

#include <GLFW/glfw3.h>

#include <vren/context.hpp>

#include "log.hpp"
#include "Game.hpp"
#include "GlfwWindow.hpp"

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

	/* */

	std::unique_ptr<GlfwWindow> window = GlfwWindow::create(100, 100, "explo");
	explo::init(*window);

	glm::ivec2 last_window_size = glm::ivec2(-1);

	while (window->is_opened())
	{
		glfwPollEvents();

		glm::ivec2 window_size = window->get_size(); // Handle resize

		if (window_size.x != last_window_size.x || window_size.y != last_window_size.y)
		{
			explo::game().on_window_resize(window_size.x, window_size.y);

			last_window_size.x = window_size.x;
			last_window_size.y = window_size.y;
		}

		explo::render();
	}

	explo::shutdown();
	window.reset();

	LOG_I("", "Bye bye!");

	glfwTerminate();

	return 0;
}
