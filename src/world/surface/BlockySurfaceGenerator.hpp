#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "SurfaceGenerator.hpp"

namespace explo
{
    class BlockySurfaceGenerator : public SurfaceGenerator
    {
       public:
        explicit BlockySurfaceGenerator() = default;
        ~BlockySurfaceGenerator() = default;

        void generate(Chunk &chunk, SurfaceWriter &surface_writer) override;  // TODO no SurfaceWriter in function prototype

       protected:
        void write_block_geometry(Chunk &chunk, glm::ivec3 const &block, uint8_t block_type, SurfaceWriter &surface_writer);
    };
}  // namespace explo
