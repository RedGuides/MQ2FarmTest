/* MQ2FarmTest.cpp
 Chatwiththisname/PeteSampras - Started MQ2Farm Initial Setup/Preperation
 Chatwiththisname/Renji - 10/07/2018 - Worked SpawnSearch and Nav Code
 Chatwiththisname 10/8/2018 - Working on Algorithms for Farming.
							- Adding FarmCommand Functionality.
							- Added Health/Endurance/Mana Checks, still requires logic for resting routines.
							- Added HaveAggro() function. Returns true if any Autohaters are populated on Xtargetlist.
 Chatwiththisname 10/16/2018 - Test has been down a while, but added a getFirstAggroed() function to try and
							 - deal with aggro before trying to find a mob with a shortest path to fix
							 - ignoring aggrod mobs.
 Chatwiththisname 11/12/2018 - Working on CastDetrimental() function to cast single target detrimental spells.
							 - Added rest routines.
 Chatwiththisname 1/2/2019 - Cleaning up the code to remove all the comments.
						   - trying to add Discs to combat routines, not having much luck.
						   - Separated FarmTest into seperate files.
						   - Tried adding TLO and member support. No luck, but kept the file.
 Chatwiththisname 1/3/2019 - Now generating a list of Discs for toons to use and separating them to sections.
						   - Firing Discs during combat based on the previously mentioned list.
						   - DiscReady function now checks endurance/mana/reagent requirements before reporting true.
						   - Actually enjoying testing this on my berserker. He's doing pretty good DPS without MQ2Melee.
 Chatwiththisname 1/5/2019 - Got the Farm datatype fixed and added two members. TargetID and Version. ${Farm.TargetID} ${Farm.Version}
						   - Cleaned up FarmCommand and it now report invalid command.
						   - Made additions to ignorethese and ignorethis.
						   - "/farm " Help list of commands updated to show all commands for current build.
						   - Now sitting when no target is found to kill while it waits for more things to spawn or wander in range.
						   - DiscReady function now makes sure you aren't trying to use an active disc when you already have one running.
						   - Added usage of Instant cast beneficial buffs while in combat.
						   - Made a HUD for use with the ${Farm.TargetID} to get a display of what it's going after and some information, useful for debugging. :-)
						   - You can now add and remove Discs with INI entries. [DiscRemove] section for removing. [DiscAdd] section for Additions.
 Chatwiththisname 1/6/2019 - INI's are now character specific, stored in release folder as MQ2FarmTest_CharacterName.ini
						   - Discs are now grabbed and sorted when the /farm on command is given instead of when the plugin loads. 
 Chatwiththisname 1/7/2019 - Added more refined sorting for summmoning discs and Endurance Regenaration Discs.
						   - Adding abilities base on the INI is no longer Rank Specific.
						   - Removing Combat Abilities base on the INI is no longer rank specific.
						   - INI Name now updated when the farm command is used for anything to ensure that the proper file is being written to.
						   - Added in the casting of Aura Discs and Endurance Regen Discs for use out of combat.
						   - Refined ActiveDisc detections to eliminate trying to use a disc when there is already an active disc, but allow the use of combat abilities unhindered.
 Chatwiththisname 1/20/2019 - Added the use of the FarmMobIgnored.ini file in the macro folder for the global ignores
							  so players didn't need to reinvent the wheel and could use the existing ignores file.
							- Added group health mana and endurance checks so you can now have the merc tag along
							  without worry of running them out of mana.
							- Reduced the delay between pulses by 1/3rd
							- Nav will now refresh destination if target is moving while navigating to the selected spawn.
							- Made some adjustments to stopping distance to prevent "You are to far away" issues.
							- Discs now verify target is in range before attempting to use if you have a target.
							- Fixed a crash that occured if no valid spawns were found.
 Chatwiththisname 1/22/2019 - CHANGE: Fixed a typo in setting the ignore file, changed from FarmMobIgnores.ini to FarmMobIgnored.ini
 Chatwiththisname 1/28/2019 - CHANGE: If the pSpawn has no ManaMax, lie and say they're at 100% mana, even if they don't have a mana bar (rogue merc fix)
							- CHANGE: Added a delay after a disc is used before sitting again to avoid interupting discs during downtimes
							- CHANGE: Tell user about the "/farm help" command on initilize.
							- CHANGE: "/farm off" doesn't turn off attack if it's on? Bug or feature? Check for aggro, if none, turn off
							  attack, otherwise leave it on?
							- CHANGE: Add a message to let the user know there are no valid targets nearby and you are sitting while waiting for more.
 Chatwiththisname 1/29/2019 - CHANGE: Changed the distance checks to Distance3DToSpawn function instead of GetDistance
							- CHANGE: No longer checking Health/Mana/Endurance for you or group while you have Aggro.
							- CHANGE: Adjust a "radius" circle on MQ2Map to depict the Radius engage
							- CHANGE: Added /farm command param "ShowSettings" to display all valid settings.
							- CHANGE: Gave radius a purpose. Set it so that where you issue the /farm on command is the anchor point for all potential
							  target consideration, and the user should not select entities outside of that circle when farming.
							- CHANGE: Added the /permignore command to replicate the usage in the macro
							- CHANGE: Added DeathChecks and Missing Group members checks. These are the same thing as far as the plugin is concerned.
							- CHANGE: DeathCheck includes mercenary's correctly. Not yet handling reviving them.

							- TODO: Fix spell casting...yes, really...maybe....I'm not sure.
							- TODO: Handle Dead mercs appropriately. (Gah, window access?)

[DiscRemove]
DiscRemove1=Hiatus
DiscRemove2=Mangling Discipline
DiscRemove3=Proactive Retaliation
DiscRemove4=Axe of Rekatok
[DiscAdd]
DiscAdd1=Breather
DiscAdd2=Disconcerting Discipline
DiscAdd3=Frenzied Resolve Discipline
DiscAdd4=Axe of the Aeons
DiscAdd5=Cry Carnage
DiscAdd6=Bloodlust Aura
DiscAdd7=Blood Brand
DiscAdd8=Axe of the Demolisher
*/


#define PLUGIN_NAME "MQ2FarmTest"
#define VERSION "0.3"
#define PLUGINMSG "\ar[\a-tMQ2Farm\ar]\ao:: "
#include "../MQ2Plugin.h"
#include "Prototypes.h"
#include "Variables.h"
#include "NavCommands.h"
#include "FarmType.h"
#include <vector>
using namespace std;

bool pulsing = false;


PreSetup(PLUGIN_NAME);
PLUGIN_VERSION(atof(VERSION));

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
	CheckAlias();
	AddMQ2Data("Farm", dataFarm);
	pFarmType = new MQ2FarmType;
	if (InGame()) {
		sprintf_s(ThisINIFileName, MAX_STRING, "%s\\MQ2FarmTest_%s.ini", gszINIPath, GetCharInfo()->Name);
		DoINIThings();
	}
	AddCommand("/farm", FarmCommand);
	AddCommand("/ignorethese", IgnoreTheseCommand);
	AddCommand("/ignorethis", IgnoreThisCommand);
	AddCommand("/permignore", PermIgnoreCommand);
	WriteChatf("%s\aw- \ag%s", PLUGINMSG, VERSION);
	WriteChatf("%s \ao/farm help \aw- \ay For a list of commands!", PLUGINMSG);
	sprintf_s(IgnoresFileName, MAX_STRING, "%s\\Macros\\FarmMobIgnored.ini", gszINIPath);

}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
	PluginOff();
	RemoveMQ2Data("Farm");
	RemoveCommand("/farm");
	RemoveCommand("/ignorethese");
	RemoveCommand("/ignorethis");
}

// Called after entering a new zone
//PLUGIN_API VOID OnZoned(VOID)
//{
//    DebugSpewAlways("MQ2FarmTest::OnZoned()");
//}

// Called once directly after initialization, and then every time the gamestate changes
//PLUGIN_API VOID SetGameState(DWORD GameState)
//{
	//DebugSpewAlways("MQ2FarmTest::SetGameState()");
	//if (GameState==GAMESTATE_INGAME)
	// create custom windows if theyre not set up, etc
//}


// This is called every time MQ pulses
PLUGIN_API VOID OnPulse(VOID)
{

	if (!InGame() || !activated || !MeshLoaded() || pulsing || ++Pulse < PulseDelay) return;
	pulsing = true;
	Pulse = 0;
	if (!HaveAggro()) {
		if (!AmIReady()) {
			if (NavActive())
				NavEnd(GetCharInfo()->pSpawn);
			if (GetCharInfo()->pSpawn->StandState == STANDSTATE_STAND && !Casting() && DiscLastTimeUsed < GetTickCount64())
				EzCommand("/sit");
			pulsing = false;
			return;
		}
		RestRoutines();
	}
	PSPAWNINFO Mob = (PSPAWNINFO)GetSpawnByID(MyTargetID);
	if (!Mob || !Mob->SpawnID || Mob->Type == SPAWN_CORPSE)
		MyTargetID = 0;
	if (getFirstAggroed() && !MyTargetID) {
		MyTargetID = getFirstAggroed();
		Mob = (PSPAWNINFO)GetSpawnByID(MyTargetID);
		NavEnd(GetCharInfo()->pSpawn);
	}
	if (!MyTargetID) {
		GemIndex = 0;
		MyTargetID = SearchSpawns(searchString);
		Mob = (PSPAWNINFO)GetSpawnByID(MyTargetID);
	}
	if (Mob && (AmIReady() || HaveAggro()))
		NavigateToID(MyTargetID);
	pulsing = false;
}


