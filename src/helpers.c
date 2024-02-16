#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <vulkan/vulkan.h>

#include "../include/structure.h"

void createCubeVertexData(const Vertex **vertices, uint32_t *vertexCount) {
       static const Vertex cubeVertices[] = {
        // Front face
        {{-0.5f, -0.5f, 0.5f}}, // Vertex 0
        {{0.5f, -0.5f, 0.5f}},  // Vertex 1
        {{0.5f, 0.5f, 0.5f}},   // Vertex 2
        {{-0.5f, 0.5f, 0.5f}},  // Vertex 3

        // Back face
        {{-0.5f, -0.5f, -0.5f}}, // Vertex 4
        {{0.5f, -0.5f, -0.5f}},  // Vertex 5
        {{0.5f, 0.5f, -0.5f}},   // Vertex 6
        {{-0.5f, 0.5f, -0.5f}},  // Vertex 7
    };
    *vertexCount = sizeof(cubeVertices) / sizeof(cubeVertices[0]);
    *vertices = cubeVertices;
}

void createCubeIndexData(const uint16_t **indices, uint32_t *indexCount) {
      static const uint16_t cubeIndices[] = {
        0, 1, 2, 2, 3, 0, // Front face
        1, 5, 6, 6, 2, 1, // Right face
        5, 4, 7, 7, 6, 5, // Back face
        4, 0, 3, 3, 7, 4, // Left face
        3, 2, 6, 6, 7, 3, // Top face
        0, 1, 5, 5, 4, 0  // Bottom face
    };

    *indices = cubeIndices;
    *indexCount = sizeof(cubeIndices) / sizeof(cubeIndices[0]);
    
}

void enumerateVulkanExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    VkExtensionProperties *availableExtensions = malloc(extensionCount * sizeof(VkExtensionProperties));
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, availableExtensions);

    for (uint32_t i = 0; i < extensionCount; i++)
    {
        printf("Available extension: %s\n", availableExtensions[i].extensionName);
    }

    free(availableExtensions);
}

void enumerateVulkanLayers()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties *availableLayers = malloc(layerCount * sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    printf("Available Vulkan layers:\n");
    for (uint32_t i = 0; i < layerCount; i++)
    {
        printf("\t%s\n", availableLayers[i].layerName);
    }

    free(availableLayers);
}

void enumeratePhysicalDevices(VkInstance instance)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

    if (deviceCount == 0)
    {
        fprintf(stderr, "Failed to find GPUs with Vulkan support\n");
        exit(EXIT_FAILURE);
    }

    VkPhysicalDevice *devices = malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    for (uint32_t i = 0; i < deviceCount; i++)
    {
        VkPhysicalDevice device = devices[i];
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        printf("Device %d: %s\n", i, deviceProperties.deviceName);
    }

    free(devices);
}

bool checkValidationLayerSupport()
{
    const char *validationLayers[] = {
        "VK_LAYER_KHRONOS_validation"};
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties *availableLayers = (VkLayerProperties *)malloc(layerCount * sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    bool layersAvailable = true;
    for (unsigned long i = 0; i < sizeof(validationLayers) / sizeof(validationLayers[0]); i++)
    {
        bool layerFound = false;

        for (uint32_t j = 0; j < layerCount; j++)
        {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            layersAvailable = false;
            break;
        }
    }

    free(availableLayers);
    return layersAvailable;
}

void copyDataToDeviceMemory(VkDevice device, VkDeviceMemory deviceMemory, const void *data, VkDeviceSize size)
{
    void *mappedMemory;
    vkMapMemory(device, deviceMemory, 0, size, 0, &mappedMemory);
    memcpy(mappedMemory, data, size);
    vkUnmapMemory(device, deviceMemory);
}

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    fprintf(stderr, "Failed to find suitable memory type!\n");
    exit(EXIT_FAILURE);
}

UserData *createUserData(uint32_t windowWidth, uint32_t windowHeight)
{
    UserData *userData = malloc(sizeof(UserData));
    if (!userData)
    {
        fprintf(stderr, "Failed to allocate memory for user data\n");
        exit(EXIT_FAILURE);
    }

    // Initialize WindowData
    userData->windowData.width = windowWidth;
    userData->windowData.height = windowHeight;
    userData->windowData.wasResized = false;

    // Initialize KeyStates
    userData->keyStates.keyWPressed = false;
    userData->keyStates.keyAPressed = false;
    userData->keyStates.keySPressed = false;
    userData->keyStates.keyDPressed = false;

    userData->keyStates.keySpacePressed = false;
    userData->keyStates.keyDeletePressed = false;

    // Initialize other fields of userData as necessary...

    return userData;
}
