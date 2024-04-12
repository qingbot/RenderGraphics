#include "vertexRenderPass.h"
#include <stdexcept>


void vertexRenderPass::CreateVkData(const VKInitData* vkData) 
{
	vertices[0] = { {0.0f, -0.5f}, {1.0f, 0.0f, 0.0f} };
	vertices[1] = { {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f} };
	vertices[2] = { {-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} };

	vkInitData = vkData;
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = 2;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(VertexPositionColor) * vertices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;	// 一个Buffer可以被多个队列拥有

	GraphicsUtils::createBuffer(*vkData->device, *vkData->physicalDevice, sizeof(VertexPositionColor) * vertices.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	// 通过Map的方式将数据拷贝到显存中，但是并不会立即执行，因为可能正在Caching
	/*
	*	You can now simply memcpy the vertex data to the mapped memory and unmap it again using vkUnmapMemory.
		Unfortunately the driver may not immediately copy the data into the buffer memory, for example because of caching.
		It is also possible that writes to the buffer are not visible in the mapped memory yet. There are two ways to deal with that problem:
		Use a memory heap that is host coherent, indicated with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		Call vkFlushMappedMemoryRanges after writing to the mapped memory, and call vkInvalidateMappedMemoryRanges before reading from the mapped memory
	*/
	void* data;
	vkMapMemory(*vkData->device, stagingBufferMemory, 0, bufferInfo.size, 0, &data);
	memcpy(data, vertices.data(), bufferInfo.size);
	vkUnmapMemory(*vkData->device, stagingBufferMemory);

	GraphicsUtils::createBuffer(*vkData->device, *vkData->physicalDevice, sizeof(VertexPositionColor) * vertices.size(),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer, vertexBufferMemory);

	vertexShaderModule = shaderManager.GetShaderModule("vert.spv");
	fragmentShaderModule = shaderManager.GetShaderModule("fragment.spv");

	// VkPipelineLayout pipelineLayout;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if(vkCreatePipelineLayout(*vkData->device, &pipelineLayoutInfo,nullptr, &pipelineLayout)!=VK_SUCCESS)
	{
		throw std::runtime_error("create pipeline layout error");
	}
	
	CreateVkRenderPass(vkData);
	CreateVkGraphicsPipeline(vkData);
	vkDestroyPipelineLayout(*vkData->device, pipelineLayout, nullptr);
}

void vertexRenderPass::CleanVK()
{
	vkDestroyRenderPass(*vkInitData->device, vkRenderPass, nullptr);
	vkDestroyPipeline(*vkInitData->device, vkGraphicsPipeline, nullptr);
	vkDestroyBuffer(*vkInitData->device, vertexBuffer, nullptr);
	vkFreeMemory(*vkInitData->device, vertexBufferMemory, nullptr);
	for(std::map<uint32_t,VkFramebuffer>::iterator i = frameBuffers.begin(); i != frameBuffers.end(); i++)
	{
		vkDestroyFramebuffer(*vkInitData->device, i->second, nullptr);
	}
}

void vertexRenderPass::CreateVkRenderPass(const VKInitData* vkData)
{
	VkAttachmentDescription attachmentDescription{};
	/*attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescription.flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
	attachmentDescription.format = *vkInitData->swapchainColorFormat;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;*/
	attachmentDescription.format = *vkInitData->swapchainColorFormat;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescription.flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;

	VkAttachmentReference colorAttachmentReference{};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDependency subpassDependency{};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;	// 0表示第一个subPass，表示第一个subPass依赖于外部的subPass
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;	// 表示依赖subpass的颜色输出阶段
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDescription subpassDescription{};
	subpassDescription.colorAttachmentCount = 1;
	//subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.pColorAttachments = &colorAttachmentReference;
	
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &attachmentDescription;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &subpassDependency;
	renderPassInfo.pNext = nullptr;
	
	if (vkCreateRenderPass(*vkData->device, &renderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS)
		throw std::runtime_error("create render pass error");
}

void vertexRenderPass::CreateVkGraphicsPipeline(const VKInitData* vkData)
{
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)vkData->width;
	viewport.height = (float)vkData->height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = *vkData->swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo
	{
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,
		0,
		1,
		&viewport,
		1,
		&scissor
	};

	VkPipelineShaderStageCreateInfo shaderStageInfo[2];
	shaderStageInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo[0].pName = "main";
	shaderStageInfo[0].module = vertexShaderModule;
	shaderStageInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageInfo[0].pNext = nullptr;
	shaderStageInfo[0].pSpecializationInfo = nullptr;

	shaderStageInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo[1].pName = "main";
	shaderStageInfo[1].module = fragmentShaderModule;
	shaderStageInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageInfo[1].pNext = nullptr;
	shaderStageInfo[1].pSpecializationInfo = nullptr;
	
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.pNext = nullptr;
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStageInfo;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = GetTriangleListInputAssemblyStateCreateInfo();
	pipelineInfo.pViewportState = &viewportStateCreateInfo;
	pipelineInfo.pRasterizationState = GetDefaultRasterizationStateCreateInfo();
	pipelineInfo.pMultisampleState = Get1xMultisampleStateCreateInfo();
	pipelineInfo.pDepthStencilState = GetDefaultDepthStencilStateCreateInfo();
	pipelineInfo.pColorBlendState = GetDefaultColorBlendStateCreateInfo();
	pipelineInfo.pDynamicState = nullptr;
	
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = vkRenderPass;
	pipelineInfo.subpass = 0;		// 这个是指定渲染pass的subpass，如果只有一个subpass，那么就是0
	
	if (vkCreateGraphicsPipelines(*vkData->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkGraphicsPipeline) != VK_SUCCESS)
		throw std::runtime_error("create graphics pipeline error");
}

void vertexRenderPass::CreateFrameBuffer(const VKInitData* vkData)
{
	if (frameBuffers.find(vkData->curretFrameIndex) != frameBuffers.end())
	{
		return;
	};

	VkImageView attachments[] = { (*vkData->swapChainImageViews)[vkData->curretFrameIndex]};
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = vkRenderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = vkData->width;
	framebufferInfo.height = vkData->height;
	framebufferInfo.layers = 1;

	VkFramebuffer frameBuffer;

	if (vkCreateFramebuffer(*vkData->device, &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS)
		throw std::runtime_error("create framebuffer error");

	frameBuffers.insert({ vkData->curretFrameIndex, frameBuffer });
}

void vertexRenderPass::CreateBuffer(const VkCommandBuffer& commandBuffer)
{
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = sizeof(VertexPositionColor) * vertices.size();
	vkCmdCopyBuffer(commandBuffer, stagingBuffer, vertexBuffer, 1, &copyRegion);
}

void vertexRenderPass::ClearTempBuffer()
{
	vkDestroyBuffer(*vkInitData->device, stagingBuffer, nullptr);
	vkFreeMemory(*vkInitData->device, stagingBufferMemory, nullptr);
}

void vertexRenderPass::ExcutePass(const VKInitData* vkInitData, VkCommandBuffer& commandBuffer)
{
	CreateFrameBuffer(vkInitData);

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.framebuffer = frameBuffers[vkInitData->curretFrameIndex];
	renderPassBeginInfo.clearValueCount = 1;
	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassBeginInfo.pClearValues = &clearColor;
	renderPassBeginInfo.renderPass = vkRenderPass;
	renderPassBeginInfo.renderArea = { 0, 0, vkInitData->width, vkInitData->height };
	VkDeviceSize offsets[] = { 0 };
	
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkGraphicsPipeline);
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
	
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);
}
