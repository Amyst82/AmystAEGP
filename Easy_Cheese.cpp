#include "../../../../cpp-httplib-0.19.0/httplib.h"
#include <iostream>
#include "../../../../../Downloads/json.hpp"
#include <memory>
#include "Easy_Cheese.h"
#include <filesystem>
#include <Windows.h>
#include <vector>
#include <string>
#include <mutex>
#include <queue>
#include "Win/Utils.h"
#include "Win/Executables.h"

using json = nlohmann::json;


using namespace std;
using namespace httplib;
namespace fs = std::filesystem;

std::vector<std::string> effectList;
std::vector<std::string> presetFiles;

inline static std::vector<IExecutable*> executables{};
inline static ApplyEffectByName ApplyEffectByName_e;
inline static ExecuteScriptExternally ExecuteScriptExternally_e;
inline static ApplyPresetByPath ApplyPresetByPath_e;
inline static std::map<A_short, std::string> AeVersions = 
{ 
	{(A_short)115, "After Effects CC 2018"},
	{(A_short)116, "After Effects 2019"},
	{(A_short)117, "After Effects 2020"},
	{(A_short)118, "After Effects 2021"},
	{(A_short)122, "After Effects 2022"},
	{(A_short)123, "After Effects 2023"},
	{(A_short)124, "After Effects 2024"},
	{(A_short)125, "After Effects 2025"}
};

static std::string getCurrentDateTimeString()
{
	// Get the current time
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	// Format the time as a string (e.g., "2023-10-05_14-30-45")
	std::stringstream ss{};
	ss << std::put_time(std::localtime(&in_time_t), "%B %d %Y at %I:%M %p");
	return ss.str();
}

std::string GetUsernameFromEnv() 
{
	char* username = nullptr;
	size_t len = 0;

	// Try USERNAME (Windows) first
	_dupenv_s(&username, &len, "USERNAME");
	if (!username) 
	{
		// Try USER (Unix-like)
		_dupenv_s(&username, &len, "USER");
	}

	std::string result(username ? username : "");
	free(username);
	return result;
}

static void getEffectList()
{
	AEGP_SuiteHandler suites(sP);
	AEGP_InstalledEffectKey dummy = AEGP_InstalledEffectKey_NONE;
	effectList.clear();

	while(true)
	{
		suites.EffectSuite3()->AEGP_GetNextInstalledEffect(dummy, &dummy);
		if (dummy == AEGP_InstalledEffectKey_NONE)
			break;
		A_char name[256];
		suites.EffectSuite3()->AEGP_GetEffectName(dummy, name);

		A_char mname[256];
		suites.EffectSuite3()->AEGP_GetEffectMatchName(dummy, mname);

		A_char cname[256];
		suites.EffectSuite3()->AEGP_GetEffectCategory(dummy, cname);

		effectList.push_back(std::string(name) + ";" + std::string(mname)+";"+std::string(cname));
	}
}

std::vector<fs::path> find_files_by_extension(const fs::path& root, const std::string& ext) 
{
	std::vector<fs::path> results;
	try 
	{
		for (const auto& entry : fs::recursive_directory_iterator(root)) 
		{
			if (entry.is_regular_file() && entry.path().extension() == ext) 
			{
				results.push_back(entry.path());
			}
		}
	}
	catch (const fs::filesystem_error& e) 
	{
		std::cerr << "Error accessing " << root << ": " << e.what() << '\n';
	}

	return results;
}

static bool GetPresetsList()
{
	try
	{
		AEGP_SuiteHandler suites(sP);
		A_short min;
		A_short max;
		suites.UtilitySuite6()->AEGP_GetDriverImplementationVersion(&max, &min);
		std::string a = AeVersions[max];

		char path[2048];
		GetModuleFileNameA(NULL, path, 2048);
		std::string MainPath(path);
		std::vector<std::string> paths = 
		{
			"C:\\Users\\" + GetUsernameFromEnv() + "\\Documents\\Adobe\\" + a,
			MainPath.substr(0, MainPath.length() - 11).append("Presets\\")
		};
		for (const auto& path : paths)
		{
			for (const auto& entry : find_files_by_extension(path, ".ffx"))
			{
				presetFiles.push_back(entry.string());
			}
		}
		return true;
	}
	catch (const char* err)
	{
		return false;
	}
}

int GetEffectsCount()
{
	A_Err err = A_Err_NONE;
	AEGP_SuiteHandler suites(sP);
	A_long numEffects = 0;
	err = suites.EffectSuite3()->AEGP_GetNumInstalledEffects(&numEffects);
	return (int)numEffects;
}

