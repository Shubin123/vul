#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "../include/structure.h"

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)scancode; // Avoid unused parameter warning
    (void)mods;

    // Retrieve the combined user data
    UserData *userData = (UserData *)glfwGetWindowUserPointer(window);
    if (!userData)
    {
        return; // Safety check
    }

    // Access the KeyStates part of userData
    KeyStates *keyStates = &userData->keyStates;

    // printf("key: %d\n", key);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_W)
        keyStates->keyWPressed = (action != GLFW_RELEASE);
    if (key == GLFW_KEY_A)
        keyStates->keyAPressed = (action != GLFW_RELEASE);
    if (key == GLFW_KEY_S)
        keyStates->keySPressed = (action != GLFW_RELEASE);
    if (key == GLFW_KEY_D)
        keyStates->keyDPressed = (action != GLFW_RELEASE);
    if (key == GLFW_KEY_BACKSPACE)
        keyStates->keyDeletePressed = (action != GLFW_RELEASE);
    if (key == GLFW_KEY_SPACE)
        keyStates->keySpacePressed = (action != GLFW_RELEASE);
    if (key == GLFW_KEY_1)
        keyStates->key1Pressed = (action != GLFW_RELEASE);
    if (key == GLFW_KEY_2)
        keyStates->key2Pressed = (action != GLFW_RELEASE);
}

void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    UserData *userData = (UserData *)glfwGetWindowUserPointer(window);
    if (userData)
    {
        userData->windowData.width = width;
        userData->windowData.height = height;
        userData->windowData.wasResized = true;
    }
}

GLFWwindow *createWindow(uint32_t windowWidth, uint32_t windowHeight)
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "Vulkan Bimbo", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    return window;
}

void handleWindowResize(UserData *userData, mat4 *projection)
{
    if (userData && userData->windowData.wasResized)
    {
        int width = userData->windowData.width;
        int height = userData->windowData.height;
        float aspectRatio = (float)width / (float)height;

        glm_perspective(glm_rad(45.0f), aspectRatio, 0.1f, 10.0f, *projection);

        // Reapply the Y-axis inversion for Vulkan
        (*projection)[1][1] *= -1;

        userData->windowData.wasResized = false;
    }
}
