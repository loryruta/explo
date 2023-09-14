#pragma once

#include "SurfaceWriter.hpp"

namespace explo
{
    // Forward decl
    class Chunk;

    class SurfaceGenerator
    {
       public:
        virtual void generate(Chunk &chunk, SurfaceWriter &surface_writer) = 0;
    };
}  // namespace explo