// This is called each time a spawn is removed from a zone (removed from EQ's list of spawns).
// It is NOT called for each existing spawn when a plugin shuts down.
PLUGIN_API VOID OnRemoveSpawn(PSPAWNINFO pSpawn)
{
	//I may use this, time will tell. 
	//DebugSpewAlways("MQ2FarmTest::OnRemoveSpawn(%s)",pSpawn->Name);
}

PLUGIN_API VOID Zoned(VOID)
{
	if (!MeshLoaded()) {
		WriteChatf("%s\a-rNo Mesh loaded for this zone. MQ2Farm does't work without it. Create a mesh and type /nav reload", PLUGINMSG);
	}
	//DebugSpewAlways("MQ2FarmTest::Zoned");
}

void CheckAlias()
{
	char aliases[MAX_STRING] = { 0 };
	if (RemoveAlias("/farm"))
		strcat_s(aliases, " /farm ");
	if (RemoveAlias("/ignorethis"))
		strcat_s(aliases, " /ignorethis ");
	if (RemoveAlias("/ignorethese"))
		strcat_s(aliases, " /ignorethese ");
	if (RemoveAlias("/loadignore"))
		strcat_s(aliases, " /loadignore ");
	if (strlen(aliases) > 0)
		WriteChatf("%s\ayWARNING\ao::\awAliases for \ao%s\aw were detected and removed.", PLUGINMSG, aliases);
}

void VerifyINI(char Section[MAX_STRING], char Key[MAX_STRING], char Default[MAX_STRING], char ININame[MAX_STRING])
{
	char temp[MAX_STRING] = { 0 };
	if (GetPrivateProfileString(Section, Key, 0, temp, MAX_STRING, ININame) == 0)
	{
		WritePrivateProfileString(Section, Key, Default, ININame);
	}
}

//convert char array to bool
bool atob(char x[MAX_STRING])
{
	for (int i = 0; i < 4; i++)
		x[i] = tolower(x[i]);
	if (!strcmp(x, "true") || atoi(x) != 0)
		return true;
	return false;
}

inline bool InGame()
{
	return(GetGameState() == GAMESTATE_INGAME && GetCharInfo() && GetCharInfo()->pSpawn && GetCharInfo2());
}

void DoINIThings()
{
	char temp[MAX_STRING] = { 0 };
	DebugSpewAlways("Initializing MQ2FarmTest");
	//Check for INI entry for Key Debugging in General section.
	VerifyINI("General", "Debugging", "false", ThisINIFileName);
	GetPrivateProfileString("General", "Debugging", "false", temp, MAX_STRING, ThisINIFileName);
	Debugging = atob(temp);
	if (Debugging) WriteChatf("\ar[\a-tGeneral\ar]");
	if (Debugging) WriteChatf("\ayDebugging: \at%d", Debugging);

	//Check for INI entry for Key useLogOut in General section.
	VerifyINI("General", "useLogOut", "false", ThisINIFileName);
	GetPrivateProfileString("General", "useLogOut", "false", temp, MAX_STRING, ThisINIFileName);
	useLogOut = atob(temp);
	if (Debugging) WriteChatf("\ayuseLogOut: \at%d", useLogOut);

	//Check for INI entry for Key useEQBC in General section.
	VerifyINI("General", "useEQBC", "false", ThisINIFileName);
	GetPrivateProfileString("General", "useEQBC", "false", temp, MAX_STRING, ThisINIFileName);
	useEQBC = atob(temp);
	if (Debugging) WriteChatf("\ayuseEQBC: \at%d", useEQBC);

	//Check for INI entry for Key useMerc in General section.
	VerifyINI("General", "useMerc", "false", ThisINIFileName);
	GetPrivateProfileString("General", "useMerc", "false", temp, MAX_STRING, ThisINIFileName);
	useMerc = atob(temp);
	if (Debugging) WriteChatf("\ayuseMerc: \at%d", useMerc);

	//Check for INI entry for Key CastDetrimental in General section.
	VerifyINI("General", "CastDetrimental", "false", ThisINIFileName);
	GetPrivateProfileString("General", "CastDetrimental", "false", temp, MAX_STRING, ThisINIFileName);
	CastDetrimental = atob(temp);
	if (Debugging) WriteChatf("\ayCastDetrimental: \at%d", CastDetrimental);

	if (Debugging) WriteChatf("\ar[\a-tPull\ar]");
	//Check for INI entry for Key ZRadius in Pull section.
	VerifyINI("Pull", "ZRadius", "25", ThisINIFileName);
	GetPrivateProfileString("Pull", "ZRadius", "25", temp, MAX_STRING, ThisINIFileName);
	ZRadius = atoi(temp);
	if (Debugging) WriteChatf("\ayZRadius: \at%d", ZRadius);

	//Check for INI entry for Key Radius in Pull section.
	VerifyINI("Pull", "Radius", "100", ThisINIFileName);
	GetPrivateProfileString("Pull", "Radius", "100", temp, MAX_STRING, ThisINIFileName);
	if (atoi(temp) == 0) {
		Radius = 1;
	}
	else {
		Radius = atoi(temp);
	}
	sprintf_s(temp, MAX_STRING, "/squelch /mapfilter CastRadius color 0 0 255");
	EzCommand(temp);
	sprintf_s(temp, MAX_STRING, "/squelch /mapfilter CastRadius %i", Radius);
	EzCommand(temp);
	if (Debugging) WriteChatf("\ayRadius: \at%d", Radius);

	//Check for INI entry for Key PullAbility in Pull section.
	VerifyINI("Pull", "PullAbility", "melee", ThisINIFileName);
	GetPrivateProfileString("Pull", "PullAbility", "melee", PullAbility, MAX_STRING, ThisINIFileName);
	if (Debugging) WriteChatf("\ayPullAbility: \at%s", PullAbility);

	if (Debugging) WriteChatf("\ar[\a-tHealth\ar]");
	//Check for INI entry for Key HealAt in Health section.
	VerifyINI("Health", "HealAt", "70", ThisINIFileName);
	GetPrivateProfileString("Health", "HealAt", "70", temp, MAX_STRING, ThisINIFileName);
	HealAt = atoi(temp);
	if (Debugging) WriteChatf("\ayHealAt: \at%u", HealAt);

	//Check for INI entry for Key HealTill in Health section.
	VerifyINI("Health", "HealTill", "100", ThisINIFileName);
	GetPrivateProfileString("Health", "HealTill", "100", temp, MAX_STRING, ThisINIFileName);
	HealTill = atoi(temp);
	if (Debugging) WriteChatf("\ayHealTill: \at%u", HealTill);

	if (Debugging) WriteChatf("\ar[\a-tEndurance\ar]");
	//Check for INI entry for Key MedEndAt in Endurance section.
	VerifyINI("Endurance", "MedEndAt", "10", ThisINIFileName);
	GetPrivateProfileString("Endurance", "MedEndAt", "10", temp, MAX_STRING, ThisINIFileName);
	MedEndAt = atoi(temp);
	if (Debugging) WriteChatf("\ayMedEndAt: \at%u", MedEndAt);

	//Check for INI entry for Key MedEndTill in Endurance section.
	VerifyINI("Endurance", "MedEndTill", "100", ThisINIFileName);
	GetPrivateProfileString("Endurance", "MedEndTill", "100", temp, MAX_STRING, ThisINIFileName);
	MedEndTill = atoi(temp);
	if (Debugging) WriteChatf("\ayMedEndAt: \at%u", MedEndTill);

	if (Debugging) WriteChatf("\ar[\a-tMana\ar]");
	//Check for INI entry for Key MedAt in Mana section.
	VerifyINI("Mana", "MedAt", "30", ThisINIFileName);
	GetPrivateProfileString("Mana", "MedAt", "30", temp, MAX_STRING, ThisINIFileName);
	MedAt = atoi(temp);
	if (Debugging) WriteChatf("\ayMedAt: \at%u", MedAt);

	//Check for INI entry for Key MedTill in Mana section.
	VerifyINI("Mana", "MedTill", "100", ThisINIFileName);
	GetPrivateProfileString("Mana", "MedTill", "100", temp, MAX_STRING, ThisINIFileName);
	MedTill = atoi(temp);
	if (Debugging) WriteChatf("\ayMedTill: \at%u", MedTill);
	UpdateSearchString();

}

//Start Farm Commands
void IgnoreTheseCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!strlen(szLine)) {
		if (pTarget) {
			CHAR IgnoreString[MAX_STRING];
			sprintf_s(IgnoreString, MAX_STRING, "add 1 %s", ((PSPAWNINFO)pTarget)->DisplayedName);
			Alert(GetCharInfo()->pSpawn, IgnoreString);
			MyTargetID = 0;
			ClearTarget();
			return;
		}
		else {
			WriteChatf("%s \atYou must have a target or provide a NPC name to ignore. Please try again.", PLUGINMSG);
		}
	}
	else {
		Alert(GetCharInfo()->pSpawn, szLine);
		return;
	}
}

void IgnoreThisCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!strlen(szLine)) {
		if (pTarget) {
			CHAR IgnoreString[MAX_STRING];
			sprintf_s(IgnoreString, MAX_STRING, "add 1 %s", ((PSPAWNINFO)pTarget)->Name);
			Alert(GetCharInfo()->pSpawn, IgnoreString);
			MyTargetID = 0;
			ClearTarget();
		}
		else {
			WriteChatf("%s \atYou must have a target to ignore to ignore a specific mob", PLUGINMSG);
		}
	}
	else {
		WriteChatf("%s \atYou cannot use a name for this ignore command. It will only use your target. \arNothing added to ignore!", PLUGINMSG);
	}
}

void FarmCommand(PSPAWNINFO pChar, PCHAR Line) {
	if (!InGame()) {
		WriteChatf("%s\arYou can't use this command unless you are in game!", PLUGINMSG);
		return;
	}
	if (!strlen(Line)) {
		ListCommands();
	}

	char buffer[MAX_STRING] = "";
	sprintf_s(buffer, MAX_STRING, GetCharInfo()->Name);
	if (!strlen(ThisINIFileName) || !strstr(ThisINIFileName, buffer)) {
		WriteChatf("%s\ayUpdating the characters INI to be this character", PLUGINMSG);
		sprintf_s(ThisINIFileName, MAX_STRING, "%s\\MQ2FarmTest_%s.ini", gszINIPath, GetCharInfo()->Name);
	}
	if (strlen(Line)) {
		CHAR Arg1[MAX_STRING] = { 0 }, Arg2[MAX_STRING] = { 0 };
		GetArg(Arg1, Line, 1);
		if (Debugging) WriteChatf("\ar[\a-tMQ2Farm\ar]\ao::\atArg1 is: \ap%s", Arg1);

		if (!_stricmp(Arg1, "help")) {
			ListCommands();
			return;
		}

		if (!_stricmp(Arg1, "on")) {
			PluginOn();
			return;
		}

		if (!_stricmp(Arg1, "off"))
		{
			PluginOff();
			return;
		}
		
		if (!_stricmp(Arg1, "radius"))
		{
			GetArg(Arg2, Line, 2);
			if (IsNumber(Arg2))
			{
				Radius = atoi(Arg2);
				WritePrivateProfileString("Pull", "Radius", Arg2, ThisINIFileName);
				UpdateSearchString();
				CHAR temp[MAX_STRING] = "";
				sprintf_s(temp, MAX_STRING, "/squelch /mapfilter CastRadius %i", Radius);
				EzCommand(temp);
				WriteChatf("%s\atRadius is now: \ap%i", PLUGINMSG, Radius);
				return;
			}
			else {
				WriteChatf("%s\atRadius: \ap%i", PLUGINMSG, Radius);
				return;
			}
		}

		if (!_stricmp(Arg1, "zradius"))
		{
			GetArg(Arg2, Line, 2);
			if (IsNumber(Arg2))
			{
				ZRadius = atoi(Arg2);
				WritePrivateProfileString("Pull", "ZRadius", Arg2, ThisINIFileName);
				UpdateSearchString();
				WriteChatf("%s\atZRadius is now: \ap%i", PLUGINMSG, ZRadius);
				return;
			}
			else {
				WriteChatf("%s\atZRadius: \ap%i", PLUGINMSG, ZRadius);
				return;
			}
		}

		if (!_stricmp(Arg1, "farmmob"))
		{
			GetArg(Arg2, Line, 2);
			if (!strlen(Arg2))
			{
				WriteChatf("%sCurrent farmmob is %s", PLUGINMSG, FarmMob);
				return;
			}
			if (!IsNumber(Arg2))
			{
				if (!_stricmp(Arg2, "clear"))
				{
					sprintf_s(FarmMob, MAX_STRING, "");
					return;
				}
				else {
					sprintf_s(FarmMob, MAX_STRING, Arg2);
				}
				UpdateSearchString();
				WriteChatf("%s\atFarmMob is now: \ap%s", PLUGINMSG, FarmMob);
				return;
			}
		}
		if (!_stricmp(Arg1, "castdetrimental"))
		{
			GetArg(Arg2, Line, 2);
			if (!strlen(Arg2)) {
				if (CastDetrimental) {
					WriteChatf("%sCast Detrimental is On!", PLUGINMSG);
					return;
				}
				else {
					WriteChatf("%sCast Detrimental is Off!", PLUGINMSG);
					return;
				}
			}
			if (IsNumber(Arg2))
			{
				if (!_stricmp(Arg2, "1"))
				{
					if (!CastDetrimental) {
						WriteChatf("%sTurning Cast Detrimental: On!", PLUGINMSG);
						CastDetrimental = true;
						WritePrivateProfileString("General", "CastDetrimental", "true", ThisINIFileName);
						return;
					}
				}
				if (!_stricmp(Arg2, "0"))
				{
					if (CastDetrimental) {
						CastDetrimental = false;
						WritePrivateProfileString("General", "CastDetrimental", "false", ThisINIFileName);
						return;
					}
				}
			}
			else {
				if (!_stricmp(Arg2, "on"))
				{
					if (!CastDetrimental) {
						CastDetrimental = true;
						WritePrivateProfileString("General", "CastDetrimental", "true", ThisINIFileName);
						return;
					}
				}
				if (!_stricmp(Arg2, "off"))
				{
					if (CastDetrimental) {
						CastDetrimental = false;
						WritePrivateProfileString("General", "CastDetrimental", "false", ThisINIFileName);
						return;
					}
				}
			}
		}

		if (!_stricmp(Arg1, "debug") || !_stricmp(Arg1, "debugging"))
		{
			GetArg(Arg2, Line, 2);
			if (!strlen(Arg2)) {
				WriteChatf("%sDebugging: %d", PLUGINMSG, Debugging);
				return;
			}
			if (!IsNumber(Arg2))
			{
				if (!_stricmp(Arg2, "on"))
				{
					if (!Debugging) {
						Debugging = true;
						WritePrivateProfileString("General", "Debugging", "true", ThisINIFileName);
						WriteChatf("%sDebugging: \agOn", PLUGINMSG);
						return;
					}
				}
				if (!_stricmp(Arg2, "off"))
				{
					if (Debugging) {
						Debugging = false;
						WritePrivateProfileString("General", "Debugging", "false", ThisINIFileName);
						WriteChatf("%sDebugging: \arOff", PLUGINMSG);
						return;
					}
				}
			}
			else {
				if (atoi(Arg2) == 1) {
					if (!Debugging) {
						Debugging = true;
						WritePrivateProfileString("General", "Debugging", "true", ThisINIFileName);
						WriteChatf("%sDebugging: \agOn", PLUGINMSG);
						return;
					}
				}
				if (atoi(Arg2) == 0) {
					if (Debugging) {
						Debugging = false;
						WritePrivateProfileString("General", "Debugging", "false", ThisINIFileName);
						WriteChatf("%sDebugging: \arOff", PLUGINMSG);
						return;
					}
				}
			}
		}

		//Mana updates
		if (!_stricmp(Arg1, "MedAt"))
		{
			GetArg(Arg2, Line, 2);
			if (IsNumber(Arg2))
			{
				MedAt = atoi(Arg2);
				WritePrivateProfileString("Mana", "MedAt", Arg2, ThisINIFileName);
				WriteChatf("%s\atMedAt is now: \ap%i", PLUGINMSG, MedAt);
				return;
			}
			else {
				WriteChatf("%s\atMedAt: \ap%i", PLUGINMSG, MedAt);
				return;
			}
		}

		if (!_stricmp(Arg1, "MedTill"))
		{
			GetArg(Arg2, Line, 2);
			if (IsNumber(Arg2))
			{
				MedTill = atoi(Arg2);
				WritePrivateProfileString("Mana", "MedTill", Arg2, ThisINIFileName);
				WriteChatf("%s\atMedTill is now: \ap%i", PLUGINMSG, MedTill);
				return;
			}
			else {
				WriteChatf("%s\atMedTill: \ap%i", PLUGINMSG, MedTill);
				return;
			}
		}

		//Health Updates
		if (!_stricmp(Arg1, "HealAt"))
		{
			GetArg(Arg2, Line, 2);
			if (IsNumber(Arg2))
			{
				HealAt = atoi(Arg2);
				WritePrivateProfileString("Health", "HealAt", Arg2, ThisINIFileName);
				WriteChatf("%s\atHealAt is now: \ap%i", PLUGINMSG, HealAt);
				return;
			}
			else {
				WriteChatf("%s\atHealAt: \ap%i", PLUGINMSG, HealAt);
				return;
			}
		}

		if (!_stricmp(Arg1, "HealTill"))
		{
			GetArg(Arg2, Line, 2);
			if (IsNumber(Arg2))
			{
				HealTill = atoi(Arg2);
				WritePrivateProfileString("Health", "HealTill", Arg2, ThisINIFileName);
				WriteChatf("%s\atHealTill is now: \ap%i", PLUGINMSG, HealTill);
				return;
			}
			else {
				WriteChatf("%s\atHealTill: \ap%i", PLUGINMSG, HealTill);
				return;
			}
		}
		//Endurance updates
		if (!_stricmp(Arg1, "MedEndAt"))
		{
			GetArg(Arg2, Line, 2);
			if (IsNumber(Arg2))
			{
				MedEndAt = atoi(Arg2);
				WritePrivateProfileString("Endurance", "MedEndAt", Arg2, ThisINIFileName);
				WriteChatf("%s\atMedEndAt is now: \ap%i", PLUGINMSG, MedEndAt);
				return;
			}
			else {
				WriteChatf("%s\atMedEndAt: \ap%i", PLUGINMSG, MedEndAt);
				return;
			}
		}

		if (!_stricmp(Arg1, "MedEndTill"))
		{
			GetArg(Arg2, Line, 2);
			if (IsNumber(Arg2))
			{
				MedEndTill = atoi(Arg2);
				WritePrivateProfileString("Endurance", "MedEndTill", Arg2, ThisINIFileName);
				WriteChatf("%s\atMedEndTill is now: \ap%i", PLUGINMSG, MedEndTill);
				return;
			}
			else {
				WriteChatf("%s\atMedEndTill: \ap%i", PLUGINMSG, MedEndTill);
				return;
			}
		}
		if (!_stricmp(Arg1, "ShowSettings"))
		{
			ShowSettings();
			return;
		}
		GetArg(Arg1, Line, 1);
		WriteChatf("%s \arYou provided an invalid command - %s - is not a valid command!", PLUGINMSG, Arg1);
	}
}