DWORD WINAPI mainThread(LPVOID param)
{
	Server svr;
	svr.Get("/api/CheckAlive", [](const Request& req, Response& res)
		{
			res.status = 200;
		});

	svr.Get("/api/ApplyEffect", [](const Request& req, Response& res) 
		{
			std::string name = req.get_param_value("name");
			ApplyEffectByName_e.StringParam = name;
			ApplyEffectByName_e.ExecutionRequired = true;
			res.status = 200;
		});

	svr.Get("/api/ApplyPreset", [](const Request& req, Response& res)
		{
			std::string path = req.get_param_value("path");
			ApplyPresetByPath_e.StringParam = path;
			ApplyPresetByPath_e.ExecutionRequired = true;
			res.status = 200;
		});

	svr.Get("/api/FxList", [](const Request& req, Response& res)
		{
			std::string response{};
			for (int i = 0; i < effectList.size(); i++)
			{
				response += effectList[i] + "\n";
			}
			res.set_content(response, "text/plain");
			res.status = 200;
		});
	svr.Get("/api/PresetsList", [](const Request& req, Response& res)
		{
			if (!GetPresetsList())
				res.status = 400;
			else
			{
				std::string response{};
				for (int i = 0; i < presetFiles.size(); i++)
				{
					response += presetFiles[i] + "\n";
				}
				res.set_content(response, "text/plain");
				res.status = 200;
			}
		});
	// POST endpoint
	svr.Post("/api/ExecuteScript", [](const Request& req, Response& res) 
		{
			ExecuteScriptExternally_e.StringParam = req.body;
			ExecuteScriptExternally_e.ExecutionRequired = true;
			res.status = 200;
		});
	// Start the server on port 13377
	std::cout << "Server is running on http://localhost:13377" << std::endl;
	svr.listen("localhost", 13377);
	return 0;
}
void HandleInternalRequests()
{
	for (auto& executable : executables)
	{
		if (executable->ExecutionRequired)
		{
			executable->Execute();
		}
	}
}
bool renderStarted = false;


A_Err PostRenderActions(bool success)
{

	A_Err err = A_Err_NONE;

	char path[2048];
	GetModuleFileNameA(NULL, path, 2048);
	std::string MainPath(path);
	std::string JsonPath = MainPath.substr(0, MainPath.length() - 11).append("Plug-ins\\Amyst\\settings.json");
	if (!std::filesystem::exists(JsonPath))
	{
		return err;
	}
	std::ifstream file(JsonPath);
	json data = json::parse(file);

	std::string accountKey = data.value("AlertzyKey", "");
	bool notification = data["PostRenderActions"].value("Notification", false);
	bool PurgeSave = data["PostRenderActions"].value("PurgeSave", false);	
	bool PurgeSaveQuit = data["PostRenderActions"].value("PurgeSaveQuit", false);	
	bool PurgeSaveShutdown = data["PostRenderActions"].value("PurgeSaveShutdown", false);


	std::string message = std::string("Render finished on ") + getCurrentDateTimeString();
	if(!success)
		message = std::string("Render stopped due to an error on ") + getCurrentDateTimeString();

	if (PurgeSave)
	{
		std::string script = "var cmdId = app.findMenuCommandId('AllMemoryDiskCache'); app.executeCommand(cmdId); cmdId = app.findMenuCommandId('Save'); app.executeCommand(cmdId);";
		ExecuteScript(script.c_str());
		cout << "Purging and saving project!" << endl;

		if (!success)
			message = std::string("Render stopped due to an error on ") + getCurrentDateTimeString() + "! Purging and saving!";
		else
			message = std::string("Render finished on ") + getCurrentDateTimeString() + "! Purging and saving!";
	}
	if (PurgeSaveQuit)
	{
		std::string script = "app.purge(PurgeTarget.ALL_CACHES); app.executeCommand(5); ";
		ExecuteScript(script.c_str());
		cout << "Purging and saving project, then quitting!" << endl;

		if (!success)
			message = std::string("Render stopped due to an error on ") + getCurrentDateTimeString() + "! Purging and saving then quitting!";
		else
			message = std::string("Render finished on ") + getCurrentDateTimeString() + "! Purging and saving then quitting!";
	}
	
	if (notification)
	{
		std::string script = "cmd = 'curl -X POST https://alertzy.app/send \\ -d \"accountKey=" + accountKey + "\" \\ -d \"title=After Effects\" \\ -d \"message=" + message + "\"';" + "\nsystem.callSystem(cmd);";
		ExecuteScript(script.c_str());

	}

	if (PurgeSaveShutdown)
	{
		std::string script = "app.purge(PurgeTarget.ALL_CACHES); app.executeCommand(5); ";
		ExecuteScript(script.c_str());
		cout << "Purging and saving project, then shutting down the PC!" << endl;

		if (!success)
			message = std::string("Render stopped due to an error on ") + getCurrentDateTimeString() + "! Purging,saving & shutting PC down lol!";
		else
			message = std::string("Render finished on ") + getCurrentDateTimeString() + "! Purging,saving & shutting PC down lol! ";

		std::string script2 = "cmd = 'shutdown -s -t 3';\nsystem.callSystem(cmd);";
		ExecuteScript(script2.c_str());

		HANDLE hProcess = GetCurrentProcess();
		TerminateProcess(hProcess, 0);  // Exit code 0
		CloseHandle(hProcess);
	}
	if (PurgeSaveQuit)
	{
		HANDLE hProcess = GetCurrentProcess();
		TerminateProcess(hProcess, 0);  // Exit code 0
		CloseHandle(hProcess);
	}
	cout << "Render is finished. Executing post-render actions!" << endl;
	return err;
}

