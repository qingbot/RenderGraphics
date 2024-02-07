#pragma once
#ifndef GRAPHICS_UTILS_H
#define GRAPHICS_UTILS_H

#include<vector>
#include<fstream>
#include<string>
#include<Vulkan/vulkan.h>

using std::vector;
using std::ifstream;
using std::string;

namespace GraphicsUtils
{

	void readFile(const string& fileName, vector<char>& data);
	
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);

	void createShaderModule(VkDevice device, const vector<char>& code, VkShaderModule& shaderModule);

	void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size,
		VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
}
#endif // !GRAPHICS_UTILS_H