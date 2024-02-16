#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// #include <vulkan/vulkan.h>
// #include <GLFW/glfw3.h>
// #include <cglm/cglm.h>

#include "myvulkan.c"
#include "mymath.c"
#include "myglfw.c" 
#include "mygltf.c"
#include "helpers.c"
#include "../include/structure.h" 



int main()
{

    // initialization/creation process to the end:
    // initialize_window => vulkan_instance => vulkan_surface => physical_device => find_graphics_queue_family_index (not really init/creation) => logical_device => choose_swap_surface_format (not really init/creation) => swap_chain => image_views => render_pass => load shaders (frag+vert) (not really init) => ubo initialization / descriptor setting  => graphics_pipeline =>  frame_buffers => command_buffers => command_pool => vertex_buffer => instace_buffer => sync_objects => instance buffer (model matrices) => view_buffer => projection_buffer => index_buffer =>  main_loop => cleanup
    // tdlr: window, Vulkan instance, physical device, logical device, swap chain, image views, render pass, ubo, graphics pipeline, frame buffer, command pool, command buffer, vertex buffer, index buffer, sync object, mvp, mainloop, clean up

    setenv("MVK_CONFIG_LOG_LEVEL", "3", 1); // 1 means overwrite existing value
    setenv("MVK_DEBUG", "2", 1);
    uint32_t initialWindowWidth = 800;
    uint32_t initialWindowHeight = 600;

    VkQueue graphicsQueue, presentQueue;

    uint32_t swapChainImageCount;

    VkExtent2D swapChainExtent;

    // enumerateVulkanExtensions(); // to figure out all extensions available on the system

    // Initialize GLFW Window, WindowData struct with the created window handle

    UserData *userData = createUserData(initialWindowWidth, initialWindowHeight); // used for key press handling and window resizing

    GLFWwindow *window = createWindow(initialWindowWidth, initialWindowHeight);

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

    // Unified buffer object setup (do not mistake the uniform buffer with the vertex buffer and the layout descriptor with the attribute and binding descriptor)

    VkBuffer uniformBuffers[swapChainImageCount];
    VkDeviceMemory uniformBufferMemory[swapChainImageCount]; // created as an array but used in update as an item of said array
    createUniformBuffers(device, physicalDevice, swapChainImageCount, uniformBuffers, uniformBufferMemory);

    VkDescriptorSetLayout descriptorSetLayout = createDescriptorSetLayout(device);
    VkDescriptorPool descriptorPool = createDescriptorPool(device, swapChainImageCount);
    VkDescriptorSet *descriptorSets = createDescriptorSets(device, descriptorPool, descriptorSetLayout, uniformBuffers, swapChainImageCount);

    // pipeline layout creation and and actual pipeline creation (note the descriptor for bindings and attributes not the same as layout descriptor)

    VkPipelineLayout pipelineLayout = createPipelineLayout(device, &descriptorSetLayout, 1);
    VkPipeline graphicsPipeline = createGraphicsPipeline(device, swapChainExtent, renderPass, pipelineLayout, vertexShaderCode, vertexShaderSize, fragmentShaderCode, fragmentShaderSize);

    // Framebuffers, Command Pool, Command Buffers, Vertex Buffer, Synchronization Objects

    VkFramebuffer *swapChainFramebuffers = createFramebuffers(device, swapChainImageViews, swapChainImageCount, swapChainExtent, renderPass);
    VkCommandPool commandPool = createCommandPool(device, graphicsQueueFamilyIndex);
    VkCommandBuffer *commandBuffers = allocateCommandBuffers(device, commandPool, swapChainImageCount);

    // vertex buffers, index buffers -- FOR CUBES --
const Vertex *cubeVertices;
uint32_t vertexCount;
createCubeVertexData(&cubeVertices, &vertexCount);

const uint16_t *cubeIndices;
uint32_t indexCount;
createCubeIndexData(&cubeIndices, &indexCount);

VkBuffer vertexBuffer, indexBuffer;
VkDeviceMemory vertexBufferMemory, indexBufferMemory;

createVertexBuffer(device, physicalDevice, cubeVertices, vertexCount, &vertexBuffer, &vertexBufferMemory);
createIndexBuffer(device, physicalDevice, cubeIndices, indexCount, &indexBuffer, &indexBufferMemory);




    // semaphore, fence and sync objects
    const int MAX_FRAMES_IN_FLIGHT = 3; // 3 for full triple buffering potential
    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *inFlightFences;

    createSyncObjects(device, MAX_FRAMES_IN_FLIGHT, &imageAvailableSemaphores, &renderFinishedSemaphores, &inFlightFences);

    // projection setup
    // Example of camera parameters
    vec3 cameraPos = {0.0f, 0.0f, 7.0f};    // Camera position (max render distance is 10)
    vec3 cameraTarget = {0.0f, 0.0f, 0.0f}; // Camera target
    vec3 up = {0.0f, 1.0f, 0.0f};           // Up direction

    // Models matrices (within instanceData)

    // Create an instance buffer
    VkBuffer instanceBuffer;
    VkDeviceMemory instanceBufferMemory;
    uint32_t instanceCount = 6;

    InstanceData *instanceData = malloc(sizeof(InstanceData) * instanceCount);

    createModelMatricesForGridArray(instanceData, instanceCount);

    createInstanceBuffer(device, physicalDevice, instanceData, instanceCount, &instanceBuffer, &instanceBufferMemory);

    // View matrix
    mat4 view;
    createViewMatrix(view, cameraPos, cameraTarget, up);

    // Projection matrix
    mat4 projection;
    float aspectRatio = swapChainExtent.width / (float)swapChainExtent.height;
    createProjectionMatrix(projection, 45.0f, aspectRatio, 0.1f, 10.0f);

    // Set the glfw callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetWindowUserPointer(window, userData);

    // Initialize the Transform struct
    Transform transform = {
        .translateX = 0.0f,
        .translateY = 0.0f};

    // calc fps
    // double lastTime = glfwGetTime();
    // int numFrames = 0;

    // Main loop
    size_t currentFrame = 0; // used for buffer synchronization

    while (!glfwWindowShouldClose(window))
    {

        glfwPollEvents(); // poll early for early inputs (like key presses) before render process. this may make the user experience better


        double currentTime = glfwGetTime();
        
        // framecounter move this to helpers.c
        // numFrames++;
        // if (currentTime - lastTime >= 1.0)
        // { // If last print was more than 1 sec ago
        //     printf("%f ms/frame, %d frames/sec\n", 1000.0 / numFrames, numFrames);
        //     numFrames = 0;
        //     lastTime += 1.0;
        // }

        if (userData->keyStates.keyWPressed)
        {
            transform.translateY += 0.01f;
            transform.scale *= 1.2f; // Increase scale
        }
        if (userData->keyStates.keySPressed)
            transform.translateY -= 0.01f;
        if (userData->keyStates.keyAPressed)
            transform.translateX -= 0.01f;
        if (userData->keyStates.keyDPressed)
            transform.translateX += 0.01f;
        if (userData->keyStates.keyDeletePressed)
            if (instanceCount > 1) // must have atleast one instance
                removeInstance(instanceData, &instanceCount, instanceCount - 1);

        if (userData->keyStates.keySpacePressed)
            addInstance(device, physicalDevice, transform, &instanceBuffer, &instanceBufferMemory, &instanceData, &instanceCount); // adds to instance count no need to do this elsewhere

        if (userData->keyStates.key1Pressed)
            updateAllInstanceTransformations(instanceData, instanceCount, 1.02); // Update all instances

        if (userData->keyStates.key2Pressed)
            updateAllInstanceTransformations(instanceData, instanceCount, 0.98); // Update all instances

        applyFriction(&transform, 0.95f);

        vec3 translation = {transform.translateX, transform.translateY, 0.0f}; // Only translate in X and Y

        for (uint32_t i = 0; i < instanceCount; i++)
        {
            glm_translate(instanceData[i].model, translation);
        }

        // 0. Wait for the previous frame to finish
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        // 1. Acquire an image from the swap chain
        uint32_t imageIndex;
        vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        handleWindowResize(userData, &projection);

        updateUniformBuffer(device, uniformBufferMemory[imageIndex], currentTime, view, projection); // will need to modify this

        updateInstanceBuffer(device, instanceBufferMemory, instanceData, instanceCount);

        recordCommandBuffers(commandBuffers, imageIndex, renderPass, swapChainExtent, swapChainFramebuffers, graphicsPipeline, vertexBuffer, indexBuffer, instanceBuffer, indexCount, instanceCount, descriptorSets, pipelineLayout);
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

    // Cleanup: Instance Buffer

    vkDestroyBuffer(device, instanceBuffer, NULL);    // Destroy the instance buffer
    vkFreeMemory(device, instanceBufferMemory, NULL); // Free the memory used by the instance buffer
    free(instanceData);

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

    // Cleanup: Vertex and Index Buffer and its associated memory
    vkDestroyBuffer(device, vertexBuffer, NULL);
    vkDestroyBuffer(device, indexBuffer, NULL);
    vkFreeMemory(device, indexBufferMemory, NULL);
    vkFreeMemory(device, vertexBufferMemory, NULL);

    // Cleanup: Shader Modules, Pipeline, Render Pass, Image Views, Swap Chain
    free(vertexShaderCode);
    free(fragmentShaderCode);
    vkDestroyPipeline(device, graphicsPipeline, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyRenderPass(device, renderPass, NULL);

    // UBO and Descriptor related cleanup
    vkDestroyDescriptorPool(device, descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);

    for (uint32_t i = 0; i < swapChainImageCount; i++)
    {
        vkDestroyBuffer(device, uniformBuffers[i], NULL);
        vkFreeMemory(device, uniformBufferMemory[i], NULL);
    }

    for (uint32_t i = 0; i < swapChainImageCount; i++)
    {
        vkDestroyImageView(device, swapChainImageViews[i], NULL);
    }
    free(swapChainImageViews);
    vkDestroySwapchainKHR(device, swapChain, NULL);

    // Cleanup: Logical Device, Surface, Vulkan Instance
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);

    // cleanup: glfw
    free(userData);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
