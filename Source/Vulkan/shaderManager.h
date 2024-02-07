#pragma once
#ifndef SHADER_MANAGER_INCLUDE
#define SHADER_MANAGER_INCLUDE

#include<vector>
#include<map>
#include<filesystem>
#include<iostream>
#include<string>
#include<Vulkan/vulkan.h>
#include"graphicsUtils.h"

using std::map;

#define MAX_SHADER_NAME_LENGTH 64

static const char* shaderPath = "D:/project/DX11/Shader/";

class ShaderManager 
{
public:

	ShaderManager(const ShaderManager& other) = delete;
	ShaderManager(ShaderManager&& other) = delete;

	ShaderManager();
	~ShaderManager();

	void LoadShader(const VkDevice& device);
	void DestroyShader();

	VkShaderModule GetShaderModule(const char* shaderName) const;
	
private:
	std::map<std::string, VkShaderModule> allShaders;
	VkDevice device;
};
extern ShaderManager shaderManager;
// ShaderManager shaderManager;
#endif // !SHADER_MANAGER_INCLUDE
