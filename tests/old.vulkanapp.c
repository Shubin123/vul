#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // Specify GLFW to not create an OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Create a GLFW window
    GLFWwindow *window = glfwCreateWindow(800, 600, "Vulkan Window", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    // Set up Vulkan application info
    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // Get required GLFW extensions
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Include VK_KHR_portability_enumeration extension
    uint32_t totalExtensionCount = glfwExtensionCount + 1;
    const char **allExtensions = malloc(totalExtensionCount * sizeof(const char *));
    memcpy(allExtensions, glfwExtensions, glfwExtensionCount * sizeof(const char *));
    allExtensions[glfwExtensionCount] = "VK_KHR_portability_enumeration";

    // Create Vulkan instance
    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = totalExtensionCount;
    createInfo.ppEnabledExtensionNames = allExtensions;
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    VkInstance instance;
    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create Vulkan instance\n");
        free(allExtensions);
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Create a Vulkan surface for the GLFW window
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create window surface\n");
        vkDestroyInstance(instance, NULL);
        free(allExtensions);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }


// device support 

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

    if (deviceCount == 0)
    {
        fprintf(stderr, "Failed to find GPUs with Vulkan support\n");
        exit(EXIT_FAILURE);
    }

    VkPhysicalDevice *devices = malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

for (int i = 0; i < deviceCount; i++) {
    VkPhysicalDevice device = devices[i];

    // Query the properties of the physical device
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    printf("Device %d: %s\n", i, deviceProperties.deviceName);


    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties* queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for (int j = 0; j < queueFamilyCount; j++) {
        if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            // This device has a queue family that supports graphics operations
            // You might also want to check for presentation support here
            break;
        }
    }

    free(queueFamilies);

    // Additional checks for device suitability...
}




    VkSwapchainKHR swapChain;

    VkCommandPool commandPool;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        // Rendering and presentation logic goes here
    }

    // Clean up
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);
    free(allExtensions);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
