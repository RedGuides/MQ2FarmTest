#pragma once
#include "..\MQ2Plugin.h"

using fIsNavMeshLoaded = bool(*)();
using fIsNavPathActive = bool(*)();
using fIsNavPossible = bool(*)(const char* szLine);
using fGetNavPathLength = float(*)(const char* szLine);
using fExecuteNavCommand = bool(*)(const char* szLine);
using fIsNavPathPaused = bool(*)();

unsigned long long NavTimer = 0;

static PMQPLUGIN FindMQ2NavPlugin()
{
	PMQPLUGIN pPlugin = pPlugins;
	while (pPlugin)
	{
		if (!_stricmp("MQ2Nav", pPlugin->szFilename))
		{
			return pPlugin;
		}

		pPlugin = pPlugin->pNext;
	}

	return nullptr;
}

float PathLength(unsigned long SpawnID)
{
	PMQPLUGIN pNav;
	fGetNavPathLength NavPathLength = 0;
	if (pNav = FindMQ2NavPlugin())
	{
		NavPathLength = (fGetNavPathLength)GetProcAddress(pNav->hModule, "GetNavPathLength");
		if (NavPathLength)
		{
			char szNavLine[32] = { 0 };
			sprintf_s(szNavLine, 32, "id %u", SpawnID);
			bool bTemp = gFilterMQ;
			gFilterMQ = true;
			float bTemp2 = NavPathLength(szNavLine);
			gFilterMQ = bTemp;
			return bTemp2;
		}
	}
	return 9999.0f;
}

bool PathExists(unsigned long SpawnID)
{
	PMQPLUGIN pNav;
	fIsNavPossible NavPathExists = 0;
	if (pNav = FindMQ2NavPlugin())
	{
		NavPathExists = (fIsNavPossible)GetProcAddress(pNav->hModule, "IsNavPossible");
		if (NavPathExists)
		{
			char szNavLine[32] = { 0 };
			sprintf_s(szNavLine, 32, "id %u", SpawnID);
			bool bTemp = gFilterMQ;
			gFilterMQ = true;
			bool bTemp2 = NavPathExists(szNavLine);
			gFilterMQ = bTemp;
			if (bTemp2) return bTemp2;
		}
	}
	return false;
}

bool MeshLoaded()
{
	PMQPLUGIN pNav;
	fIsNavMeshLoaded NavMeshLoaded = 0;
	if (pNav = FindMQ2NavPlugin())
	{
		//Custom MeshCheck
		NavMeshLoaded = (fIsNavMeshLoaded)GetProcAddress(pNav->hModule, "IsNavMeshLoaded");
		if (NavMeshLoaded)
		{
			return NavMeshLoaded();
		}
	}
	return false;
}

bool NavActive()
{
	PMQPLUGIN pNav;
	fIsNavPathActive NavPathActive = 0;
	if (pNav = FindMQ2NavPlugin())
	{
		NavPathActive = (fIsNavMeshLoaded)GetProcAddress(pNav->hModule, "IsNavPathActive");
		if (NavPathActive)
		{
			return NavPathActive();
		}
	}
	return false;
}

bool NavPaused()
{
	PMQPLUGIN pNav;
	fIsNavPathPaused NavPathPaused = 0;
	if (pNav = FindMQ2NavPlugin())
	{
		NavPathPaused = (fIsNavPathPaused)GetProcAddress(pNav->hModule, "IsNavPathPaused");
		if (NavPathPaused)
		{
			return NavPathPaused();
		}
	}
	return false;
}

void NavCommand(char* szLine)
{
	PMQPLUGIN pNav;
	fExecuteNavCommand TheNavCommand = 0;
	if (pNav = FindMQ2NavPlugin())
	{
		TheNavCommand = (fExecuteNavCommand)GetProcAddress(pNav->hModule, "ExecuteNavCommand");
		if (TheNavCommand)
		{
			char szNavLine[MAX_STRING] = { 0 };
			sprintf_s(szNavLine, MAX_STRING, "%s", szLine);
			bool bTemp = gFilterMQ;
			gFilterMQ = true;
			TheNavCommand(szNavLine);
			gFilterMQ = bTemp;
			NavTimer = GetTickCount64();
		}
	}
	else {
		WriteChatf("Couldn't find MQ2Nav Plugin in the plugin list");
	}
	return;
}

void NavEnd()
{
	PMQPLUGIN pNav;
	fExecuteNavCommand TheNavCommand = 0;
	if (pNav = FindMQ2NavPlugin())
	{
		TheNavCommand = (fExecuteNavCommand)GetProcAddress(pNav->hModule, "ExecuteNavCommand");
		if (TheNavCommand)
		{
			bool bTemp = gFilterMQ;
			gFilterMQ = true;
			TheNavCommand("stop");
			gFilterMQ = bTemp;
		}
	}
}
