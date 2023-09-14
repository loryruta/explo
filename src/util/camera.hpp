#pragma once

#include <glm/glm.hpp>
#include <vren/camera.hpp>

namespace explo
{
    using camera_t = vren::camera;

    glm::mat4 build_orientation_mat(float yaw, float pitch);

    inline glm::vec3 get_right_vec(glm::mat4 const &orientation_mat)
    {
        return orientation_mat[0];
    }
    inline glm::vec3 get_up_vec(glm::mat4 const &orientation_mat)
    {
        return orientation_mat[1];
    }
    inline glm::vec3 get_forward_vec(glm::mat4 const &orientation_mat)
    {
        return orientation_mat[2];
    }

}  // namespace explo