//End Farm Commands

//Example: SearchSpawns("npc noalert 1 targetable radius 100 zradius 50");
DWORD SearchSpawns(char szIndex[MAX_STRING])
{
	#define pZone ((PZONEINFO)pZoneInfo)
	double fShortest = 9999.0f;
	PSPAWNINFO sShortest;
	SEARCHSPAWN ssSpawn;
	ClearSearchSpawn(&ssSpawn);
	ssSpawn.FRadius = 999999.0f;
	ParseSearchSpawn(szIndex, &ssSpawn);
	bool bFound = false;
	char temp[MAX_STRING] = "";
	char IgnoredMobList[MAX_STRING] = "";
	char IgnoredMobList1[MAX_STRING] = "";
	VerifyINI(pZone->ShortName, "Ignored", "|", IgnoresFileName);
	GetPrivateProfileString(pZone->ShortName, "Ignored", "|", IgnoredMobList, MAX_STRING, IgnoresFileName);
	VerifyINI(pZone->ShortName, "Ignored1", "|", IgnoresFileName);
	GetPrivateProfileString(pZone->ShortName, "Ignored1", "|", IgnoredMobList1, MAX_STRING, IgnoresFileName);
	for (unsigned long N = 0; N < gSpawnCount; N++)
	{
		if (EQP_DistArray[N].Value.Float > ssSpawn.FRadius && !ssSpawn.bKnownLocation || N > 50)
			break;
		if (SpawnMatchesSearch(&ssSpawn, (PSPAWNINFO)pCharSpawn, (PSPAWNINFO)EQP_DistArray[N].VarPtr.Ptr)) {
			if (PSPAWNINFO pSpawn = (PSPAWNINFO)EQP_DistArray[N].VarPtr.Ptr) {
				if (strlen(pSpawn->Lastname))
					continue;
				sprintf_s(temp, MAX_STRING, "|%s|", pSpawn->DisplayedName);
				if (strstr(IgnoredMobList, temp)) {
					if (Debugging) WriteChatf("\ar[\a-t%u\ar]\ap%s was on the ignore list", pSpawn->SpawnID, temp);
					continue;
				}
				if (strstr(IgnoredMobList1, temp)) {
					if (Debugging) WriteChatf("\ar[\a-t%u\ar]\ap%s was on the ignore list", pSpawn->SpawnID, temp);
					continue;
				}
				if (PathExists(pSpawn->SpawnID) && (int)PathLength(pSpawn->SpawnID) >= (int)Distance3DToSpawn(GetCharInfo()->pSpawn, pSpawn)) {
					if (fShortest > PathLength(pSpawn->SpawnID)) {
						fShortest = PathLength(pSpawn->SpawnID);
						sShortest = pSpawn;
						if (!bFound) bFound = true;
					}
				}
			}
		}
	}
	#undef pZone
	if (bFound) {
			WriteChatf("%s\ar[%u]: %s is my Target", PLUGINMSG, sShortest->SpawnID, sShortest->Name);
			return sShortest->SpawnID;
	}
	if (GetCharInfo()->pSpawn->StandState == STANDSTATE_STAND && !Casting() && DiscLastTimeUsed < GetTickCount64()) {
		WriteChatf("%s\arCharacter is resting while waiting for valid targets or aggro.", PLUGINMSG);
		EzCommand("/sit");
	}
	return false;
}

void ClearTarget() {
	bool bTemp = gFilterMQ;
	gFilterMQ = true;
	if (GetCharInfo()) Target(GetCharInfo()->pSpawn, "clear");
	gFilterMQ = bTemp;
}

void ListCommands()
{
	WriteChatf("%s\ar[\atCommands Available\ar]", PLUGINMSG);
	WriteChatf("%s\ay/farm help \awor \ay/farm\aw--- \atWill output this help menu", PLUGINMSG);
	WriteChatf("%s\ay/farm on|off \aw--- \atWill turn on|off farming with INI settings.", PLUGINMSG);
	WriteChatf("%s\ay/farm radius #### \aw--- \atWill set radius to number provided.", PLUGINMSG);
	WriteChatf("%s\ay/farm zradius #### \aw--- \atWill set zradius to number provided.", PLUGINMSG);
	WriteChatf("%s\ay/farm farmmob \"Mob Name Here\" \aw--- \atWill specify a farmmob to farm.", PLUGINMSG);
	WriteChatf("%s\ay/farm farmmob clear \aw--- \atWill clear the FarmMob and attack anything not on an alertlist.", PLUGINMSG);
	WriteChatf("%s\ay/farm castdetrimental 1|On 0|Off \aw--- \atWill turn on and off casting of single target detrimental spells.", PLUGINMSG);
	WriteChatf("%s\ay/farm debug 1|on 0|Off \aw---\atWill turn on and off debugging messages", PLUGINMSG);
	WriteChatf("%s\ay/farm MedAt \aw---\atWill show you when you will med mana", PLUGINMSG);
	WriteChatf("%s\ay/farm MedAt #### \aw---\atWill set when you when you will med mana", PLUGINMSG);
	WriteChatf("%s\ay/farm MedTill \aw---\atWill show you when you will stop medding mana", PLUGINMSG);
	WriteChatf("%s\ay/farm MedTill ####\aw---\atWill set when you when you will stop medding mana", PLUGINMSG);
	WriteChatf("%s\ay/farm HealAt \aw---\atWill show you when you will med health", PLUGINMSG);
	WriteChatf("%s\ay/farm HealAt #### \aw---\atWill show you when you will med health", PLUGINMSG);
	WriteChatf("%s\ay/farm HealTill \aw---\atWill show you when you will stop medding health", PLUGINMSG);
	WriteChatf("%s\ay/farm HealTill #### \aw---\atWill set when you when you will med health", PLUGINMSG);
	WriteChatf("%s\ay/farm MedEndAt \aw---\atWill show you when you will med endurance", PLUGINMSG);
	WriteChatf("%s\ay/farm MedEndAt #### \aw---\atWill set you when you will med endurance", PLUGINMSG);
	WriteChatf("%s\ay/farm MedEndTill \aw---\atWill show you when you will stop medding endurance", PLUGINMSG);
	WriteChatf("%s\ay/farm MedEndTill #### \aw---\atWill set when you will stop medding endurance", PLUGINMSG);
	WriteChatf("%s\ay/ignorethis \aw--- \atWill temporarily ignore your current target.", PLUGINMSG);
	WriteChatf("%s\ay/ignorethese \aw--- \atWill temporarily ignore all spawns with this targets clean name.", PLUGINMSG);
}

