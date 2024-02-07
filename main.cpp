#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <iostream>
// #include "Source/DX11Content.h"
#include "thirdPart/glfw/include/glfw3.h"
#include"thirdPart/glfw/include/glfw3native.h"
#include "vulkan/vulkan.h"
#include "Source/Vulkan/graphicsBase.h"
#include "Source/Vulkan/vertexRenderPass.h"
// using namespace std;

void InitVulkan();
void windowFocusCallback(GLFWwindow* window, int focused);

graphicsBase graphics;
GLFWwindow* window;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
    glfwSetWindowFocusCallback(window, windowFocusCallback);

    uint32_t extensionCount = 0;

    auto pMonitor = glfwGetPrimaryMonitor();
    
    InitVulkan();
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    
    vertexRenderPass vertexRenderPass;
    graphics.EnqueueRenderPass(&vertexRenderPass);

    graphics.transformVertexBuffer();

    std::cout << "start draw" << std::endl;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        graphics.drawFrameWithPass();
        // graphics.drawFrame();
    }
    
    // vertexRenderPass.CleanVK();
    glfwDestroyWindow(window);
    glfwTerminate();
    graphics.Clearup();
    return 0;
}

void InitVulkan()
{
    int width,  height;
    glfwGetFramebufferSize(window, &width, &height);

    bool r = graphics.InitGraphics(
        glfwGetWin32Window(window), GetModuleHandle(nullptr),static_cast<uint32_t>(width),static_cast<uint32_t>(height),window);
    //std::cout << r << std::endl;
}

void windowFocusCallback(GLFWwindow* window, int focused)
{
	if (focused)
	{
		std::cout << "window focused" << std::endl;
	}
	else
	{
		std::cout << "window unfocused" << std::endl;
	}
}