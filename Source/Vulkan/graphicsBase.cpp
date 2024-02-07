#include "graphicsBase.h"
#include <stdexcept>
#include <iostream>
#include <numbers>

graphicsBase::graphicsBase()
{

}

graphicsBase::graphicsBase(const graphicsBase& other)
{

}

graphicsBase::graphicsBase(graphicsBase&& other)
{

}

graphicsBase::~graphicsBase()
{
	
}

bool graphicsBase::InitGraphics(HWND hwnd, HINSTANCE hinstance,uint32_t width,uint32_t height, GLFWwindow* window)
{

	this->hwnd = hwnd;
	this->hinstance = hinstance;
	this->width = width;
	this->height = height;
	this->window = window;

	//glfwInit();
	//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// 创建实例
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "Hello world";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Vulkan Engine";
	appInfo.apiVersion = VK_API_VERSION_1_0;

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (int i = 0; i < glfwExtensionCount; ++i)
	{
		instanceExtension.push_back(glfwExtensions[i]);
	}
	if (enableValidationLayers)
	{
		instanceExtension.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	 

	createInfo.enabledExtensionCount = instanceExtension.size();
	createInfo.ppEnabledExtensionNames = instanceExtension.data();
	createInfo.enabledLayerCount = 0;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &vkinstance);
	if (result != VK_SUCCESS)
		throw std::runtime_error("create vulkan instance failed");

	// 创建物理设备
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vkinstance, &deviceCount, nullptr);
	if (deviceCount == 0)
		throw std::runtime_error("failed to find gpu with vulkan support");

	vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(vkinstance, &deviceCount, devices.data());
	
	
	for(const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			break;
		}
	}
	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("faild to find a suitable GPU");
	}

	//// 创建队列
	//uint32_t queueFamilyCount = 0;
	//vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	//if (queueFamilyCount == 0)
	//	throw std::runtime_error("failed to find queue family");

	//queueFamilies.resize(queueFamilyCount);
	//vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	enableValidationLayers = checkValidationLayerSupport();

	createDebugLayer();
	createSurface();

	queryQueueFamily(physicalDevice,surface);

	createLogicDevice();
	createSwapChain(physicalDevice, device, surface, queueFamilyIndices);
	createImageView();
	// CreateGraphicsPipeline();
	// createRenderPass();
	// createFrameBuffer();
	createCommandPool();
	createCommandBuffer();
	createSyncObjects();
	createVKInitData();

	shaderManager.LoadShader(device);

	return true;
}

void graphicsBase::Clearup()
{
	// 等待栅栏结束
	for (int i = 0; i < inFlightFence.size(); ++i)
	{
		vkWaitForFences(device, 1, &inFlightFence[i], VK_TRUE, UINT64_MAX);
	}
	for (int i = 0; i < swapChainImageViews.size(); ++i)
	{
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}

	/*vkDestroyShaderModule(device, vertexShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);*/

	//for (int i = 0; i < swapChainFramebuffers.size(); ++i)
	//{
	//	vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
	//}
	for (int i = 0; i < imageAvailableSemaphore.size(); ++i)
		vkDestroySemaphore(device, imageAvailableSemaphore[i], nullptr);
	for(int i = 0; i < renderFinishedSemaphore.size(); ++i)
		vkDestroySemaphore(device, renderFinishedSemaphore[i], nullptr);
	for(int i =0;i < inFlightFence.size(); ++i)
		vkDestroyFence(device, inFlightFence[i], nullptr);

	shaderManager.DestroyShader();

	for (int i = 0; i < scriptableRenderPasses.size(); ++i)
		scriptableRenderPasses[i]->CleanVK();
	
	vkDestroyCommandPool(device, commandPool, nullptr);

	vkDestroySwapchainKHR(device, swapchain, nullptr);
	// vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	

	// vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	destroyDebugLayer();
	vkDestroyDevice(device, nullptr);
	// destroyLogicDevice();
	vkDestroySurfaceKHR(vkinstance, surface, nullptr);
	// destroySurface();
	vkDestroyInstance(vkinstance, nullptr);
	
}

