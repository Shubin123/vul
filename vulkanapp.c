#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

typedef struct
{
    float inPosition[3]; // Or use a vec3-like struct if you have one
    // ... other vertex attributes ...
} Vertex;

// helpers and printers and utilities

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

// initializers

VkInstance createVulkanInstance()
{
    VkInstance instance;

    if (!checkValidationLayerSupport())
    {
        fprintf(stderr, "Validation layers requested, but not available!\n");
        enumerateVulkanLayers();
        exit(EXIT_FAILURE);
    }

    // Get required GLFW extensions
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    // Calculate total number of extensions (GLFW extensions + 3 additional)
    uint32_t totalExtensionCount = glfwExtensionCount + 17; // set to 17 for all other available extensions and 3 for the known neccessary
    const char **extensions = malloc(totalExtensionCount * sizeof(char *));
    memcpy(extensions, glfwExtensions, glfwExtensionCount * sizeof(char *));

    // Add VK_KHR_surface and VK_MVK_macos_surface extensions
    extensions[glfwExtensionCount] = "VK_KHR_portability_enumeration";
    extensions[glfwExtensionCount + 1] = "VK_KHR_surface";
    extensions[glfwExtensionCount + 2] = "VK_MVK_macos_surface";

    // the rest of these i dunno if we need
    extensions[glfwExtensionCount + 3] = "VK_KHR_device_group_creation";
    extensions[glfwExtensionCount + 4] = "VK_KHR_external_fence_capabilities";
    extensions[glfwExtensionCount + 5] = "VK_KHR_external_memory_capabilities";
    extensions[glfwExtensionCount + 6] = "VK_KHR_external_semaphore_capabilities";
    extensions[glfwExtensionCount + 7] = "VK_KHR_get_physical_device_properties2";
    extensions[glfwExtensionCount + 8] = "VK_KHR_get_surface_capabilities2";
    extensions[glfwExtensionCount + 9] = "VK_EXT_debug_report";
    extensions[glfwExtensionCount + 10] = "VK_EXT_debug_utils";
    extensions[glfwExtensionCount + 11] = "VK_EXT_headless_surface";
    extensions[glfwExtensionCount + 12] = "VK_EXT_layer_settings";
    extensions[glfwExtensionCount + 13] = "VK_EXT_metal_surface";
    extensions[glfwExtensionCount + 14] = "VK_EXT_surface_maintenance1";
    extensions[glfwExtensionCount + 15] = "VK_EXT_swapchain_colorspace";
    extensions[glfwExtensionCount + 16] = "VK_LUNARG_direct_driver_loading";

    // Set up Vulkan application info
    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // Create Vulkan instance
    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = totalExtensionCount;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    // Enable validation layers
    const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
    createInfo.enabledLayerCount = sizeof(validationLayers) / sizeof(validationLayers[0]);
    createInfo.ppEnabledLayerNames = validationLayers;

    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create Vulkan instance\n");
        free(extensions); // Free the allocated memory for extensions
        exit(EXIT_FAILURE);
    }

    free(extensions); // Free the allocated memory for extensions

    return instance;
}

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window)
{
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create window surface\n");
        exit(EXIT_FAILURE);
    }
    return surface;
}

GLFWwindow *createWindow()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Vulkan Bimbo", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    return window;
}

VkPhysicalDevice selectPhysicalDevice(VkInstance instance)
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

    VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < deviceCount; i++)
    {
        VkPhysicalDevice device = devices[i];
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        // Prefer discrete GPU
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            selectedDevice = device;
            break;
        }
    }

    // If no discrete GPU found, just pick the first one
    if (selectedDevice == VK_NULL_HANDLE && deviceCount > 0)
    {
        selectedDevice = devices[0];
    }

    free(devices);

    if (selectedDevice == VK_NULL_HANDLE)
    {
        fprintf(stderr, "Failed to find a suitable GPU\n");
        exit(EXIT_FAILURE);
    }

    return selectedDevice;
}

