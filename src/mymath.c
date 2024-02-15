// CGLM
#include <cglm/cglm.h>

#include "../include/structure.h"

void createModelMatrix(mat4 *modelMatrix)
{
    glm_mat4_identity(*modelMatrix);
}

void createViewMatrix(mat4 viewMatrix, vec3 cameraPos, vec3 cameraTarget, vec3 cameraUp)
{
    glm_lookat(cameraPos, cameraTarget, cameraUp, viewMatrix);
}

void createProjectionMatrix(mat4 projectionMatrix, float fov, float aspectRatio, float nearPlane, float farPlane)
{
    glm_perspective(glm_rad(fov), aspectRatio, nearPlane, farPlane, projectionMatrix);
    projectionMatrix[1][1] *= -1; // Flip the Y axis this needs to be done again when screen is resized
}

void applyFriction(Transform *transform, float frictionFactor)
{
    if (!transform)
        return;

    // Apply friction to velocities
    transform->translateX *= frictionFactor;
    transform->translateY *= frictionFactor;

    // Optional: Set a threshold below which velocities are zeroed
    if (fabs(transform->translateX) < 0.001f)
        transform->translateX = 0.0f;
    if (fabs(transform->translateY) < 0.001f)
        transform->translateY = 0.0f;
}
