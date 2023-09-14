#include "Game.hpp"

#include "video/RenderApi.hpp"

using namespace explo;

Game::Game(GlfwWindow &window) :
    /* Misc utils */
    m_main_thread_executor{},
    m_thread_pool(std::thread::hardware_concurrency()),

    /* Video */
    m_window(window)
{
    m_debug_ui = std::make_unique<DebugUi>(RenderApi::renderer());

    RenderApi::ui_draw(
        [this]()
        {
            m_debug_ui->display();
        }
    );

    glfwSetWindowUserPointer(m_window.handle(), this);

    glfwSetKeyCallback(
        m_window.handle(),
        [](GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            Game *game = static_cast<Game *>(glfwGetWindowUserPointer(window));
            game->on_key_change(key, action);
        }
    );

    run_on_main_thread(
        [this]()
        {
            late_initialize();
        }
    );
}

Game::~Game()
{
    RenderApi::ui_draw([]() {});
}

void Game::late_initialize()
{
    m_world = std::make_shared<World>(m_volume_generator, m_surface_generator);

    m_player = std::make_shared<Entity>(*m_world, glm::vec3(0, 10, 0));
    m_player_controller = std::make_unique<EntityController>(*m_player);

    const glm::ivec3 k_render_distance(20, 0, 20);
    m_player->recreate_world_view(k_render_distance);

    RenderApi::camera_set_position(m_player->get_position());
    RenderApi::camera_set_rotation(m_player->get_yaw(), m_player->get_pitch());
    RenderApi::camera_set_projection_params(90.0f, 1.0f, 0.01f, 1000.0f);
}

void Game::run_on_main_thread(std::function<void()> const &job)
{
    m_main_thread_executor.enqueue_job(job);
}

void Game::run_async(std::function<void()> const &job)
{
    m_thread_pool.enqueue_job(job);
}

void Game::on_window_resize(uint32_t width, uint32_t height)
{
    RenderApi::window_resize(width, height);
}

void Game::render()
{
    m_fps_counter++;

    // FPS
    if (!m_last_fps_time || (glfwGetTime() - *m_last_fps_time) >= 1.0f)
    {
        m_fps = m_fps_counter;
        m_fps_counter = 0;
        m_last_fps_time = (float)glfwGetTime();
    }

    // dt
    if (m_last_frame_time)
    {
        m_dt = float(glfwGetTime()) - *m_last_frame_time;
    }

    m_last_frame_time = (float)glfwGetTime();

    m_main_thread_executor.process();  // Process main thread jobs

    if (m_player_controller->update_position()) RenderApi::camera_set_position(m_player->get_position());

    if (m_player_controller->update_rotation()) RenderApi::camera_set_rotation(m_player->get_yaw(), m_player->get_pitch());

    RenderApi::render();
}

void Game::on_key_change(int key, int action)
{
    // Cursor logic
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if (m_window.is_cursor_disabled())
        {
            m_window.enable_cursor();
        }
        else
            m_window.close();
    }
    else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
    {
        if (!m_window.is_cursor_disabled()) m_window.disable_cursor();
    }
}

// --------------------------------------------------------------------------------------------------------------------------------

std::unique_ptr<Game> s_game;

void explo::init(GlfwWindow &window)
{
    RenderApi::init(window.handle());

    s_game = std::make_unique<Game>(window);
}

void explo::shutdown()
{
    s_game.reset();

    RenderApi::destroy();
}

void explo::render()
{
    s_game->render();
}

Game &explo::game()
{
    return *s_game;
}

void explo::run_on_main_thread(std::function<void()> const &job)
{
    s_game->run_on_main_thread(job);
}

void explo::run_async(std::function<void()> const &job)
{
    s_game->run_async(job);
}
