#pragma once

namespace explo
{
	// Forward decl
	class game;

	class debug_ui
	{
	public:
		explicit debug_ui();
		~debug_ui() = default;

		void display();

	private:
		void display_renderer_window();
		void display_jobs_window();
	};
} // namespace explo