int findGraphicsQueueFamilyIndex(VkPhysicalDevice physicalDevice)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

    VkQueueFamilyProperties *queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);

    int graphicsQueueFamilyIndex = -1;
    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsQueueFamilyIndex = i;
            break;
        }
    }

    free(queueFamilies);

    if (graphicsQueueFamilyIndex == -1)
    {
        fprintf(stderr, "Failed to find a graphics queue family\n");
        exit(EXIT_FAILURE);
    }

    return graphicsQueueFamilyIndex;
}

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, uint32_t graphicsQueueFamilyIndex, VkQueue *graphicsQueue, VkQueue *presentQueue)
{
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {0};
    // Set any physical device features you'll be using here

    VkDeviceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;

    // If you're using specific device extensions (like for swap chains), list them here
    const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset"}; // similar VK_KHR_portability_enumeration something for non-native(windows)
    createInfo.enabledExtensionCount = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    VkDevice device;
    if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create logical device\n");
        exit(EXIT_FAILURE);
    }

    // Retrieve the graphics queue
    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, graphicsQueue);
    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, presentQueue);

    return device;
}

VkSwapchainKHR createSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkExtent2D *swapChainExtent)
{
    // Query Surface Capabilities
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

    // Choose a Surface Format
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);
    VkSurfaceFormatKHR *surfaceFormats = malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats);

    VkSurfaceFormatKHR chosenFormat = surfaceFormats[0];
    for (uint32_t i = 0; i < formatCount; i++)
    {
        if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            chosenFormat = surfaceFormats[i];
            break;
        }
    }

    // Select a Presentation Mode
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
    VkPresentModeKHR *presentModes = malloc(presentModeCount * sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);

    VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < presentModeCount; i++)
    {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            chosenPresentMode = presentModes[i];
            break;
        }
    }

    // Determine Swap Extent
    *swapChainExtent = surfaceCapabilities.currentExtent;

    // Create the Swap Chain
    VkSwapchainCreateInfoKHR swapChainCreateInfo = {0};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = surface;
    swapChainCreateInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
    swapChainCreateInfo.imageFormat = chosenFormat.format;
    swapChainCreateInfo.imageColorSpace = chosenFormat.colorSpace;
    swapChainCreateInfo.imageExtent = *swapChainExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.presentMode = chosenPresentMode;
    swapChainCreateInfo.clipped = VK_TRUE;
    swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapChain;
    if (vkCreateSwapchainKHR(device, &swapChainCreateInfo, NULL, &swapChain) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create swap chain\n");
        exit(EXIT_FAILURE);
    }

    // Clean up
    free(surfaceFormats);
    free(presentModes);

    return swapChain;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);

    VkSurfaceFormatKHR *formats = malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats);

    VkSurfaceFormatKHR chosenFormat = formats[0];
    for (uint32_t i = 0; i < formatCount; i++)
    {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            chosenFormat = formats[i];
            break;
        }
    }

    free(formats);
    return chosenFormat;
}

VkImageView *createImageViews(VkDevice device, VkSwapchainKHR swapChain, VkFormat swapChainImageFormat, uint32_t *swapChainImageCount)
{
    // Get the number of images in the swap chain
    vkGetSwapchainImagesKHR(device, swapChain, swapChainImageCount, NULL);

    VkImage *swapChainImages = malloc(*swapChainImageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(device, swapChain, swapChainImageCount, swapChainImages);

    // Allocate an array of image views
    VkImageView *swapChainImageViews = malloc(*swapChainImageCount * sizeof(VkImageView));

    for (uint32_t i = 0; i < *swapChainImageCount; i++)
    {
        VkImageViewCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, NULL, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to create image views\n");
            exit(EXIT_FAILURE);
        }
    }

    free(swapChainImages);
    return swapChainImageViews;
}

VkRenderPass createRenderPass(VkDevice device, VkFormat swapChainImageFormat)
{
    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {0};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkRenderPass renderPass;
    if (vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create render pass\n");
        exit(EXIT_FAILURE);
    }

    return renderPass;
}

char *loadShaderCode(const char *filename, size_t *fileSize)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        fprintf(stderr, "Failed to open shader file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    rewind(file);

    char *buffer = malloc(*fileSize);
    if (!buffer)
    {
        fprintf(stderr, "Failed to allocate memory for shader file %s\n", filename);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fread(buffer, 1, *fileSize, file);
    fclose(file);

    return buffer;
}

VkShaderModule createShaderModule(VkDevice device, const char *code, size_t codeSize)
{
    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeSize;
    createInfo.pCode = (const uint32_t *)code;

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create shader module\n");
        exit(EXIT_FAILURE);
    }

    return shaderModule;
}