A_Err HandleRender()
{
	A_Err err = A_Err_NONE;
	try
	{
		if (renderStarted)
		{
			AEGP_SuiteHandler suitesForRender(sP);
			A_long numRQItems;
			suitesForRender.RQItemSuite3()->AEGP_GetNumRQItems(&numRQItems);
			for (int i = 0; i < numRQItems; i++)
			{
				AEGP_RQItemRefH rq_itemH;
				suitesForRender.RQItemSuite3()->AEGP_GetRQItemByIndex(i, &rq_itemH);
				AEGP_CompH rq_compH;
				suitesForRender.RQItemSuite3()->AEGP_GetCompFromRQItem(rq_itemH, &rq_compH);

				AEGP_RenderItemStatusType rq_state;
				suitesForRender.RQItemSuite3()->AEGP_GetRenderState(rq_itemH, &rq_state);

				if (rq_state == AEGP_RenderItemStatus_DONE)
				{
					if (renderStarted == false)
						return err;			
					PostRenderActions(true);
					cout << "executed!";
					renderStarted = false;
				}
				else if (rq_state == AEGP_RenderItemStatus_ERR_STOPPED)
				{
					PostRenderActions(false);
					renderStarted = false;
				}
			}
		}
		else
			return err;
	}
	catch (A_Err& thrown_err)
	{
		cout << "ERROR" << endl;
		//err = thrown_err;
	}
	return err;
}
static A_Err IdleHook(AEGP_GlobalRefcon	plugin_refconP, AEGP_IdleRefcon refconP, A_long *max_sleepPL)
{
	const A_Err err = A_Err_NONE;
	if (S_idle_count == 50)
	{
		/*AllocConsole();
		FILE* fDummy;
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		cout << "Console initialized!\n";*/
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)mainThread, nullptr, 0, nullptr);
	}
	HandleRender();
	HandleInternalRequests();
	S_idle_count++;
	return err;
}

static A_Err CommandHook(AEGP_GlobalRefcon	plugin_refconPV, AEGP_CommandRefcon	refconPV, AEGP_Command command, AEGP_HookPriority hook_priority, A_Boolean already_handledB, A_Boolean *handledPB)
{
	A_Err err = A_Err_NONE; 		
	AEGP_SuiteHandler suites(sP);
	try 
	{
		if (S_Easy_Cheese_cmd == command)
		{
			*handledPB = TRUE; //TRUE invalides the command
			err = A_Err_NONE;
		}
		else if (command == 2302) //Click on 'Render' button
		{
			*handledPB = FALSE;
			renderStarted = true;
			err = A_Err_NONE;
		}
	} 
	catch (A_Err &thrown_err) 
	{
		err = thrown_err;
	}
	return err;
}

A_Err EntryPointFunc(struct SPBasicSuite *pica_basicP, A_long major_versionL, A_long minor_versionL, AEGP_PluginID aegp_plugin_id, AEGP_GlobalRefcon *global_refconP)
{
	S_my_id	= aegp_plugin_id;
	A_Err err = A_Err_NONE, err2 = A_Err_NONE;
	sP = pica_basicP;
	AEGP_SuiteHandler suites(pica_basicP);
	ERR(suites.RegisterSuite5()->AEGP_RegisterCommandHook(S_my_id, AEGP_HP_BeforeAE, AEGP_Command_ALL, CommandHook, 0));
	ERR(suites.RegisterSuite5()->AEGP_RegisterIdleHook(S_my_id, IdleHook, 0));
	getEffectList();
	executables.push_back(&ApplyEffectByName_e);
	executables.push_back(&ExecuteScriptExternally_e);
	executables.push_back(&ApplyPresetByPath_e);
	if (err)
	{
		ERR2(suites.UtilitySuite3()->AEGP_ReportInfo(S_my_id, "Easy_Cheese : Could not register command hook."));
	}
	return err;
}
				   