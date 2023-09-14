#pragma once

#include "Image3d.hpp"
#include "glm/glm.hpp"

namespace explo
{
    /// A 3d image equipped with a start offset for every axis, like a circular buffer.
    template <typename _T>
    class CircularImage3d
    {
       private:
        Image3d<_T> &m_image_3d;

        glm::ivec3 m_size;
        glm::ivec3 m_offset;

       public:
        explicit CircularImage3d(Image3d<_T> &image_3d, glm::uvec3 const &size) :
            m_image_3d(image_3d),
            m_size(size)
        {
        }
        ~CircularImage3d() = default;

        _T &get_pixel(glm::ivec3 const &position) const { return m_image_3d.get_pixel(to_image_position(position)); }

        void set_pixel(glm::ivec3 const &position, _T const &value) { m_image_3d.set_pixel(to_image_position(position), value); }

        void shift(glm::ivec3 const &offset) { m_offset += offset; }

       private:
        glm::uvec3 to_image_position(glm::ivec3 const &position) { return (position + m_offset) % m_size; }
    };
}  // namespace explo
