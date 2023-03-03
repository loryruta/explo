#pragma once

#include <GLFW/glfw3.h>

#include "input/EntityController.hpp"
#include "world/BlockRegistry.hpp"
#include "world/Entity.hpp"
#include "video/debug_ui.hpp"
#include "util/SyncJobExecutor.hpp"
#include "util/ThreadPool.hpp"
#include "world/volume/SinCosVolumeGenerator.hpp"
#include "world/surface/BlockySurfaceGenerator.hpp"

namespace explo
{
    class Game
    {
    public:
		/* Misc utils */
		SyncJobExecutor m_main_thread_executor;
		ThreadPool m_thread_pool;

        /* Input */
        std::unique_ptr<EntityController> m_entity_controller;

        /* World */
		BlockRegistry m_block_registry;
		SinCosVolumeGenerator m_sincos_volume_generator;
		BlockySurfaceGenerator m_blocky_surface_generator;

		std::shared_ptr<World> m_world;
		std::shared_ptr<Entity> m_player;

        /* Video */
        GLFWwindow* m_window;

		float m_dt = 0.0f;
		std::optional<float> m_last_frame_time;

    public:
        explicit Game(GLFWwindow* window);
        ~Game();

		GLFWwindow* get_window() const { return m_window; };
		float get_dt() const { return m_dt; }

		/* Job executors */

		void run_on_main_thread(std::function<void()> const& job);
		void run_async(std::function<void()> const& job);

		void on_window_resize(uint32_t width, uint32_t height);
        void render();

	private:
		void on_key_change(int key, int action);
    };

	// --------------------------------------------------------------------------------------------------------------------------------

	void init(GLFWwindow* window);
	void shutdown();

	void render();

	Game& game();

	/* Helpers */

	void run_on_main_thread(std::function<void()> const& job);
	void run_async(std::function<void()> const& job);
}
