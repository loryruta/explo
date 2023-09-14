#include "camera.hpp"

glm::mat4 explo::build_orientation_mat(float yaw, float pitch)
{
    glm::mat4 m(1);
    m = glm::rotate(m, yaw, glm::vec3(0, 1, 0));
    m = glm::rotate(m, -pitch, glm::vec3(1, 0, 0));
    return m;
}
