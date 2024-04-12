#include "gpuParticlePass.h"
#include "graphicsUtils.h"

void GpuParticlePass::CreateVkData(const VKInitData* vkData)
{
	RenderPassBase::CreateVkData(vkData);

	vertices[0] = { {0.0f, -0.5f}, {1.0f, 0.0f, 0.0f} };
	vertices[1] = { {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f} };
	vertices[2] = { {-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} };

	GraphicsUtils::createBuffer(*vkData->device, *vkData->physicalDevice, sizeof(VertexPositionColor) * vertices.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

	GraphicsUtils::createBuffer(*vkData->device,*vkData->physicalDevice, sizeof(VertexPositionColor) * vertices.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(*vkData->device, stagingBufferMemory, 0, sizeof(VertexPositionColor) * vertices.size(), 0, &data);
	// TODO 在这里赋予data数据
	// memecpy
	vkUnmapMemory(*vkData->device, stagingBufferMemory);

	vertexShaderModule = shaderManager.GetShaderModule(vertexShaderName);
	fragmentShaderModule = shaderManager.GetShaderModule(fragmentShaderName);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(*vkData->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

}

void GpuParticlePass::ClearTempBuffer()
{
	vkDestroyBuffer(*vkInitData->device, stagingBuffer, nullptr);
	vkFreeMemory(*vkInitData->device, stagingBufferMemory, nullptr);
}

void GpuParticlePass::CreateBuffer(const VkCommandBuffer& commandBuffer)
{
	VkBufferCopy copyRegion = {};
	copyRegion.dstOffset = 0;
	copyRegion.srcOffset = 0;
	copyRegion.size = sizeof(VertexPositionColor) * vertices.size();
	vkCmdCopyBuffer(commandBuffer, stagingBuffer, vertexBuffer, 1, &copyRegion);
}

// https://unity.com/releases/editor/archive