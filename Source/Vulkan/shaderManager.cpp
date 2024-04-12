#include "shaderManager.h"

ShaderManager shaderManager;

void ShaderManager::LoadShader(const VkDevice& device)
{
	this->device = device;
	for (const auto& entry : std::filesystem::directory_iterator(shaderPath))
	{
		if (entry.path().extension() == SHADER_FILE_EXTENSION)
		{

			if (allShaders.find(entry.path().filename().string().c_str()) != allShaders.end())
			{
				spdlog::info("ShaderManager: {0} already exist!", entry.path().filename().string());
				continue;
			}
			vector<char> data;
			GraphicsUtils::readFile(entry.path().string(), data);
			VkShaderModuleCreateInfo createInfo{};
			createInfo.codeSize = data.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.pNext = nullptr;
			
			VkShaderModule shaderModule;
			if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			{
				spdlog::error("ShaderManager:: {} failed to create shader module", entry.path().filename().string());
			}
			else {
				spdlog::info("ShaderManager::{} create shader module success!", entry.path().filename().string());
				allShaders[entry.path().filename().string()] = shaderModule;
			}
			data.clear();
		}
	}
}

void ShaderManager::DestroyShader()
{
	for (auto& shader : allShaders)
	{
		vkDestroyShaderModule(device, shader.second, nullptr);
	}
}

ShaderManager::ShaderManager()
{
	spdlog::info("ShaderManager::ShaderManager()");
}

ShaderManager::~ShaderManager()
{
	spdlog::info("ShaderManager::~ShaderManager()");
}

VkShaderModule ShaderManager::GetShaderModule(const char* shaderName) const
{
	auto it = allShaders.find(shaderName);
	if (it != allShaders.end())
	{
		return it->second;
	}
	else
	{
		spdlog::error("ShaderManager::{} not found!", shaderName);
		return VK_NULL_HANDLE;
	}
}