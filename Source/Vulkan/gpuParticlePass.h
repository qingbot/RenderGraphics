#pragma once
#include "renderPassBase.h"
#include "vertex.h"
#include "shaderManager.h"

class GpuParticlePass : public RenderPassBase
{
public:

	const char* vertexShaderName = "particleVert.spv";
	const char* fragmentShaderName = "particleFrag.spv";

	GpuParticlePass() = default;

	void CleanVK() override;
	void ExcutePass(const VKInitData*,VkCommandBuffer& commnadBuffer) override;

	void CreateVkData(const VKInitData*) override;
	void CreateBuffer(const VkCommandBuffer& commandBuffer) override;
	void ClearTempBuffer() override;


protected:
	VkPipelineLayout pipelineLayout;

	VkBuffer vertexBuffer;
	VkBuffer stagingBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkDeviceMemory stagingBufferMemory;

	VkShaderModule vertexShaderModule;
	VkShaderModule fragmentShaderModule;

	std::array<VertexPositionColor, 4> vertices;
};