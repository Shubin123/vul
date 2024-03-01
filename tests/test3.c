#include <stdio.h>
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>

typedef struct
{
    mat4 model;
} InstanceData;


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

void createModelMatricesForGridArray(InstanceData *instanceData, uint32_t instanceCount)
{
    int rows = floor(sqrt(instanceCount));
    int cols = ceil((float)instanceCount / rows);

      for (uint32_t i = 0; i < instanceCount; ++i){
        for (uint32_t j = 0; j < 4; ++j){
          for (uint32_t k = 0; k < 4; ++k){
            printf("%f", instanceData[i].model[j][k]);
          }
                            printf("\n");

        }
                            printf("\n");

      }

    for (uint32_t i = 0; i < instanceCount; ++i)
    {
        int row = i / cols;
        int col = i % cols;



        glm_mat4_identity(instanceData[i].model);

        // Set translation for each cube to create a grid
        vec3 translation = {1.5f * col, 1.5f * row, 0.0f };
        glm_translate(instanceData[i].model, translation);


    }

     for (uint32_t i = 0; i < instanceCount; ++i){
        for (uint32_t j = 0; j < 4; ++j){
          for (uint32_t k = 0; k < 4; ++k){
            printf("%f", instanceData[i].model[j][k]);
          }
                            printf("\n");

        }
                            printf("\n");

      }

          
}

int main()
{
   
VkBuffer instanceBuffer;
    VkDeviceMemory instanceBufferMemory;
    uint32_t instanceCount = 5;

    InstanceData *instanceData = malloc(sizeof(InstanceData) * instanceCount);

    createModelMatricesForGridArray(instanceData, instanceCount);

    return 0;
}