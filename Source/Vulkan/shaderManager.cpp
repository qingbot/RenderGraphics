#include "shaderManager.h"

ShaderManager shaderManager;

void ShaderManager::LoadShader(const VkDevice& device)
{
	this->device = device;
	for (const auto& entry : std::filesystem::directory_iterator(shaderPath))
	{
		if (entry.path().extension() == ".spv")
		{

			if (allShaders.find(entry.path().filename().string().c_str()) != allShaders.end())
			{
				std::cout << "ShaderManager: " << entry.path().filename() << " already exist!" << std::endl;
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
				std::cout<< "ShaderManager::"<<entry.path().filename() << " failed to create shader module!" << std::endl;
			}
			else {
				std::cout<< "ShaderManager::"<<entry.path().filename().string() << " create shader module success!" << std::endl;
				allShaders[entry.path().filename().string()] = shaderModule;
				// allShaders.insert({ entry.path().filename().string().c_str() ,shaderModule });
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
	std::cout<< "ShaderManager::ShaderManager()" << std::endl;
}

ShaderManager::~ShaderManager()
{
	std::cout<< "ShaderManager::~ShaderManager()" << std::endl;
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
		std::cout << "ShaderManager::" << shaderName << " not found!" << std::endl;
		return VK_NULL_HANDLE;
	}
}