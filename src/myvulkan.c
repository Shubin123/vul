#include <vulkan/vulkan.h>
#include <cglm/cglm.h>

#include <stdlib.h>
#include <stdio.h>

#include "../include/structure.h"
#include "helpers.c"

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

VkPipelineLayout createPipelineLayout(VkDevice device, VkDescriptorSetLayout *descriptorSetLayouts, uint32_t descriptorSetLayoutCount)
{
    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    pipelineLayoutInfo.setLayoutCount = descriptorSetLayoutCount; // Can be 0 if no descriptor sets are used
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;        // Can be NULL if no descriptor sets are used

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

    // fixed-function stages

    // description
    VkVertexInputBindingDescription bindingDescription = {0};
    bindingDescription.binding = 0;             // The binding index (used in the vertex buffer)
    bindingDescription.stride = sizeof(Vertex); // Size of a single vertex object
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputBindingDescription instanceBindingDescription = {0};
    instanceBindingDescription.binding = 1; // Binding index 1 for instance data
    instanceBindingDescription.stride = sizeof(InstanceData);
    instanceBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    // attributes
    VkVertexInputAttributeDescription attributeDescriptions[2] = {0};
    attributeDescriptions[0].binding = 0;                           // Matches the binding in the bindingDescription
    attributeDescriptions[0].location = 0;                          // Location in the shader (layout(location = 0))
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;   // Format for vec3 (3 floats)
    attributeDescriptions[0].offset = offsetof(Vertex, inPosition); // Offset of 'inPosition' within the Vertex struct

    VkVertexInputAttributeDescription instanceAttributeDescriptions[4];
    for (int i = 0; i < 4; ++i)
    {
        instanceAttributeDescriptions[i].binding = 1;      // Assuming instance data is at binding index 1
        instanceAttributeDescriptions[i].location = 1 + i; // Locations 1, 2, 3, 4
        instanceAttributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        instanceAttributeDescriptions[i].offset = sizeof(float) * 4 * i;
    }

    // vertex binding
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 1;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkVertexInputBindingDescription bindings[] = {bindingDescription, instanceBindingDescription};
    VkVertexInputAttributeDescription attributes[] = {attributeDescriptions[0], instanceAttributeDescriptions[0], instanceAttributeDescriptions[1], instanceAttributeDescriptions[2], instanceAttributeDescriptions[3]};

    vertexInputInfo.vertexBindingDescriptionCount = 2; // Two bindings: one for vertex, one for instance
    vertexInputInfo.pVertexBindingDescriptions = bindings;
    vertexInputInfo.vertexAttributeDescriptionCount = 5; // Total number of attribute descriptions
    vertexInputInfo.pVertexAttributeDescriptions = attributes;

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
    // rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // cull (efficient but may lead to render bugs)
    rasterizer.cullMode = VK_CULL_MODE_NONE;     // to disable culling/
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

    pipelineInfo.layout = pipelineLayout;

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

void createUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t bufferCount, VkBuffer *uniformBuffers, VkDeviceMemory *uniformBufferMemory)
{
    VkDeviceSize bufferSize = sizeof(UBO); // Assuming UBO is your Uniform Buffer Object structure

    for (uint32_t i = 0; i < bufferCount; ++i)
    {
        // Create the buffer
        VkBufferCreateInfo bufferInfo = {0};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, NULL, &uniformBuffers[i]) != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to create uniform buffer\n");
            exit(EXIT_FAILURE);
        }

        // Allocate memory for the buffer
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, uniformBuffers[i], &memRequirements);

        VkMemoryAllocateInfo allocInfo = {0};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(device, &allocInfo, NULL, &uniformBufferMemory[i]) != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to allocate uniform buffer memory\n");
            exit(EXIT_FAILURE);
        }

        vkBindBufferMemory(device, uniformBuffers[i], uniformBufferMemory[i], 0);
    }
}

void updateUniformBuffer(VkDevice device, VkDeviceMemory uniformBufferMemory, double time, mat4 view, mat4 projection)
{
    UBO ubo;
    ubo.time = time;
    glm_mat4_copy(view, ubo.view);             // Copy the view matrix
    glm_mat4_copy(projection, ubo.projection); // Copy the projection matrix

    void *data;
    vkMapMemory(device, uniformBufferMemory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniformBufferMemory);
}

