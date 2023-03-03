#include "EntityController.hpp"

#include "GLFW/glfw3.h"

#include "Game.hpp"

using namespace explo;

EntityController::EntityController(Entity& entity) :
    m_entity(entity)
{
}

EntityController::~EntityController()
{
}

bool EntityController::update_position()
{
	if (!is_input_mode_enabled())
		return false;

	GLFWwindow* window = game().get_window();
	float dt = game().get_dt();

	bool updated = false;

	glm::vec3 position = m_entity.get_position();
	glm::vec3 dp{};

	if (glfwGetKey(window, GLFW_KEY_A)) dp += -m_entity.get_right(), updated = true;
	if (glfwGetKey(window, GLFW_KEY_D)) dp += m_entity.get_right(), updated = true;
	if (glfwGetKey(window, GLFW_KEY_S)) dp += -m_entity.get_forward(), updated = true;
	if (glfwGetKey(window, GLFW_KEY_W)) dp += m_entity.get_forward(), updated = true;
	if (glfwGetKey(window, GLFW_KEY_SPACE)) dp += m_entity.get_up(), updated = true;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) dp += -m_entity.get_up(), updated = true;

	if (updated)
	{
		position += dp * k_movement_sensitivity * (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) ? k_movement_speed_boost : 1.0f) * dt;
		m_entity.set_position(position);
	}

	return updated;
}

bool EntityController::update_rotation()
{
	if (!is_input_mode_enabled())
		return false;

	GLFWwindow* window = game().get_window();
	float dt = game().get_dt();

	bool updated = false;

	double cursor_x, cursor_y;
	glfwGetCursorPos(window, &cursor_x, &cursor_y);

	if (m_last_cursor_x && m_last_cursor_y)
	{
		float yaw = m_entity.get_yaw();
		float pitch = m_entity.get_pitch();

		float cursor_dx = static_cast<float>(cursor_x - *m_last_cursor_x);
		float cursor_dy = static_cast<float>(cursor_y - *m_last_cursor_y);

		if (cursor_dx || cursor_dy)
		{
			yaw += cursor_dx * k_rotation_sensitivity * dt;
			pitch += -cursor_dy * k_rotation_sensitivity * dt;

			pitch = glm::clamp(pitch, -glm::pi<float>() / 2.0f, glm::pi<float>() / 2.0f);

			m_entity.set_rotation(yaw, pitch);

			updated = true;
		}
	}

	m_last_cursor_x = cursor_x;
	m_last_cursor_y = cursor_y;

	return updated;
}

bool EntityController::is_input_mode_enabled() const
{
	return glfwGetInputMode(game().get_window(), GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
}