VkPipelineLayout createPipelineLayout(VkDevice device)
{
    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // No descriptor set layouts or push constants for this simple example

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create pipeline layout\n");
        exit(EXIT_FAILURE);
    }

    return pipelineLayout;
}

VkPipeline createGraphicsPipeline(VkDevice device, VkExtent2D swapChainExtent, VkRenderPass renderPass, VkPipelineLayout pipelineLayout, const char *vertShaderCode, size_t vertShaderSize, const char *fragShaderCode, size_t fragShaderSize)
{
    // Create shader modules
    VkShaderModule vertexShaderModule = createShaderModule(device, vertShaderCode, vertShaderSize);
    VkShaderModule fragmentShaderModule = createShaderModule(device, fragShaderCode, fragShaderSize);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertexShaderModule;
    vertShaderStageInfo.pName = "main"; // Make sure this matches the entry point in your shader

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragmentShaderModule;
    fragShaderStageInfo.pName = "main"; // Make sure this matches the entry point in your shader

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // TODO: Set up fixed-function pipeline stages:
    // VkPipelineVertexInputStateCreateInfo, VkPipelineInputAssemblyStateCreateInfo,
    // VkPipelineViewportStateCreateInfo, VkPipelineRasterizationStateCreateInfo,
    // VkPipelineMultisampleStateCreateInfo, VkPipelineColorBlendStateCreateInfo,
    // and VkPipelineDepthStencilStateCreateInfo.
    // If any stages are not used, they must be properly omitted or configured.

    // fixed-function stages

    VkVertexInputBindingDescription bindingDescription = {0};
    bindingDescription.binding = 0;             // The binding index (used in the vertex buffer)
    bindingDescription.stride = sizeof(Vertex); // Size of a single vertex object
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescription = {0};
    attributeDescription.binding = 0;                           // Matches the binding in the bindingDescription
    attributeDescription.location = 0;                          // Location in the shader (layout(location = 0))
    attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;   // Format for vec3 (3 floats)
    attributeDescription.offset = offsetof(Vertex, inPosition); // Offset of 'inPosition' within the Vertex struct

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 1;
    vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription;

    // Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewports and Scissors
    VkViewport viewport = {0};
    VkRect2D scissor = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {0};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Color Blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {0};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Graphics Pipeline Creation
    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout; // Assumes a pipelineLayout variable is defined
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    // TODO: If you are using a pipeline layout, set it here.
    // pipelineInfo.layout = pipelineLayout;

    VkPipeline graphicsPipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create graphics pipeline\n");
        exit(EXIT_FAILURE);
    }

    // Clean up shader modules after pipeline creation
    vkDestroyShaderModule(device, vertexShaderModule, NULL);
    vkDestroyShaderModule(device, fragmentShaderModule, NULL);

    return graphicsPipeline;
}

VkFramebuffer *createFramebuffers(VkDevice device, VkImageView *swapChainImageViews, uint32_t swapChainImageCount, VkExtent2D swapChainExtent, VkRenderPass renderPass)
{
    VkFramebuffer *swapChainFramebuffers = malloc(swapChainImageCount * sizeof(VkFramebuffer));

    for (size_t i = 0; i < swapChainImageCount; i++)
    {
        VkImageView attachments[] = {
            swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {0};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, NULL, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to create framebuffer\n");
            exit(EXIT_FAILURE);
        }
    }
    return swapChainFramebuffers;
}

VkCommandPool createCommandPool(VkDevice device, uint32_t graphicsQueueFamilyIndex)
{

    VkCommandPool commandPool;
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    poolInfo.flags = 0; // Optional

    if (vkCreateCommandPool(device, &poolInfo, NULL, &commandPool) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create command pool\n");
        exit(EXIT_FAILURE);
    }

    return commandPool;
}

VkCommandBuffer *allocateCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t swapChainImageCount)
{
    VkCommandBuffer *commandBuffers = malloc(swapChainImageCount * sizeof(VkCommandBuffer));

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = swapChainImageCount;

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to allocate command buffers\n");
        exit(EXIT_FAILURE);
    }
    return commandBuffers;
}

