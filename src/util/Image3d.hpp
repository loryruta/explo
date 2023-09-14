#pragma once

#include <vector>

#include "glm/glm.hpp"

namespace explo
{
    template <typename _T>
    class Image3d
    {
    protected:
        glm::uvec3 m_size;
        std::vector<_T> m_image_data;

    public:
        explicit Image3d(glm::uvec3 const &size) :
            m_size(size)
        {
            m_image_data.resize(m_size.x * m_size.y * m_size.z);
        }

        ~Image3d() = default;

        glm::uvec3 const &get_size() const { return m_size; }
        _T const *get_data() const { return m_image_data.data(); }

        uint32_t flatten_3d_index(glm::uvec3 const &position) { return position.x * (m_size.x * m_size.y) + position.y * m_size.x + position.z; }

        _T &get_pixel(glm::uvec3 const &position) const
        {
            uint32_t i = flatten_3d_index(position);
            return m_image_data.at(i);
        }

        void set_pixel(glm::uvec3 const &position, _T const &pixel)
        {
            uint32_t i = flatten_3d_index(position);
            m_image_data[i] = pixel;
        }
    };
}  // namespace explo
