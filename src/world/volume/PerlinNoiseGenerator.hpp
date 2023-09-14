#pragma once

#include "VolumeGenerator.hpp"
#include "util/PerlinNoise.hpp"

namespace explo
{
    class PerlinNoiseGenerator : public VolumeGenerator
    {
    public:
        static constexpr uint32_t k_max_world_height = 100;

    private:
        siv::PerlinNoise m_perlin_noise;

    public:
        explicit PerlinNoiseGenerator();
        ~PerlinNoiseGenerator();

        void generate_volume(Chunk &chunk);

    private:
        int get_height_at(int x, int z);
    };
}  // namespace explo
