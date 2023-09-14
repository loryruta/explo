#pragma once

#include <optional>

namespace explo
{
    // Forward decl
    class Entity;

    class EntityController
    {
    public:
        static constexpr float k_movement_sensitivity = 5.0f;  // m/s
        static constexpr float k_movement_speed_boost = 10.0f;
        static constexpr float k_rotation_sensitivity = 0.1f;

    private:
        Entity &m_entity;

        std::optional<double> m_last_cursor_x, m_last_cursor_y;
        std::optional<double> m_last_timestamp;

    public:
        explicit EntityController(Entity &entity);
        ~EntityController();

        bool update_position();
        bool update_rotation();

    private:
        bool is_input_mode_enabled() const;
    };
}  // namespace explo
