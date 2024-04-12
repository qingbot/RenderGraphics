#include "renderPassBase.h"


void RenderPassBase::CreateVkData(const VKInitData* vkData)
{
	this->vkInitData = vkData;
}

void RenderPassBase::CleanVK()
{
}

// void RenderPassBase::CreateVkRenderPass(const VKInitData* vkData)
// {
// }
//
// void RenderPassBase::CreateVkGraphicsPipeline(const VKInitData* vkData)
// {
// }

void RenderPassBase::CreateCommandBuffer(const VKInitData* vkData)
{
}

void RenderPassBase::CreateSyncObjects(const VKInitData* vkData)
{
}

void RenderPassBase::ExcutePass(const VKInitData*, VkCommandBuffer& commandBuffer)
{}

void RenderPassBase::CreateBuffer(const VkCommandBuffer& commandBuffer)
{}

void RenderPassBase::ClearTempBuffer() 
{}