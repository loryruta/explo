#pragma once

#include "input/EntityController.hpp"
#include "world/BlockRegistry.hpp"
#include "world/Entity.hpp"
#include "video/DebugUi.hpp"
#include "util/SyncJobExecutor.hpp"
#include "util/ThreadPool.hpp"
#include "world/volume/SinCosVolumeGenerator.hpp"
#include "world/volume/PerlinNoiseGenerator.hpp"
#include "world/surface/BlockySurfaceGenerator.hpp"
#include "GlfwWindow.hpp"

namespace explo
{
    class Game
    {
    public:
		/* Misc utils */
		SyncJobExecutor m_main_thread_executor;
		ThreadPool m_thread_pool;

        /* World */
		BlockRegistry m_block_registry;
		PerlinNoiseGenerator m_volume_generator;
		BlockySurfaceGenerator m_surface_generator;

        /* Video */
        GlfwWindow& m_window;
		std::unique_ptr<DebugUi> m_debug_ui;

		float m_dt = 0.0f;
		std::optional<float> m_last_frame_time;

		// FPS
		std::optional<float> m_last_fps_time;
		int m_fps_counter = 0;
		int m_fps = 0;

		/* Late initialize */
		std::shared_ptr<World> m_world;
		std::shared_ptr<Entity> m_player;
		std::unique_ptr<EntityController> m_player_controller;

    public:
        explicit Game(GlfwWindow& window);
        ~Game();

		GlfwWindow& get_window() { return m_window; };
		float get_dt() const { return m_dt; }

		/* Job executors */

		void run_on_main_thread(std::function<void()> const& job);
		void run_async(std::function<void()> const& job);

		void on_window_resize(uint32_t width, uint32_t height);
        void render();

	private:
		void late_initialize();

		void on_key_change(int key, int action);
    };

	// --------------------------------------------------------------------------------------------------------------------------------

	void init(GlfwWindow& window);
	void shutdown();

	void render();

	Game& game();

	/* Helpers */

	void run_on_main_thread(std::function<void()> const& job);
	void run_async(std::function<void()> const& job);
}
