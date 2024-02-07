#pragma once


#include "renderPassBase.h"
#include "vertex.h"
#include"graphicsUtils.h"
#include"shaderManager.h"
#include <map>

class vertexRenderPass : public RenderPassBase
{
public:
	vertexRenderPass():RenderPassBase(){}

	void CleanVK() override;

	void ExcutePass(const VKInitData*, VkCommandBuffer& commandBuffer) override;

	void CreateBuffer(const VkCommandBuffer& commandBuffer) override;
	void ClearTempBuffer() override;

protected:
	VkVertexInputBindingDescription bindingDescription = VertexPositionColor::getBindingDescription();
	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = VertexPositionColor::getAttributeDescription();
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

	VkPipelineLayout pipelineLayout;
	
	VkBufferCreateInfo bufferInfo{};
	VkBuffer vertexBuffer;
	VkBuffer stagingBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkDeviceMemory stagingBufferMemory;
	std::array<VertexPositionColor,3> vertices;

	std::map< uint32_t, VkFramebuffer> frameBuffers;
	// VkFramebuffer frameBuffer;

	VkShaderModule vertexShaderModule;
	VkShaderModule fragmentShaderModule;
	// VkShaderModule 
	
	void CreateVkData(const VKInitData*) override;

	void CreateVkRenderPass(const VKInitData*) override;

	void CreateVkGraphicsPipeline(const VKInitData*) override;

	void CreateFrameBuffer(const VKInitData*);
};