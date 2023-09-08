#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "util/camera.hpp"
#include "world/World.hpp"
#include "world/WorldView.hpp"

namespace explo
{
	class Entity : public std::enable_shared_from_this<Entity>
	{
	private:
		World* m_world;

		glm::vec3 m_position;

		float m_yaw;
		float m_pitch;
		glm::mat4 m_orientation_matrix;

		std::unique_ptr<WorldView> m_world_view;

	public:
		inline static glm::vec3 k_camera_offset = glm::vec3(0, 2 /* Entity's height */, 0);

		explicit Entity(World& world, glm::vec3 const& init_position = glm::vec3(0));
		~Entity();

		World& get_world() { return *m_world; };
		void set_world(World& world, glm::vec3 const& position = glm::vec3(0));

		glm::vec3 const& get_position() const { return m_position; };
		void set_position(glm::vec3 const& position);

		/// Gets the position relative to the chunk the player is in.
		glm::vec3 get_chunk_relative_position() const;

		/// Gets the position of the chunk where the player is at.
		glm::ivec3 get_chunk_position() const;

		float get_yaw() const { return m_yaw; };
		float get_pitch() const { return m_pitch; };
		void set_rotation(float yaw, float pitch);

		void update_orientation_matrix();

		glm::vec3 get_right() const;
		glm::vec3 get_up() const;
		glm::vec3 get_forward() const;

		bool has_world_view() const;
		WorldView& recreate_world_view(glm::ivec3 const& render_distance);
		WorldView& get_world_view();
	};
}