void recordCommandBuffers(VkCommandBuffer *commandBuffers, uint32_t swapChainImageCount, VkRenderPass renderPass, VkExtent2D swapChainExtent, VkPipeline graphicsPipeline, VkFramebuffer *swapChainFramebuffers, VkBuffer vertexBuffer)
{
    for (size_t i = 0; i < swapChainImageCount; i++)
    {
        VkCommandBufferBeginInfo beginInfo = {0};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = NULL;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to begin recording command buffer\n");
            exit(EXIT_FAILURE);
        }

        VkRenderPassBeginInfo renderPassInfo = {0};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0); // Drawing a triangle (change accordingly for your object)

        vkCmdEndRenderPass(commandBuffers[i]);
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to record command buffer\n");
            exit(EXIT_FAILURE);
        }
    }
}

void createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer *vertexBuffer, VkDeviceMemory *vertexBufferMemory)
{

    const Vertex vertices[] = {
        {{0.0f, -0.5f, 0.0f}}, // Vertex 1
        {{0.5f, 0.5f, 0.0f}},  // Vertex 2
        {{-0.5f, 0.5f, 0.0f}}  // Vertex 3
    };

    const uint32_t vertexCount = sizeof(vertices) / sizeof(vertices[0]);
    VkDeviceSize bufferSize = sizeof(Vertex) * vertexCount;

    // Create buffer
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, NULL, vertexBuffer) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create vertex buffer\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the buffer
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *vertexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(device, &allocInfo, NULL, vertexBufferMemory) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to allocate vertex buffer memory\n");
        exit(EXIT_FAILURE);
    }

    vkBindBufferMemory(device, *vertexBuffer, *vertexBufferMemory, 0);

    // Assuming you have your vertex data ready to be copied to the buffer
    void *data;
    vkMapMemory(device, *vertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t)bufferSize); // 'vertices' is your vertex data
    vkUnmapMemory(device, *vertexBufferMemory);
}

void createSyncObjects(VkDevice device, uint32_t maxFramesInFlight, VkSemaphore **imageAvailableSemaphores, VkSemaphore **renderFinishedSemaphores, VkFence **inFlightFences)
{
    *imageAvailableSemaphores = malloc(sizeof(VkSemaphore) * maxFramesInFlight);
    *renderFinishedSemaphores = malloc(sizeof(VkSemaphore) * maxFramesInFlight);
    *inFlightFences = malloc(sizeof(VkFence) * maxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Initialize fences in the signaled state

    for (size_t i = 0; i < maxFramesInFlight; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &((*imageAvailableSemaphores)[i])) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, NULL, &((*renderFinishedSemaphores)[i])) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, NULL, &((*inFlightFences)[i])) != VK_SUCCESS)
        {

            fprintf(stderr, "Failed to create synchronization objects for a frame\n");
            exit(EXIT_FAILURE);
        }
    }
}

