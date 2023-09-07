#pragma once

#include <string>
#include <memory>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace explo
{
	class GlfwWindow
	{
	private:
		GLFWwindow* m_handle;

	public:
		explicit GlfwWindow(GLFWwindow* handle);
		GlfwWindow(GlfwWindow const& other) = delete;
		~GlfwWindow();

		GLFWwindow* handle() const { return m_handle; };

		/// \param key_code the key. Currently matches `GLFW_KEY_*`.
		bool is_key_pressed(int key_code) const;

		glm::vec2 get_cursor_position() const;

		void enable_cursor();
		void disable_cursor();
		bool is_cursor_disabled() const;

		bool is_opened() const;
		void close();

		glm::ivec2 get_size() const;

		static std::unique_ptr<GlfwWindow> create(int width, int height, std::string const& title);
		static std::unique_ptr<GlfwWindow> create_headless(int width, int height);
	};
} // namespace explo