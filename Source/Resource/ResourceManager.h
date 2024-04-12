#pragma once

#include<map>
#include<fstream>
#include"spdlog/spdlog.h"

using std::map;

using RESOURCEID = UINT32;

/*
用以管理资源
管理资源实例
*/
class ResourceManager
{
private:
	RESOURCEID resourceID = 0;

public:
	void LoadResource(const char* path, const char* name);
	void UnLoadResource(const char*);
};


extern ResourceManager resourceManager;