int main()
{
    // initialization/creation process:
    // initialize_window => vulkan_instance => vulkan_surface => physical_device => find_graphics_queue_family_index (not really init/creation) => logical_device => choose_swap_surface_format (not really init/creation) => swap_chain => image_views => render_pass => load shaders (frag+vert) (not really init) =>graphics_pipeline =>  main_loop
    // tdlr: Initialization, Vulkan instance, physical device, logical device, swap chain, image views, render pass, graphics pipeline, framebuffer, command pool, command buffer, vertex buffer, sync object, mainloop

    setenv("MVK_CONFIG_LOG_LEVEL", "20", 1); // 1 means overwrite existing value
    setenv("MVK_DEBUG", "1", 1);

    VkQueue graphicsQueue, presentQueue;

    uint32_t swapChainImageCount;

    VkExtent2D swapChainExtent;

    // enumerateVulkanExtensions();
    GLFWwindow *window = createWindow();
    VkInstance instance = createVulkanInstance();

    VkSurfaceKHR surface = createSurface(instance, window);

    // enumeratePhysicalDevices(instance);

    VkPhysicalDevice physicalDevice = selectPhysicalDevice(instance);

    uint32_t graphicsQueueFamilyIndex = findGraphicsQueueFamilyIndex(physicalDevice);

    VkDevice device = createLogicalDevice(physicalDevice, graphicsQueueFamilyIndex, &graphicsQueue, &presentQueue);

    VkSurfaceFormatKHR chosenFormat = chooseSwapSurfaceFormat(physicalDevice, surface);
    VkSwapchainKHR swapChain = createSwapChain(physicalDevice, device, surface, &swapChainExtent);

    VkImageView *swapChainImageViews = createImageViews(device, swapChain, chosenFormat.format, &swapChainImageCount);

    VkRenderPass renderPass = createRenderPass(device, chosenFormat.format);

    size_t vertexShaderSize, fragmentShaderSize;

    // Load the shader code from spv files in ./shaders sub-directory
    char *vertexShaderCode = loadShaderCode("./shaders/vertex_shader.spv", &vertexShaderSize);
    char *fragmentShaderCode = loadShaderCode("./shaders/fragment_shader.spv", &fragmentShaderSize);

    VkPipelineLayout pipelineLayout = createPipelineLayout(device);

    VkPipeline graphicsPipeline = createGraphicsPipeline(device, swapChainExtent, renderPass, pipelineLayout, vertexShaderCode, vertexShaderSize, fragmentShaderCode, fragmentShaderSize);

    VkFramebuffer *swapChainFramebuffers = createFramebuffers(device, swapChainImageViews, swapChainImageCount, swapChainExtent, renderPass);


    VkCommandPool commandPool = createCommandPool(device, graphicsQueueFamilyIndex);

    VkCommandBuffer *commandBuffers = allocateCommandBuffers(device, commandPool, swapChainImageCount);

    
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    createVertexBuffer(device, physicalDevice, &vertexBuffer, &vertexBufferMemory);

    recordCommandBuffers(commandBuffers, swapChainImageCount, renderPass, swapChainExtent, graphicsPipeline, swapChainFramebuffers, vertexBuffer);

    const int MAX_FRAMES_IN_FLIGHT = 2;
    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *inFlightFences;
    
    createSyncObjects(device, MAX_FRAMES_IN_FLIGHT, &imageAvailableSemaphores, &renderFinishedSemaphores, &inFlightFences);
    
    size_t currentFrame = 0;
    
    // Main loop

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Wait for the previous frame to finish
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        // 1. Acquire an image from the swap chain
        uint32_t imageIndex;
        vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        // 2. Submit the command buffer
        VkSubmitInfo submitInfo = {0};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to submit draw command buffer\n");
            exit(EXIT_FAILURE);
        }

        // 3. Present the image
        VkPresentInfoKHR presentInfo = {0};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(presentQueue, &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    // Clean up

    // Wait for the logical device to finish operations before cleanup
    vkDeviceWaitIdle(device);

    // Cleanup: Synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], NULL);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], NULL);
        vkDestroyFence(device, inFlightFences[i], NULL);
    }
    free(renderFinishedSemaphores);
    free(imageAvailableSemaphores);
    free(inFlightFences);

    // Cleanup: Command Buffers and Command Pool
    vkFreeCommandBuffers(device, commandPool, swapChainImageCount, commandBuffers);
    free(commandBuffers);
    vkDestroyCommandPool(device, commandPool, NULL);

    // Cleanup: Framebuffers
    for (uint32_t i = 0; i < swapChainImageCount; i++)
    {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], NULL);
    }
    free(swapChainFramebuffers);

    // Cleanup: Vertex Buffer
    vkDestroyBuffer(device, vertexBuffer, NULL);
    vkFreeMemory(device, vertexBufferMemory, NULL);

    // Cleanup: Shader Modules, Pipeline, Render Pass, Image Views, Swap Chain
    free(vertexShaderCode);
    free(fragmentShaderCode);
    vkDestroyPipeline(device, graphicsPipeline, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyRenderPass(device, renderPass, NULL);

    for (uint32_t i = 0; i < swapChainImageCount; i++)
    {
        vkDestroyImageView(device, swapChainImageViews[i], NULL);
    }
    free(swapChainImageViews);
    vkDestroySwapchainKHR(device, swapChain, NULL);

    // Cleanup: Logical Device, Surface, Vulkan Instance, GLFW Window
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
