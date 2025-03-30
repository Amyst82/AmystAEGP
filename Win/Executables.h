#pragma once
#include <string>
#include "Win/Utils.h"

class IExecutable
{
public:
	virtual void Init() = 0;
	virtual void Execute() = 0;
	bool ExecutionRequired = false;
	std::string StringParam{};
};

class ApplyEffectByName : public IExecutable
{
public:
	void Init()
	{

	}
	void Execute()
	{
		std::string script = std::string("try{var selectedLayers = app.project.activeItem.selectedLayers;for(i = 0; i < selectedLayers.length;i++){selectedLayers[i].property('Effects').addProperty('") + StringParam + std::string("');}}catch(err){}");
		ExecuteScript(script.c_str());
		ExecutionRequired = false;
	}
};

class ApplyPresetByPath : public IExecutable
{
public:
	void Init()
	{

	}
	void Execute()
	{
		//Doubling the backslashes (After Effects applyPreset function requires that)
		std::string result;
		result.reserve(StringParam.length() * 2); // Maximum possible size

		for (char c : StringParam) 
		{
			result += c;
			if (c == '\\') 
			{
				result += '\\';
			}
		}
		StringParam = result;
		char buffer[1024];
		sprintf(buffer, "try{var selectedLayers = app.project.activeItem.selectedLayers;for(i = 0; i < selectedLayers.length;i++){var thePreset = File(\"%s\"); selectedLayers[i].applyPreset(thePreset);}}catch(err){}", StringParam.c_str());
		std::string script = std::string(buffer);
		ExecuteScript(script.c_str());
		ExecutionRequired = false;
	}
};

class ExecuteScriptExternally : public IExecutable
{
public:
	void Init()
	{

	}
	void Execute()
	{
		ExecuteScript(StringParam.c_str());
		ExecutionRequired = false;
	}
};