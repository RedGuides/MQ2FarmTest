
typedef bool(__cdecl *fNavPathExists)(PCHAR szLine);
fNavPathExists NavPathExists = 0;

typedef float(__cdecl *fNavPathLength)(PCHAR szLine);
fNavPathLength NavPathLength = 0;

typedef VOID(__cdecl *fNavCommand)(PSPAWNINFO pChar, PCHAR szLine);
fNavCommand TheNavCommand = 0;

typedef bool(__cdecl *fNavMeshLoaded)();
fNavMeshLoaded NavMeshLoaded = 0;

typedef bool(__cdecl *fNavPathActive)();
fNavPathActive NavPathActive = 0;

typedef bool(__cdecl *fNavPathPaused)();
fNavPathPaused NavPathPaused = 0;

PMQPLUGIN pNav = nullptr;

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

float PathLength(DWORD SpawnID)
{
	PMQPLUGIN pNav;
	fNavPathLength NavPathLength = 0;
	if (pNav = FindMQ2NavPlugin())
	{
		NavPathLength = (fNavPathLength)GetProcAddress(pNav->hModule, "NavPathLength");
		if (NavPathLength)
		{
			CHAR szNavLine[32];
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
bool PathExists(DWORD SpawnID)
{
	PMQPLUGIN pNav;
	fNavPathExists NavPathExists = 0;
	if (pNav = FindMQ2NavPlugin())
	{
		NavPathExists = (fNavPathExists)GetProcAddress(pNav->hModule, "NavPossible");
		if (NavPathExists)
		{
			CHAR szNavLine[32];
			sprintf_s(szNavLine, 32, "id %u", SpawnID);
			bool bTemp = gFilterMQ;
			gFilterMQ = true;
			bool bTemp2 = NavPathExists(szNavLine);
			WriteChatf("Temp2: %d", bTemp2);
			gFilterMQ = bTemp;
			return bTemp2;
		}
	}
	return false;
}

bool MeshLoaded()
{
	PMQPLUGIN pNav;
	fNavMeshLoaded NavMeshLoaded = 0;
	if (pNav = FindMQ2NavPlugin())
	{
		//Custom MeshCheck
		NavMeshLoaded = (fNavMeshLoaded)GetProcAddress(pNav->hModule, "NavMeshLoaded"); //mmo MeshCheck
		if (!NavMeshLoaded)
			NavMeshLoaded = (fNavMeshLoaded)GetProcAddress(pNav->hModule, "IsMeshLoaded"); //rg MeshCheck
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
	fNavPathActive NavPathActive = 0;
	if (pNav = FindMQ2NavPlugin())
	{
		NavPathActive = (fNavMeshLoaded)GetProcAddress(pNav->hModule, "NavPathActive");
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
	fNavPathPaused NavPathPaused = 0;
	if (pNav = FindMQ2NavPlugin())
	{
		NavPathPaused = (fNavPathPaused)GetProcAddress(pNav->hModule, "NavPathPaused");
		if (NavPathPaused)
		{
			return NavPathPaused();
		}
	}
	return false;
}

VOID NavCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	PMQPLUGIN pNav;
	fNavCommand TheNavCommand = 0;
	if (pNav = FindMQ2NavPlugin())
	{
		TheNavCommand = (fNavCommand)GetProcAddress(pNav->hModule, "NavCommand");
		if (TheNavCommand)
		{
			CHAR szNavLine[32];
			sprintf_s(szNavLine, 32, "id %s", szLine);
			bool bTemp = gFilterMQ;
			gFilterMQ = true;
			TheNavCommand(pChar, szNavLine);
			gFilterMQ = bTemp;
		}

	}
	return;
}

void NavEnd(PSPAWNINFO pChar)
{
	PMQPLUGIN pNav;
	fNavCommand TheNavCommand = 0;
	if (pNav = FindMQ2NavPlugin())
	{
		TheNavCommand = (fNavCommand)GetProcAddress(pNav->hModule, "NavCommand");
		if (TheNavCommand)
		{
			bool bTemp = gFilterMQ;
			gFilterMQ = true;
			TheNavCommand(pChar, "stop");
			gFilterMQ = bTemp;
		}
	}
}
