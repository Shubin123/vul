#pragma once // need these structs in two c files (especially UserData)
#include <GLFW/glfw3.h>

typedef struct
{
    float inPosition[3]; // Or use a vec3-like struct if you have one
} Vertex;

typedef struct
{
    float time;
    mat4 view;
    mat4 projection;
} UBO;

typedef struct
{
    mat4 model;
} InstanceData;

typedef struct
{
    int keyWPressed;
    int keyAPressed;
    int keySPressed;
    int keyDPressed;
    int keySpacePressed;
    int keyDeletePressed;
    int key1Pressed;
    int key2Pressed;
} KeyStates;

typedef struct
{
    GLFWwindow *handle;
    int width;
    int height;
    bool wasResized;
} WindowData;

typedef struct
{
    WindowData windowData;
    KeyStates keyStates;
} UserData;

typedef struct
{
    float translateX;
    float translateY;
    float scale; // Added scale factor
    // Add more transformation fields as needed (e.g., translateZ, rotate, scale)
} Transform;
