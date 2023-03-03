#include "Game.hpp"

#include <GLFW/glfw3.h>

#include "video/RenderApi.hpp"

using namespace explo;

Game::Game(GLFWwindow* window) :
	/* Misc utils */
	m_main_thread_executor{},
	m_thread_pool(std::thread::hardware_concurrency()),

    /* Video */
    m_window(window)
{
	m_world = std::make_shared<World>(
		m_sincos_volume_generator,
		m_blocky_surface_generator
		);

	m_player = std::make_shared<Entity>(
		*m_world,
		glm::vec3(0, 100, 0) /* position */
		);;

	m_entity_controller = std::make_unique<EntityController>(*m_player);

	glfwSetWindowUserPointer(m_window, this);

	glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
		game->on_key_change(key, action);
	});

	int width, height;
	glfwGetWindowSize(m_window, &width, &height);
}

Game::~Game()
{
}

void Game::run_on_main_thread(std::function<void()> const& job)
{
	m_main_thread_executor.enqueue_job(job);
}

void Game::run_async(std::function<void()> const& job)
{
	m_thread_pool.enqueue_job(job);
}

void Game::on_window_resize(uint32_t width, uint32_t height)
{
	RenderApi::window_resize(width, height);
}

void Game::render()
{
	// Calculate dt
	if (m_last_frame_time)
		m_dt = float(glfwGetTime()) - *m_last_frame_time;

	m_last_frame_time = (float) glfwGetTime();

	m_main_thread_executor.process(); // Process main thread jobs

	if (m_entity_controller->update_position())
		RenderApi::camera_set_position(m_player->get_position());

	if (m_entity_controller->update_rotation())
		RenderApi::camera_set_rotation(m_player->get_yaw(), m_player->get_pitch());

	RenderApi::render();
}

void Game::on_key_change(int key, int action)
{
	// Cursor logic
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		if (glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
			glfwSetWindowShouldClose(m_window, true);
		} else if (glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
	else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
	{
		if (glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

// --------------------------------------------------------------------------------------------------------------------------------

std::unique_ptr<Game> s_game;

void explo::init(GLFWwindow* window)
{
	s_game = std::make_unique<Game>(window);
}

void explo::shutdown()
{
	s_game.reset();
}

void explo::render()
{
	s_game->render();
}

Game& explo::game()
{
	return *s_game;
}

void explo::run_on_main_thread(std::function<void()> const& job)
{
	s_game->run_on_main_thread(job);
}

void explo::run_async(std::function<void()> const& job)
{
	s_game->run_async(job);
}
