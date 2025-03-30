#include <A.h>
#include <AEGP_SuiteHandler.h>
#pragma once
static AEGP_Command S_Easy_Cheese_cmd = 0L;
static AEGP_PluginID S_my_id = 0L;
static A_long S_idle_count = 0L;
static SPBasicSuite* sP = NULL;

static A_Err ExecuteScript(const char* scriptext)
{
	A_Err err = A_Err_NONE;
	AEGP_SuiteHandler suites(sP);
	AEGP_MemHandle outResultPH;
	AEGP_MemHandle outErrorStringPH;
	err = suites.UtilitySuite6()->AEGP_ExecuteScript(S_my_id, scriptext, true, &outResultPH, &outErrorStringPH);

	err = suites.MemorySuite1()->AEGP_FreeMemHandle(outResultPH);
	err = suites.MemorySuite1()->AEGP_FreeMemHandle(outErrorStringPH);
	return err;
}
