#pragma once

namespace explo
{
    // Forward decl
    class Renderer;

    class DebugUi
    {
    private:
        Renderer &m_renderer;

    public:
        explicit DebugUi(Renderer &renderer);
        ~DebugUi();

        void display();

    private:
        void display_jobs_window();
        void display_player_window();
        void display_world_view_window();
        void display_baked_world_view_window();
        void display_renderer_window();
        void display_vma_memory_statistics();
    };
}  // namespace explo
