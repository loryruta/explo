#include "BlockySurfaceGenerator.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "world/Chunk.hpp"
#include "Game.hpp"

using namespace explo;

void BlockySurfaceGenerator::write_block_geometry(
	Chunk& chunk,
	glm::ivec3 const& block,
	uint8_t block_type,
	SurfaceWriter& surface_writer
	)
{
	glm::vec2 texcoord = glm::vec2(
		(float(block_type) + 0.5f) / float(game().m_block_registry.size()),
		0.5f
	);

	glm::vec3 f = chunk.to_world_position(block); // Block from (world space)
	glm::vec3 t = f + (glm::vec3(Chunk::k_world_size) / glm::vec3(Chunk::k_grid_size)); // Block to (world space)

	uint32_t v0, v1, v2, v3; // Face vertices

	// Left face
	if (block.x == 0 || chunk.get_block_type_at(block + glm::ivec3(-1, 0, 0)) == 0)
	{
		surface_writer.add_quad(
			SurfaceVertex{.m_position = glm::vec3(f.x, f.y, t.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(f.x, f.y, f.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(f.x, t.y, f.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(f.x, t.y, t.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord }
			);
	}

	// Right face
	if (block.x == Chunk::k_grid_size.x - 1 || chunk.get_block_type_at(block + glm::ivec3(1, 0, 0)) == 0)
	{
		surface_writer.add_quad(
			SurfaceVertex{.m_position = glm::vec3(t.x, f.y, f.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(t.x, f.y, t.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(t.x, t.y, t.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(t.x, t.y, f.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord }
			);
	}

	// Bottom face
	if (block.z == 0 || chunk.get_block_type_at(block - glm::ivec3(0, 1, 0)) == 0)
	{
		surface_writer.add_quad(
			SurfaceVertex{.m_position = glm::vec3(f.x, f.y, f.z), .m_normal = glm::vec3(0, -1, 0), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(f.x, f.y, t.z), .m_normal = glm::vec3(0, -1, 0), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(t.x, f.y, t.z), .m_normal = glm::vec3(0, -1, 0), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(t.x, f.y, f.z), .m_normal = glm::vec3(0, -1, 0), .m_texcoords = texcoord }
			);
	}

	// Top face
	if (block.z == Chunk::k_grid_size.y - 1 || chunk.get_block_type_at(block + glm::ivec3(0, 1, 0)) == 0)
	{
		surface_writer.add_quad(
			SurfaceVertex{.m_position = glm::vec3(f.x, t.y, f.z), .m_normal = glm::vec3(0, 1, 0), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(f.x, t.y, t.z), .m_normal = glm::vec3(0, 1, 0), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(t.x, t.y, t.z), .m_normal = glm::vec3(0, 1, 0), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(t.x, t.y, f.z), .m_normal = glm::vec3(0, 1, 0), .m_texcoords = texcoord }
			);
	}

	// Back face
	if (block.z == 0 || chunk.get_block_type_at(block + glm::ivec3(0, 0, -1)) == 0)
	{
		surface_writer.add_quad(
			SurfaceVertex{.m_position = glm::vec3(f.x, f.y, f.z), .m_normal = glm::vec3(0, 0, -1), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(f.x, t.y, f.z), .m_normal = glm::vec3(0, 0, -1), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(t.x, t.y, f.z), .m_normal = glm::vec3(0, 0, -1), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(t.x, f.y, f.z), .m_normal = glm::vec3(0, 0, -1), .m_texcoords = texcoord }
			);
	}

	// Front face
	if (block.z == Chunk::k_grid_size.z - 1 || chunk.get_block_type_at(block + glm::ivec3(0, 0, 1)) == 0)
	{
		surface_writer.add_quad(
			SurfaceVertex{.m_position = glm::vec3(f.x, f.y, t.z), .m_normal = glm::vec3(0, 0, 1), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(f.x, t.y, t.z), .m_normal = glm::vec3(0, 0, 1), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(t.x, t.y, t.z), .m_normal = glm::vec3(0, 0, 1), .m_texcoords = texcoord },
			SurfaceVertex{.m_position = glm::vec3(t.x, f.y, t.z), .m_normal = glm::vec3(0, 0, 1), .m_texcoords = texcoord }
			);
	}
}

void BlockySurfaceGenerator::generate_surface_from_grid3d_volume(
	Chunk& chunk,
	VolumeStorage& volume_storage,
	SurfaceWriter& surface_writer
	)
{
	// TODO
}

void BlockySurfaceGenerator::generate_surface_from_octree_volume(
	Chunk& chunk,
	OctreeVolumeStorage& volume_storage,
	SurfaceWriter& surface_writer
	)
{
	volume_storage.traverse_octree([&](OctreeVolumeStorage::OctreeNode const& node, uint32_t level, uint64_t morton_code) -> bool
	{
		uint8_t block_type = node.m_block;
		if (node.is_leaf() && block_type > 0) // TODO check if it's a visible block or not using the BlockRegistry?
		{
			write_block_geometry(
				chunk,
				OctreeVolumeStorage::make_position_from_morton_code(morton_code),
				block_type,
				surface_writer
				);
		}

		return true;
	});
}

void BlockySurfaceGenerator::generate(Chunk& chunk, SurfaceWriter& surface_writer)
{
	std::unique_ptr<VolumeStorage> const& volume = chunk.get_volume();

	if (!volume)
		return;

	try
	{
		OctreeVolumeStorage& octree_volume = dynamic_cast<OctreeVolumeStorage&>(*volume);
		generate_surface_from_octree_volume(chunk, octree_volume, surface_writer);
	}
	catch (std::bad_cast& exception)
	{
		generate_surface_from_grid3d_volume(chunk, *volume, surface_writer);
	}
 }