// 判断给定物理设备是否支持拓展
static bool CheckDeviceExtensionSupport(VkPhysicalDevice physicDevice,const vector<const char*>& extensions)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicDevice, nullptr, &extensionCount, nullptr);

	vector<VkExtensionProperties> avaiableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicDevice, nullptr, &extensionCount, avaiableExtensions.data());

	set<std::string> requiredExtensions(extensions.begin(), extensions.end());
	/*for (const auto& c : requiredExtensions)
	{
		std::cout << c << std::endl;
	}*/
	for (const auto& extension : avaiableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}
	/*for (const auto& c : avaiableExtensions)
	{
		std::cout<< c.extensionName << std::endl;
	}*/
	return requiredExtensions.empty();
}

bool graphicsBase::isDeviceSuitable(VkPhysicalDevice device)
{

	return CheckDeviceExtensionSupport(device, deviceExtensions);	
}

bool graphicsBase::checkValidationLayerSupport()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for(const char* i : validationLayers)
	{
		for (const auto& t : availableLayers)
		{
			if(strcmp(t.layerName,i))
			return true;
		}
	}
	return false;
}

bool graphicsBase ::createDebugLayer()
{
	if (!enableValidationLayers)return false;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr; // Optional

	/*auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkinstance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) {
		func(vkinstance, callback, pAllocator);
	}*/

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkinstance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(vkinstance, &createInfo, nullptr, &debugMessenger);
		return true;
	}
	return false;
}

VKAPI_ATTR VkBool32 VKAPI_CALL graphicsBase::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}


void graphicsBase::destroyDebugLayer()
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkinstance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(vkinstance, debugMessenger, nullptr);
	}
}

QueueFamilyIndices graphicsBase::findQueueFamilies(VkPhysicalDevice device, VkQueueFlagBits queueflag = VK_QUEUE_GRAPHICS_BIT)
{
	// queueflag == VK_QUEUE_GRAPHICS_BIT 会自动拥有转换队列的能力
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if(queueFamily.queueFlags & queueflag)
		{
			indices.graphicsFamily = i;
		}
	}
	return indices;
}

void graphicsBase::createLogicDevice()
{
	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	set<uint32_t> uniqueQueueFamilies = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };
	
	float queuePriority = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	vector<const char*> logicDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkPhysicalDeviceFeatures deviceFeatures{};
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = logicDeviceExtensions.size();
	createInfo.ppEnabledExtensionNames = logicDeviceExtensions.data();
	VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
	vkGetDeviceQueue(device, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
}

void graphicsBase::createSurface()
{
	VkWin32SurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = hwnd;
	createInfo.hinstance = hinstance;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	
	if(vkCreateWin32SurfaceKHR(vkinstance, &createInfo, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface");
	
}

void graphicsBase::destroySurface()
{
	
}

void graphicsBase::queryQueueFamily(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkBool32 presentSupport = false;
	uint32_t i = 0;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
	if (presentSupport)
	{
		queueFamilyIndices.presentFamily = i;
	}
	
}

void graphicsBase::destroyLogicDevice()
{
	
}

SwapChainSupportDetails graphicsBase::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}

VkSurfaceFormatKHR graphicsBase::chooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& availableFormats)
{
	// TODO 从给定中选择一个最好的
	return availableFormats[0];
}

VkPresentModeKHR graphicsBase::chooseSwapPresentMode(const vector<VkPresentModeKHR>& availablePresentModes)
{
	// 从给定中选择一个最好的
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D graphicsBase::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	/*if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}*/
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	VkExtent2D actualExtent = {
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};
	/*VkExtent2D actualExtent = {
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};*/
	return actualExtent;
}

void graphicsBase::createSwapChain(VkPhysicalDevice physicaDevice, VkDevice device, VkSurfaceKHR surface, const QueueFamilyIndices& queueFamily)
{
	SwapChainSupportDetails details = querySwapChainSupport(physicaDevice, surface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(details.presentModes);
	VkExtent2D extent = chooseSwapExtent(details.capabilities);
	swapChainExtent = extent;

	swapChainImageFormat = surfaceFormat.format;

	uint32_t imageCount = details.capabilities.minImageCount + 1;

	if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
		imageCount = details.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR  createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // 忽略alpha通道
	// 如何跨队列使用交换链，比如图形队列和显示队列不同则就会有这种情况
	uint32_t queueFamilyIndices[] = { queueFamily.graphicsFamily.value(), queueFamily.presentFamily.value() };
	if (queueFamily.graphicsFamily != queueFamily.presentFamily) {
		// 图像可以在没有显式转移控制权的情况下跨队列使用
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		// 一个图像在一个队列族中所有权，必须在另一个队列族中显式转移所有权，性能更好
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;	
		//createInfo.queueFamilyIndexCount = 0; // Optional
		//createInfo.pQueueFamilyIndices = nullptr; // Optional
	}
	createInfo.preTransform = details.capabilities.currentTransform;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if(vkCreateSwapchainKHR(device,&createInfo, nullptr, &swapchain) != VK_SUCCESS)
		throw std::runtime_error("failed to create swap chain");

	uint32_t swapChainimageCount;
	vkGetSwapchainImagesKHR(device, swapchain, &swapChainimageCount, nullptr);
	swapChainImages.resize(swapChainimageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &swapChainimageCount, swapChainImages.data());

	//createImageView();
}

void graphicsBase::createImageView()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); ++i)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		// 切换颜色通道的映射， 使用默认
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// subresourceRange描述了Image的目的是什么，应该访问哪一部分
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		if(vkCreateImageView(device, &createInfo, nullptr, &(swapChainImageViews[i]))!= VK_SUCCESS)
			throw std::runtime_error("failed to create image views!");
	}

}

