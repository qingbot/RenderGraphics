#pragma once
#include<Vulkan/vulkan.h>
#include<memory>
#include<vector>
// 用以初始化RenderPass的Vulkan底层数据
// 存储指针，在运行期间就算发生了改变，指针本身也不会改变
struct VKInitData
{
	uint32_t width;
	uint32_t height;
	uint32_t curretFrameIndex;

	VkDevice* device;
	VkPhysicalDevice* physicalDevice;
	VkFormat* swapchainColorFormat;
	VkExtent2D* swapChainExtent;
	VkSwapchainKHR* swapchain;
	std::vector<VkImageView>* swapChainImageViews;
};

// 将会以RenderPass为单位进行渲染
// 一个RenderPass将包含Vulkan构建一个RenderPass和GraphicsPipeline所需要的所有信息
class RenderPassBase
{
public:
	RenderPassBase() = default;
	virtual ~RenderPassBase() = default;

	// 创建RenderPass
	// 创建Pipeline
	virtual void CreateVkData(const VKInitData*);
	// 清理
	virtual void CleanVK();

	virtual void ExcutePass(const VKInitData*, VkCommandBuffer& commandBuffer);


protected:
	VkRenderPass vkRenderPass;
	VkPipeline vkGraphicsPipeline;

	const VKInitData* vkInitData;

protected:
	// 创建RenderPass
	virtual void CreateVkRenderPass(const VKInitData*);

	// 创建GraphicsPipeline
	virtual void CreateVkGraphicsPipeline(const VKInitData*);

	// 创建CommandBuffer
	virtual void CreateCommandBuffer(const VKInitData*);

	// 创建Semaphores
	virtual void CreateSyncObjects(const VKInitData*);

public:
	static const VkPipelineMultisampleStateCreateInfo* Get1xMultisampleStateCreateInfo() {
		static VkPipelineMultisampleStateCreateInfo defaultMultisampleStateCreateInfo = {
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			nullptr,
			0,
			VK_SAMPLE_COUNT_1_BIT,
			VK_FALSE,
			1.0f,
			nullptr,
			VK_FALSE,
			VK_FALSE
		};
		return &defaultMultisampleStateCreateInfo;
	}

	static const VkPipelineInputAssemblyStateCreateInfo* GetTriangleListInputAssemblyStateCreateInfo() {
		static VkPipelineInputAssemblyStateCreateInfo defaultInputAssemblyStateCreateInfo = {
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			nullptr,
			0,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_FALSE
		};
		return &defaultInputAssemblyStateCreateInfo;
	}

	static const VkPipelineRasterizationStateCreateInfo* GetDefaultRasterizationStateCreateInfo() {
		static VkPipelineRasterizationStateCreateInfo defaultRasterizationStateCreateInfo = {
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			nullptr,
			0,
			VK_FALSE,
			VK_FALSE,
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_BACK_BIT,
			VK_FRONT_FACE_COUNTER_CLOCKWISE,
			VK_FALSE,
			0.0f,
			0.0f,
			0.0f,
			1.0f
		};
		return &defaultRasterizationStateCreateInfo;
	}

	static const VkPipelineColorBlendAttachmentState* GetDefaultColorBlendAttachmentState() {
		static VkPipelineColorBlendAttachmentState defaultColorBlendAttachmentState = {
			VK_FALSE,
			VK_BLEND_FACTOR_ONE,
			VK_BLEND_FACTOR_ZERO,
			VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE,
			VK_BLEND_FACTOR_ZERO,
			VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};
		return &defaultColorBlendAttachmentState;
	}

	static const VkPipelineColorBlendStateCreateInfo* GetDefaultColorBlendStateCreateInfo() {
		static VkPipelineColorBlendStateCreateInfo defaultColorBlendStateCreateInfo = {
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			nullptr,
			0,
			VK_FALSE,
			VK_LOGIC_OP_COPY,
			1,
			GetDefaultColorBlendAttachmentState(),
			{ 0.0f, 0.0f, 0.0f, 0.0f }
		};
		return &defaultColorBlendStateCreateInfo;
	}

	static const VkPipelineDepthStencilStateCreateInfo* GetDefaultDepthStencilStateCreateInfo() {
		static VkPipelineDepthStencilStateCreateInfo defaultDepthStencilStateCreateInfo = {
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			nullptr,
			0,
			VK_TRUE,
			VK_TRUE,
			VK_COMPARE_OP_LESS,
			VK_FALSE,
			VK_FALSE,
			{},
			{},
			0.0f,
			1.0f
		};
		return &defaultDepthStencilStateCreateInfo;
	}

};