bool AmIReady()
{
	if (!InGame()) return false;
	if (HaveAggro()) return false;
	if (GROUPINFO *myGroup = GetCharInfo()->pGroupInfo) {
		//Death Check
		if (!DeadGroupMember) {
			for (int i = 0; i < 6; i++) {
				if (myGroup->pMember[i]) {
					CHAR Name[MAX_STRING] = { 0 };
					GetCXStr(myGroup->pMember[i]->pName, Name, MAX_STRING);
					CleanupName(Name, sizeof(Name), FALSE, FALSE);
					if (strlen(Name)) {
						if (PSPAWNINFO pSpawn = myGroup->pMember[i]->pSpawn) {
							if (pSpawn->StandState == STANDSTATE_DEAD || pSpawn->RespawnTimer) {
								DeadGroupMember = true;
								WriteChatf("%s\ap%s \a-t--> \arHas DIED!", PLUGINMSG, pSpawn->DisplayedName);
								return false;
							}
						}
						else {
							WriteChatf("%s\ap%s \a-t--> \arHas DIED or left the zone!", PLUGINMSG, Name);
							DeadGroupMember = true;
							return false;
						}
					}
					
				}
			}
		}
		if (DeadGroupMember) {
			bool StillDead = false;
			for (int i = 0; i < 6; i++) {
				if (myGroup->pMember[i]) {
					CHAR Name[MAX_STRING] = { 0 };
					GetCXStr(myGroup->pMember[i]->pName, Name, MAX_STRING);
					CleanupName(Name, sizeof(Name), FALSE, FALSE);//we do this to fix the mercenaryname bug
					if (strlen(Name)) {
						if (PSPAWNINFO pSpawn = myGroup->pMember[i]->pSpawn) {
							if (pSpawn->StandState == STANDSTATE_DEAD || pSpawn->RespawnTimer) {
								StillDead = true;
							}
						}
						else {
							StillDead = true;
						}
					}

				}
			}
			if (StillDead) {
				return false;
			}
			else {
				WriteChatf("%s\agNo longer waiting for dead or missing group members.", PLUGINMSG);
				DeadGroupMember = false;
			}
		}
		//Group Endurance Check
		if (!GettingEndurance) {
			for (int i = 0; i < 6; i++) {
				if (myGroup->pMember[i]) {
					if (PSPAWNINFO pSpawn = myGroup->pMember[i]->pSpawn) {
						if (PercentEndurance(pSpawn) < MedEndAt) {
							GettingEndurance = true;
							WriteChatf("%s\ap%s\ar needs to med Endurance.", PLUGINMSG, pSpawn->DisplayedName);
							return false;
						}
					}
				}
			}
		}
		else if (GettingEndurance) {
			bool stillMedding = false;
			for (int i = 0; i < 6; i++) {
				if (myGroup->pMember[i]) {
					if (PSPAWNINFO pSpawn = myGroup->pMember[i]->pSpawn) {
						if (PercentEndurance(pSpawn) < MedEndTill) {
							stillMedding = true;
						}
					}
				}
			}
			if (stillMedding) {
				return false;
			}
			else {
				WriteChatf("%s\agDone medding Endurance", PLUGINMSG);
				GettingEndurance = false;
			}
		}

		//Group Mana Check
		if (!GettingMana) {
			for (int i = 0; i < 6; i++) {
				if (myGroup->pMember[i]) {
					if (PSPAWNINFO pSpawn = myGroup->pMember[i]->pSpawn) {
						if (PercentMana(pSpawn) < MedAt) {
							GettingMana = true;
							WriteChatf("%s\ap%s\ar needs to med Mana.", PLUGINMSG, pSpawn->DisplayedName);
							return false;
						}
					}
				}
			}
		}
		else if (GettingMana) {
			bool stillMedding = false;
			for (int i = 0; i < 6; i++) {
				if (myGroup->pMember[i]) {
					if (PSPAWNINFO pSpawn = myGroup->pMember[i]->pSpawn) {
						if (PercentMana(pSpawn) < MedTill) {
							stillMedding = true;
						}
					}
				}
			}
			if (stillMedding) {
				return false;
			}
			else {
				WriteChatf("%s\agDone medding Mana", PLUGINMSG);
				GettingMana = false;
			}
		}
		//Group Health Check
		if (!GettingHealth) {
			for (int i = 0; i < 6; i++) {
				if (myGroup->pMember[i]) {
					if (PSPAWNINFO pSpawn = myGroup->pMember[i]->pSpawn) {
						if (PercentHealth(pSpawn) < HealAt) {
							GettingHealth = true;
							WriteChatf("%s\ap%s\ar needs to med Health.", PLUGINMSG, pSpawn->DisplayedName);
							return false;
						}
					}
				}
			}
		}
		else if (GettingHealth) {
			bool stillMedding = false;
			for (int i = 0; i < 6; i++) {
				if (myGroup->pMember[i]) {
					if (PSPAWNINFO pSpawn = myGroup->pMember[i]->pSpawn) {
						if (PercentHealth(pSpawn) < HealTill) {
							stillMedding = true;
						}
					}
				}
			}
			if (stillMedding) {
				return false;
			}
			else {
				WriteChatf("%s\agDone medding Health", PLUGINMSG);
				GettingHealth = false;
			}
		}
	}
	else {
		//Start Medding Endurance Here.
		PSPAWNINFO me = GetCharInfo()->pSpawn;
		if (!GettingEndurance && PercentEndurance(me) < MedEndAt) {
			if (!GettingEndurance) {
				GettingEndurance = true;
				WriteChatf("%s\arNeed to med endurance.", PLUGINMSG);
			}
			return false;
		}

		if (GettingEndurance && PercentEndurance(me) < MedEndTill) {
			return false;
		}
		else {
			if (GettingEndurance) {
				WriteChatf("%s\agDone medding Endurance.", PLUGINMSG);
				GettingEndurance = false;
			}
		}
		//GetCharInfo()->pGroupInfo->pMember[1]

		//Starting Medding Mana here. 
		if (!GettingMana && PercentMana(me) < MedAt) {
			if (!GettingMana) {
				WriteChatf("%s\arNeed to med Mana!", PLUGINMSG);
				GettingMana = true;
			}
			return false;
		}

		if (GettingMana && PercentMana(me) < MedTill) {
			//WriteChatf("%f", PercentMana());
			return false;
		}
		else {
			if (GettingMana) {
				WriteChatf("%s\agDone medding Mana!", PLUGINMSG);
				GettingMana = false;
			}
		}

		//Start medding health here.
		if (!GettingHealth && PercentHealth(me) < HealAt) {
			if (!GettingHealth) {
				WriteChatf("%s\arNeed to med Health!", PLUGINMSG);
				GettingHealth = true;
			}
			return false;
		}
		if (GettingHealth && PercentHealth(me) < HealTill) {
			return false;
		}
		else {
			if (GettingHealth) {
				WriteChatf("%s\agDone medding health!", PLUGINMSG);
				GettingHealth = false;
			}
		}
	}
	return true;
}

