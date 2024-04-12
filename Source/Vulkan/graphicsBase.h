#pragma once

#include<vector>
#include<set>
#include<optional>
#include"Vulkan/vulkan.h"
#include"glfw3.h"
#include"graphicsUtils.h"
#include"vertex.h"
#include"renderPassBase.h"
#include"shaderManager.h"
#include "spdlog/spdlog.h"

using std::vector;
using std::set;

struct QueueFamilyIndices
{
	 std::optional<uint32_t> graphicsFamily;
	 std::optional<uint32_t> presentFamily;

	 bool isComplete()
	 {
		 return graphicsFamily.has_value() && presentFamily.has_value();
	 }
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	vector<VkSurfaceFormatKHR> formats;
	vector<VkPresentModeKHR> presentModes;

	bool isSwapAequate()
	{
		return !formats.empty() && !presentModes.empty();
	}
};

class graphicsBase {
public:
	graphicsBase();
	graphicsBase(const graphicsBase&);
	graphicsBase(graphicsBase&&);
	graphicsBase& operator=(const graphicsBase&) = delete;
	virtual ~graphicsBase();

	bool InitGraphics(HWND hwnd, HINSTANCE hinstance, uint32_t width,uint32_t height, GLFWwindow* window);
	void Clearup();

	// 用以向专用显存提交数据
	void transformVertexBuffer();
	void drawFrame();
	void drawFrameWithPass();

	void EnqueueRenderPass(RenderPassBase* renderPass);

	const VKInitData* GetVKInitData() const;
	
private:
	uint32_t width, height;
	HWND hwnd;
	HINSTANCE hinstance;
	GLFWwindow* window;

	VkInstance vkinstance;
	vector<const char*> instanceLayers;
	vector<const char*> instanceExtensions;
	QueueFamilyIndices queueFamilyIndices;

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipelineShaderStageCreateInfo shaderStages[2];
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	VkPipelineViewportStateCreateInfo viewportState{};
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	VkPipelineMultisampleStateCreateInfo multisampling{};
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};

	vector<char> vertexShaderCode;
	VkShaderModule vertexShaderModule;
	vector<char> fragmentShaderCode;
	VkShaderModule fragShaderModule;

	VkPipeline graphicsPipeline;

	VkExtent2D swapChainExtent;
	VkFormat swapChainImageFormat;
	vector<VkImage> swapChainImages;
	vector<VkImageView> swapChainImageViews;

	VkQueue graphicsQueue;
	VkQueue presentQueue;						// 显示队列
	VkDebugUtilsMessengerEXT degbuUtilsMessage;	// debug接口
	VkSurfaceKHR surface;						// 系统接口

	VkDevice device;							// 逻辑设备
	VkSwapchainKHR swapchain;					// 交换链
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;	// 物理设备
	vector<VkQueueFamilyProperties> queueFamilies;		// 队列族
	
	vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	vector<VkCommandBuffer> commandBuffer;

	vector<VkSemaphore> imageAvailableSemaphore;
	vector<VkSemaphore> renderFinishedSemaphore;
	vector<VkFence> inFlightFence;

	std::optional<VKInitData> vkInitData;

	std::vector<RenderPassBase*> scriptableRenderPasses;

private:
	bool enableValidationLayers = true;	//在运行时会修改此值,运行后可通过检测此值来判断是否开启了验证层
	vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
	vector<const char*>  instanceExtension;/* = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};*/
	vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	std::vector<VkDynamicState> dynamicStates = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR
	};

	int MAX_FRAMES_IN_FLIGHT = 3;
	uint32_t currentFrame = 0;

	VkApplicationInfo appInfo;
	VkInstanceCreateInfo createInfo;

	VkDebugUtilsMessengerEXT debugMessenger;

	VkDebugReportCallbackEXT callBack;

	void queryQueueFamily(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkValidationLayerSupport();
	bool createDebugLayer();
	void destroyDebugLayer();
	void createLogicDevice();
	void createSurface();
	void destroySurface();
	void destroyLogicDevice();
	void createSwapChain(VkPhysicalDevice physicaDevice, VkDevice device, VkSurfaceKHR surface, const QueueFamilyIndices& queueFamily);
	void createImageView();
	void createFrameBuffer();
	void CreateGraphicsPipeline();
	void createRenderPass();

	void createCommandPool();
	void createCommandBuffer();
	void createSyncObjects();

	void recreateSwapChain();

	void createShaderModule(const vector<char>& code,VkShaderModule&);

	void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex);

	void createVKInitData();

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkQueueFlagBits queueflag);
	
private:
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
		
};