void updateInstanceBuffer(VkDevice device, VkDeviceMemory instanceBufferMemory, InstanceData *instanceData, uint32_t instanceCount)
{
    void *data;
    vkMapMemory(device, instanceBufferMemory, 0, sizeof(InstanceData) * instanceCount, 0, &data);
    memcpy(data, instanceData, sizeof(InstanceData) * instanceCount);
    vkUnmapMemory(device, instanceBufferMemory);
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
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;

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

void recordCommandBuffers(VkCommandBuffer *commandBuffers, uint32_t imageIndex, VkRenderPass renderPass, VkExtent2D swapChainExtent, VkFramebuffer *swapChainFramebuffers, VkPipeline graphicsPipeline, VkBuffer vertexBuffer, VkBuffer indexBuffer, VkBuffer instanceBuffer, uint32_t indexCount, uint32_t instanceCount, VkDescriptorSet *descriptorSets, VkPipelineLayout pipelineLayout)
{
    vkResetCommandBuffer(commandBuffers[imageIndex], 0);

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = NULL;

    if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to begin recording command buffer at index %u\n", imageIndex);
        exit(EXIT_FAILURE);
    }

    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind the pipeline
    vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    // Bind the vertex and instance buffers and their offsets
    VkBuffer vertexBuffers[] = {vertexBuffer, instanceBuffer};
    VkDeviceSize offsets[] = {0, 0};
    vkCmdBindVertexBuffers(commandBuffers[imageIndex], 0, 2, vertexBuffers, offsets);

    // Bind the index buffer
    vkCmdBindIndexBuffer(commandBuffers[imageIndex], indexBuffer, 0, VK_INDEX_TYPE_UINT16); // Assuming uint16_t indices

    // Bind descriptor set
    vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[imageIndex], 0, NULL);

    // Issue the indexed draw call
    vkCmdDrawIndexed(commandBuffers[imageIndex], indexCount, instanceCount, 0, 0, 0);

    // End the render pass
    vkCmdEndRenderPass(commandBuffers[imageIndex]);

    // End the command buffer
    if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to end recording command buffer at index %u\n", imageIndex);
        exit(EXIT_FAILURE);
    }
}

VkDescriptorSet *createDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkBuffer *uniformBuffers, uint32_t descriptorSetCount)
{
    VkDescriptorSet *descriptorSets = malloc(descriptorSetCount * sizeof(VkDescriptorSet));

    // Create an array of descriptor set layouts for allocation
    VkDescriptorSetLayout *layouts = malloc(descriptorSetCount * sizeof(VkDescriptorSetLayout));
    for (uint32_t i = 0; i < descriptorSetCount; ++i)
    {
        layouts[i] = descriptorSetLayout;
    }

    // Descriptor set allocate info
    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = descriptorSetCount;
    allocInfo.pSetLayouts = layouts;

    // Allocate descriptor sets
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to allocate descriptor sets\n");
        exit(EXIT_FAILURE);
    }

    // Configure each descriptor set to reference the appropriate uniform buffer
    for (uint32_t i = 0; i < descriptorSetCount; ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {0};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UBO);

        VkWriteDescriptorSet descriptorWrite = {0};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, NULL);
    }

    free(layouts);
    return descriptorSets;
}

VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {0};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // or VK_SHADER_STAGE_FRAGMENT_BIT, etc., depending on your needs
    uboLayoutBinding.pImmutableSamplers = NULL;               // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VkDescriptorSetLayout descriptorSetLayout;
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayout) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create descriptor set layout\n");
        exit(EXIT_FAILURE);
    }

    return descriptorSetLayout;
}

VkDescriptorPool createDescriptorPool(VkDevice device, uint32_t numDescriptorSets)
{
    VkDescriptorPoolSize poolSize = {0};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = numDescriptorSets; // Total number of uniform buffer descriptors

    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = numDescriptorSets; // Total number of descriptor sets

    VkDescriptorPool descriptorPool;
    if (vkCreateDescriptorPool(device, &poolInfo, NULL, &descriptorPool) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create descriptor pool\n");
        exit(EXIT_FAILURE);
    }

    return descriptorPool;
}

// createVertexBuffer and createIndexBuffer buffer functions depend on createBuffer
void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, NULL, buffer) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create buffer\n");
        exit(EXIT_FAILURE);
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to allocate buffer memory\n");
        exit(EXIT_FAILURE);
    }

    vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

void createVertexBufferOLD(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer *vertexBuffer, VkDeviceMemory *vertexBufferMemory)
{
    const Vertex vertices[] = {
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

    const uint32_t vertexCount = sizeof(vertices) / sizeof(vertices[0]);
    VkDeviceSize vertexBufferSize = sizeof(Vertex) * vertexCount;

    // Create vertex buffer
    createBuffer(device, physicalDevice, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);

    // Copy vertex data to vertex buffer
    copyDataToDeviceMemory(device, *vertexBufferMemory, vertices, vertexBufferSize);
}

void createIndexBufferOLD(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer *indexBuffer, VkDeviceMemory *indexBufferMemory, uint32_t *indexCount)
{

    const uint16_t indices[] = {
        0, 1, 2, 2, 3, 0, // Front face
        1, 5, 6, 6, 2, 1, // Right face
        5, 4, 7, 7, 6, 5, // Back face
        4, 0, 3, 3, 7, 4, // Left face
        3, 2, 6, 6, 7, 3, // Top face
        0, 1, 5, 5, 4, 0  // Bottom face
    };

    *indexCount = sizeof(indices) / sizeof(indices[0]);
    VkDeviceSize indexBufferSize = sizeof(uint16_t) * (*indexCount);

    // Create index buffer
    createBuffer(device, physicalDevice, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBuffer, indexBufferMemory);

    // Copy index data to index buffer
    copyDataToDeviceMemory(device, *indexBufferMemory, indices, indexBufferSize);
}

void createInstanceBuffer(VkDevice device, VkPhysicalDevice physicalDevice, InstanceData *instanceData, uint32_t instanceCount, VkBuffer *instanceBuffer, VkDeviceMemory *instanceBufferMemory)
{
    VkDeviceSize bufferSize = sizeof(InstanceData) * instanceCount;

    // Create the buffer
    createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, instanceBuffer, instanceBufferMemory);

    // Map the buffer and copy the instance data into it
    void *data;
    vkMapMemory(device, *instanceBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, instanceData, (size_t)bufferSize);
    vkUnmapMemory(device, *instanceBufferMemory);
}

void createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, const Vertex *vertices, uint32_t vertexCount, VkBuffer *vertexBuffer, VkDeviceMemory *vertexBufferMemory) {
    VkDeviceSize vertexBufferSize = sizeof(Vertex) * vertexCount;

    createBuffer(device, physicalDevice, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);

    copyDataToDeviceMemory(device, *vertexBufferMemory, vertices, vertexBufferSize);
}

void createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, const uint16_t *indices, uint32_t indexCount, VkBuffer *indexBuffer, VkDeviceMemory *indexBufferMemory) {
    VkDeviceSize indexBufferSize = sizeof(uint16_t) * indexCount;

    createBuffer(device, physicalDevice, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBuffer, indexBufferMemory);

    copyDataToDeviceMemory(device, *indexBufferMemory, indices, indexBufferSize);
}

void createModelMatricesForGridArray(InstanceData *instanceData, uint32_t instanceCount)
{
    int rows = floor(sqrt(instanceCount));
    int cols = ceil((float)instanceCount / rows);

    for (uint32_t i = 0; i < instanceCount; ++i)
    {
        int row = i / cols;
        int col = i % cols;

        glm_mat4_identity(instanceData[i].model);

        // Set translation for each cube to create a grid
        vec3 translation = {1.5f * col, 1.5f * row, 0.0f};
        glm_translate(instanceData[i].model, translation);
    }
}

void addInstance(VkDevice device, VkPhysicalDevice physicalDevice, Transform transform, VkBuffer *instanceBuffer, VkDeviceMemory *instanceBufferMemory, InstanceData **instanceData, uint32_t *instanceCount)
{
    *instanceCount += 1;
    int bufferSize = sizeof(InstanceData) * (*instanceCount);

    // Reallocate instanceData with the new size
    *instanceData = realloc(*instanceData, bufferSize);

    // Calculate the new cube's position in the grid
    int rows = floor(sqrt(*instanceCount - 1)); // Rows of the complete grid before adding new cube
    int cols = ceil((float)(*instanceCount - 1) / rows);
    int newCubeIndex = *instanceCount - 1;
    int newRow = newCubeIndex / cols;
    int newCol = newCubeIndex % cols;

    // Check if a new row or column should start
    if (newCol >= cols)
    {
        newRow++;
        newCol = 0;
    }

    // Apply grid transformation to the new cube
    glm_mat4_identity((*instanceData)[newCubeIndex].model);
    vec3 gridTranslation = {1.5f * newCol, 1.5f * newRow, 0.0f};
    glm_translate((*instanceData)[newCubeIndex].model, gridTranslation);

    vec3 additionalTranslation = {transform.translateX, transform.translateY, 0.0f};
    glm_translate((*instanceData)[newCubeIndex].model, additionalTranslation);

    // Destroy old buffer and create a new one with updated size
    vkDeviceWaitIdle(device); // Wait for the device to be idle before destroying and reallocating
    vkDestroyBuffer(device, *instanceBuffer, NULL);
    vkFreeMemory(device, *instanceBufferMemory, NULL);
    createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, instanceBuffer, instanceBufferMemory);

    // Map the new buffer and copy the instance data into it
    void *data;
    vkMapMemory(device, *instanceBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, *instanceData, bufferSize);
    vkUnmapMemory(device, *instanceBufferMemory);
}

void removeInstance(InstanceData *instanceData, uint32_t *instanceCount, uint32_t instanceIndexToRemove)
{
    if (instanceIndexToRemove < *instanceCount - 1)
    {
        // Replace the removed instance with the last one (optional)
        instanceData[instanceIndexToRemove] = instanceData[*instanceCount - 1];
    }
    *instanceCount -= 1; // Decrease the count of instances
}

void updateAllInstanceTransformations(InstanceData *instanceData, uint32_t instanceCount, float scale)
{
    for (uint32_t i = 0; i < instanceCount; ++i)
    {
        glm_scale(instanceData[i].model, (vec3){scale, scale, scale});
    }
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
