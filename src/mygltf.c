#include <stdlib.h>

#include <vulkan/vulkan.h>

#define CGLTF_IMPLEMENTATION
#include "../include/cgltf.h" //get ready for binary to explode in size
#include "helpers.c"

// borrows from myvulkan.c create index/vertex buffers cause thats where the api is called
void gltfLoad()
{
    cgltf_options options = {0};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, "scene.gltf", &data);

    if (result != cgltf_result_success)
    {
        fprintf(stderr, "Error parsing gltf file\n");
        exit(EXIT_FAILURE);
        cgltf_free(data);
    }

    cgltf_free(data);
}

void extractVertexDataFromPrimitive(const cgltf_primitive *primitive, Vertex **outVertices, size_t *outVertexCount)
{
    // Find and process each relevant attribute in the primitive
    const cgltf_accessor *positionAccessor = NULL;
    // Add other accessors as needed, e.g., for normals, texcoords, etc.

    for (size_t i = 0; i < primitive->attributes_count; i++)
    {
        if (primitive->attributes[i].type == cgltf_attribute_type_position)
        {
            positionAccessor = primitive->attributes[i].data;
        }
        // Handle other attribute types similarly
    }

    if (positionAccessor == NULL)
    {
        // Position data not found
        *outVertices = NULL;
        *outVertexCount = 0;
        return;
    }

    *outVertexCount = positionAccessor->count;
    *outVertices = malloc(sizeof(Vertex) * (*outVertexCount));
    if (*outVertices == NULL)
    {
        // Memory allocation failed
        *outVertexCount = 0;
        return;
    }

    if (positionAccessor == NULL)
    {
        fprintf(stderr, "Position accessor not found\n");
    }
    else
    {
        printf("Position accessor details: count = %zu, componentType = %d\n",
               positionAccessor->count, positionAccessor->component_type);
    }
printf("positionAccessor->buffer_view->buffer->data: %s\n", (char *)positionAccessor->buffer_view->buffer->data);
    // Extract position data
    for (size_t i = 0; i < *outVertexCount; i++)
    {
        const float *position = (const float *)cgltf_accessor_read_index(positionAccessor, i);
        


        if (position != NULL)
        {
            (*outVertices)[i].inPosition[0] = position[0];
            (*outVertices)[i].inPosition[1] = position[1];
            (*outVertices)[i].inPosition[2] = position[2];
            printf("Vertex %zu: (%f, %f, %f)\n", i, position[0], position[1], position[2]);
        }
        else
        {
            printf("null vertices\n");
        }
    }

    // Extract other attribute data like normals, texcoords in a similar manner
}

void extractIndexDataFromPrimitive(const cgltf_primitive *primitive, uint16_t **outIndices, size_t *outIndexCount)
{
    if (primitive->indices == NULL)
    {
        // No indices
        *outIndices = NULL;
        *outIndexCount = 0;
        fprintf(stderr, "no indices in gltf file\n");
        exit(EXIT_FAILURE);
    }

    *outIndexCount = primitive->indices->count;
    *outIndices = malloc(sizeof(uint16_t) * (*outIndexCount));
    if (*outIndices == NULL)
    {
        *outIndexCount = 0;
        exit(EXIT_FAILURE);
    }

    // Extract index data
    for (size_t i = 0; i < *outIndexCount; i++)
    {
        uint16_t index = (uint16_t)cgltf_accessor_read_index(primitive->indices, i);
        (*outIndices)[i] = index;
    }
}

void debugResultHandler(cgltf_result result)
{
    fprintf(stderr, "Error parsing GLTF file: ");
    switch (result)
    {
    case cgltf_result_data_too_short:
        fprintf(stderr, "Data too short\n");
        break;
    case cgltf_result_unknown_format:
        fprintf(stderr, "Unknown format\n");
        break;
    case cgltf_result_invalid_json:
        fprintf(stderr, "Invalid JSON\n");
        break;
    case cgltf_result_invalid_gltf:
        fprintf(stderr, "Invalid GLTF\n");
        break;
    case cgltf_result_out_of_memory:
        fprintf(stderr, "Out of memory\n");
        break;
    case cgltf_result_io_error:
        fprintf(stderr, "IO error\n");
        break;
    // Include other cases as necessary
    default:
        fprintf(stderr, "Unknown error\n");
        break;
    }
}

uint32_t loadGltfMeshes(const char *filename, VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer *vertexBuffer, VkDeviceMemory *vertexBufferMemory, VkBuffer *indexBuffer, VkDeviceMemory *indexBufferMemory)
{
    cgltf_options options = {0};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, filename, &data);

    if (result != cgltf_result_success || data == NULL)
    {
        fprintf(stderr, "Error parsing GLTF file\n");
        debugResultHandler(result);

        exit(EXIT_FAILURE);
    }

    Vertex *allVertices = NULL;
    uint16_t *allIndices = NULL;
    size_t totalVertexCount = 0;
    size_t totalIndexCount = 0;

    for (size_t i = 0; i < data->meshes_count; i++)
    {
        cgltf_mesh *mesh = &data->meshes[i];
        for (size_t j = 0; j < mesh->primitives_count; j++)
        {
            cgltf_primitive *primitive = &mesh->primitives[j];

            Vertex *vertices;
            size_t vertexCount;
            extractVertexDataFromPrimitive(primitive, &vertices, &vertexCount);

            uint16_t *indices;
            size_t indexCount;
            extractIndexDataFromPrimitive(primitive, &indices, &indexCount);

            for (size_t k = 0; k < vertexCount; k++)
            {
                // printf("Vertex %zu: (%f, %f, %f)\n", k, vertices[k].inPosition[0], vertices[k].inPosition[1], vertices[k].inPosition[2]);
            }

            for (size_t k = 0; k < indexCount; k++)
            {
                printf("Index %zu: %u\n", k, indices[k]);
            }

            // Reallocate and append new vertices and indices
            allVertices = realloc(allVertices, sizeof(Vertex) * (totalVertexCount + vertexCount));
            memcpy(allVertices + totalVertexCount, vertices, sizeof(Vertex) * vertexCount);
            free(vertices);

            allIndices = realloc(allIndices, sizeof(uint16_t) * (totalIndexCount + indexCount));
            memcpy(allIndices + totalIndexCount, indices, sizeof(uint16_t) * indexCount);
            free(indices);

            totalVertexCount += vertexCount;
            totalIndexCount += indexCount;
        }
    }

    // Create Vulkan buffers using the combined vertices and indices
    createVertexBuffer(device, physicalDevice, allVertices, totalVertexCount, vertexBuffer, vertexBufferMemory);
    createIndexBuffer(device, physicalDevice, allIndices, totalIndexCount, indexBuffer, indexBufferMemory);

    // Free the combined buffers
    free(allVertices);
    free(allIndices);

    cgltf_free(data);
    return totalIndexCount;
}
