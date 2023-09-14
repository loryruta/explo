#include "BlockySurfaceGenerator.hpp"

#include <glm/glm.hpp>

#include "Game.hpp"
#include "world/Chunk.hpp"

using namespace explo;

void BlockySurfaceGenerator::write_block_geometry(Chunk &chunk, glm::ivec3 const &block, uint8_t block_type, SurfaceWriter &surface_writer)
{
    glm::vec2 texcoord = glm::vec2((float(block_type) + 0.5f) / float(game().m_block_registry.size()), 0.5f);

    glm::vec3 f = chunk.to_world_position(block);                                        // Block from (world space)
    glm::vec3 t = f + (glm::vec3(Chunk::k_world_size) / glm::vec3(Chunk::k_grid_size));  // Block to (world space)

    // Left face
    if (block.x == 0 || chunk.get_block_type_at(block + glm::ivec3(-1, 0, 0)) == 0)  // TODO check if the offset block is still inside the chunk
    {
        surface_writer.add_quad(
            SurfaceVertex{.m_position = glm::vec3(f.x, f.y, t.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(f.x, f.y, f.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(f.x, t.y, f.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(f.x, t.y, t.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord}
        );
    }

    // Right face
    if (block.x == Chunk::k_grid_size.x - 1 || chunk.get_block_type_at(block + glm::ivec3(1, 0, 0)) == 0)
    {
        surface_writer.add_quad(
            SurfaceVertex{.m_position = glm::vec3(t.x, f.y, f.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(t.x, f.y, t.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(t.x, t.y, t.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(t.x, t.y, f.z), .m_normal = glm::vec3(-1, 0, 0), .m_texcoords = texcoord}
        );
    }

    // Bottom face
    if (block.z == 0 || chunk.get_block_type_at(block - glm::ivec3(0, 1, 0)) == 0)
    {
        surface_writer.add_quad(
            SurfaceVertex{.m_position = glm::vec3(f.x, f.y, f.z), .m_normal = glm::vec3(0, -1, 0), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(f.x, f.y, t.z), .m_normal = glm::vec3(0, -1, 0), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(t.x, f.y, t.z), .m_normal = glm::vec3(0, -1, 0), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(t.x, f.y, f.z), .m_normal = glm::vec3(0, -1, 0), .m_texcoords = texcoord}
        );
    }

    // Top face
    if (block.z == Chunk::k_grid_size.y - 1 || chunk.get_block_type_at(block + glm::ivec3(0, 1, 0)) == 0)
    {
        surface_writer.add_quad(
            SurfaceVertex{.m_position = glm::vec3(f.x, t.y, f.z), .m_normal = glm::vec3(0, 1, 0), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(f.x, t.y, t.z), .m_normal = glm::vec3(0, 1, 0), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(t.x, t.y, t.z), .m_normal = glm::vec3(0, 1, 0), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(t.x, t.y, f.z), .m_normal = glm::vec3(0, 1, 0), .m_texcoords = texcoord}
        );
    }

    // Back face
    if (block.z == 0 || chunk.get_block_type_at(block + glm::ivec3(0, 0, -1)) == 0)
    {
        surface_writer.add_quad(
            SurfaceVertex{.m_position = glm::vec3(f.x, f.y, f.z), .m_normal = glm::vec3(0, 0, -1), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(f.x, t.y, f.z), .m_normal = glm::vec3(0, 0, -1), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(t.x, t.y, f.z), .m_normal = glm::vec3(0, 0, -1), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(t.x, f.y, f.z), .m_normal = glm::vec3(0, 0, -1), .m_texcoords = texcoord}
        );
    }

    // Front face
    if (block.z == Chunk::k_grid_size.z - 1 || chunk.get_block_type_at(block + glm::ivec3(0, 0, 1)) == 0)
    {
        surface_writer.add_quad(
            SurfaceVertex{.m_position = glm::vec3(f.x, f.y, t.z), .m_normal = glm::vec3(0, 0, 1), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(f.x, t.y, t.z), .m_normal = glm::vec3(0, 0, 1), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(t.x, t.y, t.z), .m_normal = glm::vec3(0, 0, 1), .m_texcoords = texcoord},
            SurfaceVertex{.m_position = glm::vec3(t.x, f.y, t.z), .m_normal = glm::vec3(0, 0, 1), .m_texcoords = texcoord}
        );
    }
}

void BlockySurfaceGenerator::generate(Chunk &chunk, SurfaceWriter &surface_writer)
{
    chunk.octree().traverse(
        [&](uint32_t block_type, uint32_t level, uint32_t morton_code)
        {
            if (block_type > 0)  // TODO check if it's a visible block or not using the BlockRegistry?
            {
                write_block_geometry(chunk, Octree::to_voxel_position(morton_code), block_type, surface_writer);
            }
        }
    );

    surface_writer.add_instance(SurfaceInstance{
        .m_transform = glm::identity<glm::mat4>(),
    });
}