void graphicsBase::CreateGraphicsPipeline()
{
	GraphicsUtils::readFile("D:/project/DX11/Shader/Vert.spv", vertexShaderCode);
	GraphicsUtils::readFile("D:/project/DX11/Shader/Fragment.spv", fragmentShaderCode);

	createShaderModule(vertexShaderCode, vertexShaderModule);
	createShaderModule(fragmentShaderCode, fragShaderModule);

	vertexShaderCode.clear();
	fragmentShaderCode.clear();

	// VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vertexShaderModule;
	shaderStages[0].pName = "main";

	// VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = fragShaderModule;
	shaderStages[1].pName = "main";

	/*shaderStages[0] = vertShaderStageInfo;
	shaderStages[1] = fragShaderStageInfo;*/
	
	
	// VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	// 用于描述顶点输入的格式
	// VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	//vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	//vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	// 用以描述图元的渲染类型
	// VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;	// 允许重用图元
	
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)width;
	viewport.height = (float)height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;
	/*
	// 如果是Dynamic Pipeline State 且设置了VK_DYNAMIC_STATE_VIEWPORT，VK_DYNAMIC_STATE_SCISSOR
	// 那么就只需要设置二者的数量即可
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	*/
	// 否则就是静态的视口，就需要指定视口状态  视口可以指定多个，一次性往多个视口渲染
	// VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// 光栅化状态
	// VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;			// 如果设置为true，那么在远近裁剪面之外的顶点将被设置为远近裁剪面而不是丢弃 shadowmap可以开启
	rasterizer.rasterizerDiscardEnable = VK_FALSE;	// 如果为True，几何则永不通过光栅化 锁死FrameBuffer
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;	// 填充模式
	rasterizer.lineWidth = 1.0f;					// 线宽
	rasterizer.cullMode = VK_CULL_MODE_NONE;	// 剔除模式
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;	// 指定何为正面

	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	// MSAA
	// VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	// 配置 Depth 和 stencil
	// VkPipelineDepthStencilStateCreateInfo

	// 配置混合阶段
	// VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	// VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	// 常量缓冲区
	// VkPipelineLayout pipelineLayout;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
	
}

void graphicsBase::createShaderModule(const vector<char>& code, VkShaderModule& shaderModule)
{
	VkShaderModuleCreateInfo  createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}
	//return shaderModule;
}

void graphicsBase::createRenderPass()
{
	VkAttachmentDescription colorAttachment;
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	colorAttachment.flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
	
	// 附着在subpass上的引用
	VkAttachmentReference colorAttachmentRef;
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// The index of the attachment in this array is directly referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive!
	/*
	*	pInputAttachments: Attachments that are read from a shader
		pResolveAttachments: Attachments used for multisampling color attachments
		pDepthStencilAttachment: Attachment for depth and stencil data
		pPreserveAttachments: Attachments that are not used by this subpass, but for which the data must be preserved
	*/
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	// 指定subPass依赖
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;	// 前后的隐式subpass
	dependency.dstSubpass = 0;	// dstSubPass 必须始终高于srcSubPass，否则会出现循环依赖
	// 等待的阶段
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create Pipelines!");
	}


}

