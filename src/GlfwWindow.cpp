#include "GlfwWindow.hpp"

using namespace explo;

GlfwWindow::GlfwWindow(GLFWwindow* handle) :
	m_handle(handle)
{
}

GlfwWindow::~GlfwWindow()
{
	glfwDestroyWindow(m_handle);
}

bool GlfwWindow::is_key_pressed(int key_code) const
{
	return glfwGetKey(m_handle, key_code) == GLFW_PRESS;
}

glm::vec2 GlfwWindow::get_cursor_position() const
{
	glm::dvec2 cursor_pos;
	glfwGetCursorPos(m_handle, &cursor_pos.x, &cursor_pos.y);
	return glm::vec2(cursor_pos);
}

void GlfwWindow::enable_cursor()
{
	glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void GlfwWindow::disable_cursor()
{
	glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

bool GlfwWindow::is_cursor_disabled() const
{
	return glfwGetInputMode(m_handle, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
}

bool GlfwWindow::is_opened() const
{
	return !glfwWindowShouldClose(m_handle);
}

void GlfwWindow::close()
{
	glfwSetWindowShouldClose(m_handle, true);
}

glm::ivec2 GlfwWindow::get_size() const
{
	glm::ivec2 size{};
	glfwGetFramebufferSize(m_handle, &size.x, &size.y);
	return size;
}

std::unique_ptr<GlfwWindow> GlfwWindow::create(int width, int height, std::string const& title)
{
	GLFWwindow* handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	return std::make_unique<GlfwWindow>(handle);
}

std::unique_ptr<GlfwWindow> GlfwWindow::create_headless(int width, int height)
{
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	GLFWwindow* handle = glfwCreateWindow(width, height, "", nullptr, nullptr);
	return std::make_unique<GlfwWindow>(handle);
}