bool HaveAggro()
{
	if (ExtendedTargetList *xtm = GetCharInfo()->pXTargetMgr) {
		if (pAggroInfo) {
			for (int i = 0; i < xtm->XTargetSlots.Count; i++) {
				XTARGETSLOT xts = xtm->XTargetSlots[i];
				DWORD spID = xts.SpawnID;
				if (spID && xts.xTargetType == XTARGET_AUTO_HATER) {
					if (PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(spID)) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

void CastDetrimentalSpells()
{
	if (!pLocalPlayer) {
		if (Debugging) WriteChatf("No pLocalPlayer, returning");
		return;
	}
	if (GetCharInfo()->pSpawn->CastingData.IsCasting()) {
		if (Debugging) WriteChatf("already casting, returning");
		return;
	}
	if (!SpellsMemorized()) {
		if (Debugging) WriteChatf("No SpellsMemorized, returning");
		return;
	}
	//TODO: Need to verify line of sight!!!! Haven't done that yet.
	if (GemIndex > 13) {
		GemIndex = 0;
	}

	SPELL *spell = GetSpellByID(GetCharInfo2()->MemorizedSpells[GemIndex]);
	while (!spell) {
		GemIndex++;
		if (GemIndex > 13) GemIndex = 0;
		spell = GetSpellByID(GetCharInfo2()->MemorizedSpells[GemIndex]);
	}

	if (spell && spell->CanCastInCombat) {
		//BYTE    TargetType;         //03=Group v1, 04=PB AE, 05=Single, 06=Self, 08=Targeted AE, 0e=Pet, 28=AE PC v2, 29=Group v2, 2a=Directional
		if (spell->TargetType == 05) {
			//*0x180*/   BYTE    SpellType;          //0=detrimental, 1=Beneficial, 2=Beneficial, Group Only
			if (spell->SpellType == 0) {
				if (GetCharInfo()->pSpawn->SpeedRun > 0.0f) {
					if (Debugging) WriteChatf("Ending Navigation to cast");
					if (NavActive()) NavEnd(GetCharInfo()->pSpawn);
				}
				if (Debugging) WriteChatf("Spell: %s, TargetType: %d, SpellType: %d", spell->Name, spell->TargetType, spell->SpellType);
				char castcommand[MAX_STRING] = "/cast ";
				string s = to_string(GemIndex + 1);
				strcat_s(castcommand, MAX_STRING, s.c_str());
				if (GetCharInfo()->pSpawn->ManaCurrent > (int)spell->ManaCost) {
					if (pTarget && Distance3DToSpawn(GetCharInfo()->pSpawn, (PSPAWNINFO)pTarget) < spell->Range) {
						WriteChatf("%s\arCasting \a-t----> \ap%s \ayfrom Gem %d", PLUGINMSG, spell->Name, GemIndex + 1);
						EzCommand(castcommand);
						CastLastTimeUsed = GetTickCount64();
					}
					else {
						if (Debugging) WriteChatf("Target not in range, or no target.");
					}
				}
				else {
					if (Debugging) WriteChatf("Not enough Mana to cast %s", spell->Name);
				}
			}
		}
	}
	GemIndex++;
}

DWORD getFirstAggroed()
{
	if (ExtendedTargetList *xtm = GetCharInfo()->pXTargetMgr) {
		if (pAggroInfo) {
			for (int i = 0; i < xtm->XTargetSlots.Count; i++) {
				XTARGETSLOT xts = xtm->XTargetSlots[i];
				DWORD spID = xts.SpawnID;
				if (spID && xts.xTargetType == XTARGET_AUTO_HATER) {
					if (PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(spID)) {
						//WriteChatf("I have aggro from: %s", pSpawn->Name);
						return spID;
					}
				}
			}
		}
	}
	return 0;
}

void UpdateSearchString()
{
	sprintf_s(searchString, MAX_STRING, "npc noalert 1 radius %i zradius %i targetable loc %f %f %f %s", Radius, ZRadius, AnchorX, AnchorY, AnchorZ, FarmMob);
}

void PluginOn()
{
	if (activated) return;
	if (!InGame()) {
		WriteChatf("%s\arYou are not in game! Not Activated!!!", PLUGINMSG);
		return;
	}
	if (!MeshLoaded()) {
		WriteChatf("%s\a-rNo Mesh loaded for this zone. MQ2Farm does't work without it. Create a mesh and type /nav reload", PLUGINMSG);
		return;
	}
	activated = true;
	CheckAlias();
	sprintf_s(ThisINIFileName, MAX_STRING, "%s\\MQ2FarmTest_%s.ini", gszINIPath, GetCharInfo()->Name);
	DoINIThings();
	DiscSetup();
	AnchorX = GetCharInfo()->pSpawn->X;
	AnchorY = GetCharInfo()->pSpawn->Y;
	AnchorZ = GetCharInfo()->pSpawn->Z;
	UpdateSearchString();
	WriteChatf("%s\agActivated", PLUGINMSG);
}

void PluginOff()
{
	if (!activated)
		return;
	MyTargetID = 0;
	activated = false;
	NavEnd(GetCharInfo()->pSpawn);
	if (!HaveAggro() && *EQADDR_ATTACK)
		*EQADDR_ATTACK = 00000000;
	WriteChatf("%s\arDeactivated", PLUGINMSG);
}

float AmFacing(DWORD ID)
{
	if (!InGame())
		return false;
	PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(ID);
	float fMyHeading = GetCharInfo()->pSpawn->Heading*0.703125f;
	float fSpawnHeadingTo = (FLOAT)(atan2f(((PSPAWNINFO)pCharSpawn)->Y - pSpawn->Y, pSpawn->X - ((PSPAWNINFO)pCharSpawn)->X) * 180.0f / PI + 90.0f);
	if (fSpawnHeadingTo < 0.0f)
		fSpawnHeadingTo += 360.0f;
	else if (fSpawnHeadingTo >= 360.0f)
		fSpawnHeadingTo -= 360.0f;
	float facing = abs(fSpawnHeadingTo - fMyHeading);
	return facing;
}

bool SpellsMemorized()
{
	for (int i = 0; i < 13; i++) {
		SPELL *Spell = GetSpellByID(GetCharInfo2()->MemorizedSpells[i]);
		if (Spell) return true;
	}
	return false;
}

inline float PercentMana(PSPAWNINFO &pSpawn)
{
	if ((float)pSpawn->ManaMax <= 0) return 100.0f;
	return (float)pSpawn->ManaCurrent / (float)pSpawn->ManaMax * 100.0f;
}

inline float PercentHealth(PSPAWNINFO &pSpawn)
{
	return (float)pSpawn->HPCurrent / (float)pSpawn->HPMax * 100.0f;
}

inline float PercentEndurance(PSPAWNINFO &pSpawn)
{
	return (float)pSpawn->EnduranceCurrent / (float)pSpawn->EnduranceMax * 100.0f;
}

void NavigateToID(DWORD ID) {
	if (CastLastTimeUsed >= GetTickCount64()) return;
	if (PSPAWNINFO Mob = (PSPAWNINFO)GetSpawnByID(ID)) {
		CHAR szNavInfo[32];
		sprintf_s(szNavInfo, 32, "%u", ID);
		if (Mob && (!LineOfSight(GetCharInfo()->pSpawn, Mob) || Distance3DToSpawn(GetCharInfo()->pSpawn, Mob) > 15)) {
			if (!NavActive() || fabs(Mob->SpeedRun) > 0.0f) {
				if (!Casting())
					NavCommand(GetCharInfo()->pSpawn, szNavInfo);
			}
		}
		if (LineOfSight(GetCharInfo()->pSpawn, Mob) && Distance3DToSpawn(GetCharInfo()->pSpawn, Mob) < 75) {
			if (Mob && pTarget && ((PSPAWNINFO)pTarget)->SpawnID != Mob->SpawnID || !pTarget) {
				TargetIt(Mob);
			}
		}
		if (LineOfSight(GetCharInfo()->pSpawn, Mob) && Distance3DToSpawn(GetCharInfo()->pSpawn, Mob) < 12) {
			if (NavActive()) 
				NavEnd(GetCharInfo()->pSpawn);
			if (GetCharInfo()->pSpawn->StandState == STANDSTATE_SIT)
				EzCommand("/stand");
			if (AmFacing(Mob->SpawnID) > 30) 
				Face(GetCharInfo()->pSpawn, "fast");
		}
		if (Mob && pTarget) {
			if (useDiscs) UseDiscs();
			if (CastDetrimental) CastDetrimentalSpells();
			if (*EQADDR_ATTACK) return;
			*EQADDR_ATTACK = 00000001;
			EzCommand("/pet attack");
			EzCommand("/pet swarm");

		}
	}
}

void DiscSetup()
{
	if (!InGame()) return;

	bool replaced = false;
	bool duplicatetimer = false;
	int MaxIndex = 0;

	int CATemp[NUM_COMBAT_ABILITIES];
	for (int i = 0; i < NUM_COMBAT_ABILITIES; i++) {
		CATemp[i] = -1;
	}

	for (int i = 0; i < NUM_COMBAT_ABILITIES; i++) {
		if (pCombatSkillsSelectWnd->ShouldDisplayThisSkill(i)) {
			if (PSPELL pSpell = GetSpellByID(pPCData->GetCombatAbility(i))) {
				//need to make sure the disc is a usable disc. IE: Dichotomic Rage has two abilities, one is level 250.
				if (pSpell->ClassLevel[GetCharInfo()->pSpawn->mActorClient.Class] <= 110) {
					if (MaxIndex == 0) {
						//WriteChatf("Adding first item: \ay%s\aw at [i]Index:\ag %i", pSpell->Name, i);
						CATemp[i] = pSpell->ID;
						MaxIndex++;
						//WriteChatf("Adding 1 to MaxIndex: %i", MaxIndex);
					}
					else {
						for (int j = 0; j <= MaxIndex; j++) {
							//WriteChatf("[j] %i of [MaxIndex] %i", j, MaxIndex);
							if (CATemp[j] != -1) {
								if (PSPELL temp = GetSpellByID(CATemp[j])) {
									if (temp->ReuseTimerIndex == pSpell->ReuseTimerIndex) {
										//WriteChatf("Comparing \ap%s\aw Timer: %i to \ay%s \awTimer: %i", temp->Name, temp->ReuseTimerIndex, pSpell->Name, pSpell->ReuseTimerIndex);
										duplicatetimer = true;
										if (temp->ClassLevel[GetCharInfo()->pSpawn->mActorClient.Class] < pSpell->ClassLevel[GetCharInfo()->pSpawn->mActorClient.Class]) {
											//WriteChatf("\arReplacing \ay%s\aw with \ay%s", temp->Name, pSpell->Name, i);
											CATemp[j] = pSpell->ID;
											replaced = true;
											break;
										}
										else {
											//WriteChatf("Keeping: \ay%s", temp->Name);
										}
									}
								}
							}
						}
						if (!replaced && !duplicatetimer) {
							MaxIndex++;
							//WriteChatf("\agAdding: \ap%s \aw Timer: \ag%i", pSpell->Name, pSpell->ReuseTimerIndex);
							CATemp[MaxIndex] = pSpell->ID;
						}
						replaced = false;
						duplicatetimer = false;
					}
				}

			}
		}
	}
	//Should add stuff from the INI here so that it gets sorted.
	for (int i = 0; i < 30; i++) {
		CHAR temp[MAX_STRING] = "";
		CHAR Key[MAX_STRING] = "DiscRemove";
		CHAR KeyNum[MAX_STRING] = "";
		sprintf_s(KeyNum, MAX_STRING, "%s%i", Key, i);
		bool bfound = false;
		if (GetPrivateProfileString("DiscRemove", KeyNum, 0, temp, MAX_STRING, ThisINIFileName) != 0) {
			if (PSPELL pSpell = GetSpellByName(temp)) {
				for (int j = 0; j < NUM_COMBAT_ABILITIES; j++) {
					if (pCombatSkillsSelectWnd->ShouldDisplayThisSkill(j)) {
						if (PSPELL pFoundSpell = GetSpellByID(pPCData->GetCombatAbility(j))) {
							if (pFoundSpell->SpellGroup == pSpell->SpellGroup && !_strnicmp(pSpell->Name, pFoundSpell->Name, strlen(pSpell->Name))) {
								for (int k = 0; k < NUM_COMBAT_ABILITIES; k++) {
									if (CATemp[k] == pFoundSpell->ID) {
										CATemp[k] = -1;
										WriteChatf("\arRemoving %s based on INI settings.", pFoundSpell->Name);
										bfound = true;
										break;
									}
								}
							}
						}
					}
					if (bfound) break;
				}
			}
		}
		bfound = false;
	}
	for (int i = 0; i < 30; i++) {
		CHAR temp[MAX_STRING] = "";
		CHAR Key[MAX_STRING] = "DiscAdd";
		CHAR KeyNum[MAX_STRING] = "";
		sprintf_s(KeyNum, MAX_STRING, "%s%i", Key, i);
		bool bfound = false;
		if (GetPrivateProfileString("DiscAdd", KeyNum, 0, temp, MAX_STRING, ThisINIFileName) != 0) {
			if (PSPELL pSpell = GetSpellByName(temp)) {
				for (int j = 0; j < NUM_COMBAT_ABILITIES; j++) {
					if (pCombatSkillsSelectWnd->ShouldDisplayThisSkill(j)) {
						if (PSPELL pFoundSpell = GetSpellByID(pPCData->GetCombatAbility(j))) {
							if (pFoundSpell->SpellGroup == pSpell->SpellGroup && !_strnicmp(pSpell->Name, pFoundSpell->Name, strlen(pSpell->Name))) {
								for (int k = 0; k < NUM_COMBAT_ABILITIES; k++) {
									if (CATemp[k] == -1) {
										CATemp[k] = pFoundSpell->ID;
										WriteChatf("\agAdding %s from the INI", pFoundSpell->Name);
										bfound = true;
										break;
									}
								}
							}
						}
					}
					if (bfound) break;
				}
			}
		}
		bfound = false;
	}
	SingleDetrimental.clear();
	SingleBeneficial.clear();
	ToTBeneficial.clear();
	AEDetrimental.clear();
	SelfBeneficial.clear();
	GroupBeneficial.clear();
	Summons.clear();
	EndRegen.clear();
	Aura.clear();
	//BYTE    TargetType;         //03=Group v1, 04=PB AE, 05=Single, 06=Self, 08=Targeted AE, 0e=Pet, 28=AE PC v2, 29=Group v2, 2a=Directional, 46=Target Of Target
	//*0x180*/   BYTE    SpellType;          //0=detrimental, 1=Beneficial, 2=Beneficial, Group Only
	for (int i = 0; i < NUM_COMBAT_ABILITIES; i++) {
		if (CATemp[i] != -1) {
			if (PSPELL pSpell = GetSpellByID(CATemp[i])) {
				if (pSpell->Category == 132) {
					//Auras go here.
					Aura.push_back(pSpell);
					CATemp[i] = -1;
					continue;
				}
				if (pSpell->Category == 27 && pSpell->Subcategory == 151 && pSpell->TargetType == 6) {
					//Endurance Regen Discs
					EndRegen.push_back(pSpell);
					CATemp[i] = -1;
					continue;
				}
				if (pSpell->Category == 18) {
					//Summoning Disc (Zerker Axe Summons are Sub Category 110)
					Summons.push_back(pSpell);
					CATemp[i] = -1;
					continue;
				}
				if (pSpell->SpellType == 0 && pSpell->TargetType == 5) {
					//Single Target Detrimental
					SingleDetrimental.push_back(pSpell);
					CATemp[i] = -1;
					continue;
					//WriteChatf("\ar%s", pSpell->Name);
				}
				else if (pSpell->SpellType == 1 && pSpell->TargetType == 46) {
					//Beneficial Target of Target
					ToTBeneficial.push_back(pSpell);
					CATemp[i] = -1;
					continue;
					//WriteChatf("\au%s TargetType: %i", pSpell->Name, pSpell->TargetType);
				}
				else if (pSpell->SpellType == 0) {
					//All other Detrimentals AEs?
					AEDetrimental.push_back(pSpell);
					CATemp[i] = -1;
					continue;
				}
				else if (pSpell->SpellType == 1 && pSpell->TargetType == 06) {
					//Self Target Beneficial
					SelfBeneficial.push_back(pSpell);
					CATemp[i] = -1;
					continue;
					//WriteChatf("\ay%s", pSpell->Name);
				}
				else if (pSpell->SpellType == 1 && pSpell->TargetType == 05) {
					//Single Target Beneficial
					SingleBeneficial.push_back(pSpell);
					CATemp[i] = -1;
					continue;
					//WriteChatf("\ag%s", pSpell->Name);
				}
				else if (pSpell->SpellType == 2 || (pSpell->SpellType == 1 && (pSpell->TargetType == 3 || pSpell->TargetType == 29))) {
					//Group Beneficial
					GroupBeneficial.push_back(pSpell);
					CATemp[i] = -1;
					continue;
					//WriteChatf("\ap%s TargetType: %i", pSpell->Name, pSpell->TargetType);
				}
				else {
					//Everything else
					WriteChatf("\aoNoCategory:%s TargetType: %i SpellType: %i", pSpell->Name, pSpell->TargetType, pSpell->SpellType);
				}
			}
		}
	}
	WriteChatf("[Single Detrimental]");
	for (unsigned int i = 0; i < SingleDetrimental.size(); i++) {
		WriteChatf("\ar%s", SingleDetrimental.at(i)->Name);
	}
	WriteChatf("[AE Detrimental]");
	for (unsigned int i = 0; i < AEDetrimental.size(); i++) {
		WriteChatf("\au%s", AEDetrimental.at(i)->Name);
	}
	WriteChatf("[Self Beneficial]");
	for (unsigned int i = 0; i < SelfBeneficial.size(); i++) {
		WriteChatf("\ay%s", SelfBeneficial.at(i)->Name);
	}
	WriteChatf("[Single Beneficial]");
	for (unsigned int i = 0; i < SingleBeneficial.size(); i++) {
		WriteChatf("\ag%s", SingleBeneficial.at(i)->Name);
	}
	WriteChatf("[Group Beneficial]");
	for (unsigned int i = 0; i < GroupBeneficial.size(); i++) {
		WriteChatf("\ap%s", GroupBeneficial.at(i)->Name);
	}
	WriteChatf("[Target of Target Beneficial]");
	for (unsigned int i = 0; i < ToTBeneficial.size(); i++) {
		WriteChatf("\ao%s", ToTBeneficial.at(i)->Name);
	}
	WriteChatf("[Endurance Regen]");
	for (unsigned int i = 0; i < EndRegen.size(); i++) {
		WriteChatf("\ar%s", EndRegen.at(i)->Name);
	}
	WriteChatf("[Summoning Discs]");
	for (unsigned int i = 0; i < Summons.size(); i++) {
		WriteChatf("\ao%s", Summons.at(i)->Name);
	}
	WriteChatf("[Aura]");
	for (unsigned int i = 0; i < Aura.size(); i++) {
		WriteChatf("\ag%s", Aura.at(i)->Name);
	}
}

void UseDiscs()
{
	if (!useDiscs || !InGame()) return;
	if (GetCharInfo()->pSpawn->CastingData.IsCasting()) {
		if (Debugging) WriteChatf("already casting, returning");
		return;
	}

	for (unsigned int i = 0; i < SelfBeneficial.size(); i++) {
		if (DiscLastTimeUsed < GetTickCount64()) {
			if (DiscReady(SelfBeneficial.at(i))) {
				if (SelfBeneficial.at(i)->CanCastInCombat && !SelfBeneficial.at(i)->CastTime && !IHaveBuff(SelfBeneficial.at(i))) {
					WriteChatf("\agSelf Beneficial \at---\ar> \ap%s", SelfBeneficial.at(i)->Name);
					DiscLastTimeUsed = GetTickCount64();
					pCharData->DoCombatAbility(SelfBeneficial.at(i)->ID);
				}
			}
		}
	}

	for (unsigned int i = 0; i < GroupBeneficial.size(); i++) {
		if (DiscLastTimeUsed < GetTickCount64()) {
			if (DiscReady(GroupBeneficial.at(i))) {
				if (GroupBeneficial.at(i)->CanCastInCombat && !GroupBeneficial.at(i)->CastTime && !IHaveBuff(GroupBeneficial.at(i))) {
					WriteChatf("\a-pSelf Beneficial \at---\ar> \ap%s", GroupBeneficial.at(i)->Name);
					DiscLastTimeUsed = GetTickCount64();
					pCharData->DoCombatAbility(GroupBeneficial.at(i)->ID);
				}
			}
		}
	}

	for (unsigned int i = 0; i < SingleDetrimental.size(); i++) {
		if (DiscLastTimeUsed < GetTickCount64()) {
			if (DiscReady(SingleDetrimental.at(i))) {
				if (SingleDetrimental.at(i)->CanCastInCombat) {
					WriteChatf("\arSingleDetrimental \at---\ar> \ap%s", SingleDetrimental.at(i)->Name);
					DiscLastTimeUsed = GetTickCount64();
					pCharData->DoCombatAbility(SingleDetrimental.at(i)->ID);
				}
			}
		}
	}


}

bool DiscReady(PSPELL pSpell)
{
	if (!InGame()) return false;
	DWORD timeNow = (DWORD)time(NULL);
#if !defined(ROF2EMU) && !defined(UFEMU)
	if (pPCData->GetCombatAbilityTimer(pSpell->ReuseTimerIndex, pSpell->SpellGroup) < timeNow && !IHaveBuff(pSpell)) {
#else
	if (pPCData->GetCombatAbilityTimer(pSpell->ReuseTimerIndex) < timeNow && !IHaveBuff(pSpell)) {
#endif
		//If substring "Discipline" is found in the name of the disc, this is an active disc. Let's see if there is already one running.
		if (strstr(pSpell->Name, "Discipline")) {
			if (pCombatAbilityWnd) {
				if (CXWnd *Child = ((CXWnd*)pCombatAbilityWnd)->GetChildItem("CAW_CombatEffectLabel")) {
					CHAR szBuffer[2048] = { 0 };
					if (GetCXStr(Child->WindowText, szBuffer, MAX_STRING) && szBuffer[0] != '\0') {
						if (PSPELL pbuff = GetSpellByName(szBuffer)) {
							if (Debugging) WriteChatf("Already have an Active Disc: \ag%s, \awCan't use \ar%s", pbuff->Name, pSpell->Name);
							return false;
						}
					}
				}
			}
		}
		//If I have enough mana and endurance (Never know, might be mana requirements lol).
		PSPAWNINFO me = GetCharInfo()->pSpawn;
		if (me->ManaCurrent >= (int)pSpell->ManaCost && me->EnduranceCurrent >= (int)pSpell->EnduranceCost) {
			if (pTarget && me) {
				if (Distance3DToSpawn(me, (PSPAWNINFO)pTarget) > pSpell->Range) {
					return false;
				}
			}
			DWORD ReqID = pSpell->CasterRequirementID;
			if (ReqID == 518) {
				//Requires under 90% hitpoints
				if (PercentHealth(me) > 89) return false;
			}
			else if (ReqID == 825) {
				//Requires under 21% Endurance
				if (PercentEndurance(me) > 20) return false;
			}
			else if (ReqID == 826) {
				//Requires under 25% Endurance
				if (PercentEndurance(me) > 24) return false;
			}
			else if (ReqID == 827) {
				//Requires under 29% Endurance
				if (PercentEndurance(me) > 28) return false;
			}
			for (int i = 0; i < 4; i++) {
				if (pSpell->ReagentCount[i] > 0) {
					if (pSpell->ReagentID[i] != -1) {
						DWORD count = FindItemCountByID((int)pSpell->ReagentID[i]);
						if (count < pSpell->ReagentCount[i]) {
							if (PCHAR pitemname = GetItemFromContents(FindItemByID((int)pSpell->ReagentID[i]))->Name) {
								if (Debugging) WriteChatf("\ap%s\aw needs Reagent: \ay%s x \ag%i", pSpell->Name, pitemname, pSpell->ReagentCount[i]);
							}
							else {
								if (Debugging) WriteChatf("%s needs Item ID: %d x \ag%d", pSpell->Name, pSpell->ReagentID[i], pSpell->ReagentCount[i]);
							}
							return false;
						}
					}
				}
			}
		}
		else {
			return false;
		}
		return true;
	}
	return false;
}

bool IHaveBuff(PSPELL pSpell) {
	//WriteChatf("Checking for buff: %s", pSpell->Name);
	if (!InGame()) return false;
	for (int i = 0; i < NUM_LONG_BUFFS; i++) {
		if (PSPELL pBuff = GetSpellByID(GetCharInfo2()->Buff[i].SpellID)) {
			if (strstr(pBuff->Name, pSpell->Name)) {
				//WriteChatf("---Buff: %s strstr: %i", pBuff->Name, (int)strstr(pBuff->Name, pSpell->Name));
				return true;
			}
		}
	}
	for (int i = 0; i < NUM_SHORT_BUFFS; i++) {
		if (PSPELL pBuff = GetSpellByID(GetCharInfo2()->ShortBuff[i].SpellID)) {
			if (strstr(pBuff->Name, pSpell->Name)) {
				//WriteChatf("---Buff: %s strstr: %i", pBuff->Name, (int)strstr(pBuff->Name, pSpell->Name));
				return true;
			}
		}
	}
	return false;
}

void RestRoutines() {
	for (unsigned int i = 0; i < Aura.size(); i++) {
		if (DiscLastTimeUsed < GetTickCount64()) {
			if (DiscReady(Aura.at(i))) {
				if (!IHaveBuff(Aura.at(i))) {
					if (!Casting()) {
						WriteChatf("\atAura \at---\ar> \ap%s", Aura.at(i)->Name);
						DiscLastTimeUsed = GetTickCount64();
						pCharData->DoCombatAbility(Aura.at(i)->ID);
					}
				}
			}
		}
	}
	for (unsigned int i = 0; i < EndRegen.size(); i++) {
		if (DiscLastTimeUsed < GetTickCount64()) {
			if (DiscReady(EndRegen.at(i))) {
				if (!IHaveBuff(EndRegen.at(i))) {
					if (!Casting()) {
						WriteChatf("\atEndurance \at---\ar> \ap%s", EndRegen.at(i)->Name);
						DiscLastTimeUsed = GetTickCount64();
						pCharData->DoCombatAbility(EndRegen.at(i)->ID);
					}
				}
			}
		}
	}
}

inline bool Casting() {
	return GetCharInfo()->pSpawn->CastingData.IsCasting();
}

//NotUsed - Complete TODO!
void PermIgnoreCommand(PSPAWNINFO pChar, PCHAR szline) {
	if (!InGame()) return;
	char temp[MAX_STRING] = { 0 };
	char temp1[MAX_STRING] = { 0 };
#define pZone ((PZONEINFO)pZoneInfo)
	VerifyINI(pZone->ShortName, "Ignored", "|", IgnoresFileName);
	GetPrivateProfileString(pZone->ShortName, "Ignored", "|", temp, MAX_STRING, IgnoresFileName);
	if (!strlen(szline) && pTarget) {
		if (!strstr(temp, ((PSPAWNINFO)pTarget)->DisplayedName)) {
			sprintf_s(temp1, MAX_STRING, "%s|", ((PSPAWNINFO)pTarget)->DisplayedName);
			strcat_s(temp, MAX_STRING, temp1);
			WritePrivateProfileString(pZone->ShortName, "Ignored", temp, IgnoresFileName);
			WriteChatf("%s\ap%s\ay added to permanent ignore list.", PLUGINMSG, ((PSPAWNINFO)pTarget)->DisplayedName);
			return;
		}
		else {
			WriteChatf("%s\ap%s\ar is already on the permanent ignore list in the INI", PLUGINMSG, ((PSPAWNINFO)pTarget)->DisplayedName);
			return;
		}
	}
	else if (strlen(szline)) {
		if (!strstr(temp, szline)) {
			sprintf_s(temp1, MAX_STRING, "%s|", szline);
			strcat_s(temp, MAX_STRING, temp1);
			WritePrivateProfileString(pZone->ShortName, "Ignored", temp, IgnoresFileName);
			WriteChatf("%s\ap%s\ay added to permanent ignore list.", PLUGINMSG, szline);
			return;
		}
		else {
			WriteChatf("%s\ap%s\ar is already on the permanent ignore list in the INI", PLUGINMSG, szline);
			return;
		}
	}

#undef pZone
}


//NotUsed
void SummonThings(vector<PSPELL> spellList) {
	for (unsigned int i = 0; i < spellList.size(); i++) {
		if (PSPELL pSpell = spellList.at(i)) {
			if (DiscReady(pSpell)) {
				//now to figure out how to handle if I already have the item or not.
				//Cast the Summon.
			}
		}
	}
}

void ShowSettings() {
	//General Section
	WriteChatf("%s\ar[\a-tGeneral\ar]", PLUGINMSG);
	WriteChatf("%s\ayDebugging\ar:%s", PLUGINMSG, Debugging ? "\agTRUE" : "\arFALSE");
	WriteChatf("%s\ayCastDetrimental\ar:%s", PLUGINMSG, CastDetrimental ? "\agTRUE" : "\arFALSE");
	//Pull Section
	WriteChatf("%s\ar[\a-tPull\ar]", PLUGINMSG);
	WriteChatf("%s\ayRadius\ar:\ag%i", PLUGINMSG, Radius);
	WriteChatf("%s\ayZRadius\ar:\ag%i", PLUGINMSG, ZRadius);
	WriteChatf("%s\ayPullAbility\ar: NOT USED CURRENTLY!", PLUGINMSG);
	//Health Section
	WriteChatf("%s\ar[\a-tHealth\ar]",PLUGINMSG);
	WriteChatf("%s\ayHealAt\ar:\ag%i", PLUGINMSG, HealAt);
	WriteChatf("%s\ayHealTill\ar:\ag%i", PLUGINMSG, HealTill);
	//Mana Section
	WriteChatf("%s\ar[\a-tMana\ar]", PLUGINMSG);
	WriteChatf("%s\ayMedAt\ar:\ag%i", PLUGINMSG, MedAt);
	WriteChatf("%s\ayMedTill\ar:\ag%i", PLUGINMSG, MedTill);
	//Endurance Section
	WriteChatf("%s\ar[\a-tEndurance\ar]", PLUGINMSG);
	WriteChatf("%s\ayMedEndAt\ar:\ag%i", PLUGINMSG, MedEndAt);
	WriteChatf("%s\ayMedEndTill\ar:\ag%i", PLUGINMSG, MedEndTill);
}