void graphicsBase::createFrameBuffer()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		VkImageView attachments[] = {
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void graphicsBase::createCommandPool()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void graphicsBase::createCommandBuffer()
{
	commandBuffer.resize(MAX_FRAMES_IN_FLIGHT);
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffer.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void graphicsBase::recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//beginInfo.flags = 0; // Optional
	// beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChainExtent;

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;
	

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	// dynamciy stage
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void graphicsBase::createSyncObjects()
{
	imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFence.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.pNext = nullptr;
	
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	fenceInfo.pNext = nullptr;
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create semaphores!");
		}
	}
}

void graphicsBase::drawFrame()
{
	vkWaitForFences(device, 1, &inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);
	
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
			return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	vkResetFences(device, 1, &inFlightFence[currentFrame]);

	vkResetCommandBuffer(commandBuffer[currentFrame], 0);
	recordCommandBuffer(commandBuffer[currentFrame], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// 指定等待哪个信号量，以及在哪个阶段等待，Vulkan可以指定等待的阶段
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore[currentFrame]};
	// 指定等待的阶段，在颜色输出阶段之前等待，也就是说，我们可以在RT还未准备好时发起渲染，此时只会执行到VertexShader
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer[currentFrame];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	 
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	//presentInfo.pResults = nullptr; // Optional

	// 将指定交换链的FrameBuffer提交到显示队列
	vkQueuePresentKHR(presentQueue, &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void graphicsBase::recreateSwapChain()
{
	vkDeviceWaitIdle(device);
	/*
	for (int i = 0; i < inFlightFence.size(); ++i)
	{
		vkWaitForFences(device, 1, &inFlightFence[i], VK_TRUE, UINT64_MAX);
	}*/

	for (auto frameBuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(device, frameBuffer, nullptr);
	}
	for(int i = 0; i < swapChainImageViews.size(); ++i)
	{
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	
	createSwapChain(physicalDevice, device, surface, queueFamilyIndices);
	createImageView();
	createFrameBuffer();

	createVKInitData();
}

void graphicsBase::createVKInitData()
{
	vkInitData = {
		width,
		height,
		0,
		&device,
		&physicalDevice,
		&swapChainImageFormat,
		&swapChainExtent,
		&swapchain,
		&swapChainImageViews
	};
}

const VKInitData* graphicsBase::GetVKInitData() const
{
	if (!vkInitData.has_value())
		throw std::runtime_error("GetVKInitData must call after VKInit");
	return &vkInitData.value();
}

void graphicsBase::EnqueueRenderPass(RenderPassBase* renderPass)
{
	renderPass->CreateVkData(&vkInitData.value());
	scriptableRenderPasses.push_back(renderPass);
}

void graphicsBase::drawFrameWithPass()
{
	// 等待上一帧结束
	vkWaitForFences(device, 1, &inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);

	// 获取这一帧的交换链的索引
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	// 开始本帧
	vkResetFences(device, 1, &inFlightFence[currentFrame]);
	vkResetCommandBuffer(commandBuffer[currentFrame], 0);

	VkCommandBuffer& currentCommandBuffer = commandBuffer[currentFrame];
	//VkFramebuffer& currentFrameBuffer = swapChainFramebuffers[currentFrame];
	
	vkInitData.value().curretFrameIndex = currentFrame;
	
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//beginInfo.flags = 0; // Optional
	// beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(commandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	for (int i = 0; i < scriptableRenderPasses.size(); ++i)
	{
		scriptableRenderPasses[i]->ExcutePass(&vkInitData.value(), currentCommandBuffer);
	}
	
	if(vkEndCommandBuffer(commandBuffer[currentFrame])!=VK_SUCCESS)
		throw std::runtime_error("failed to record command buffer!");

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// 指定等待哪个信号量，以及在哪个阶段等待，Vulkan可以指定等待的阶段
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore[currentFrame] };
	// 指定等待的阶段，在颜色输出阶段之前等待，也就是说，我们可以在RT还未准备好时发起渲染，此时只会执行到VertexShader
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer[currentFrame];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	// 提交到显示队列
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	//presentInfo.pResults = nullptr; // Optional

	// 将指定交换链的FrameBuffer提交到显示队列
	vkQueuePresentKHR(presentQueue, &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void graphicsBase::transformVertexBuffer()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	for (int i = 0; i < scriptableRenderPasses.size(); ++i)
	{
		scriptableRenderPasses[i]->CreateBuffer(commandBuffer);
	}

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.commandBufferCount = 1;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

	for (int i = 0; i < scriptableRenderPasses.size(); ++i)
	{
		scriptableRenderPasses[i]->ClearTempBuffer();
	}
}