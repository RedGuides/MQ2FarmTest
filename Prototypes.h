//Prototypes
bool AmIReady();
bool atob(char x[MAX_STRING]);
bool DiscReady(PSPELL);
bool HaveAggro();
bool IHaveBuff(PSPELL);
bool MeshLoaded();
bool NavActive();
bool NavPaused();
bool PathExists(DWORD SpawnID);
bool SpellsMemorized();

DWORD getFirstAggroed();
DWORD SearchSpawns(char szIndex[MAX_STRING]);

float AmFacing(DWORD ID);
float PathLength(DWORD SpawnID);

inline bool Casting();
inline bool InGame();
inline float PercentMana();
inline float PercentHealth();
inline float PercentEndurance();

static PMQPLUGIN FindMQ2NavPlugin();

void CastDetrimentalSpells();
void CheckAlias();
void ClearTarget();
void DiscSetup();
void DoINIThings();
void FarmCommand(PSPAWNINFO pChar, PCHAR Line);
void IgnoreThisCommand(PSPAWNINFO pChar, PCHAR szLine);
void IgnoreTheseCommand(PSPAWNINFO pChar, PCHAR szLine);
void ListCommands();
void NavCommand(PSPAWNINFO pChar, PCHAR szLine);
void NavEnd(PSPAWNINFO pChar);
void NavigateToID(DWORD ID);
void PluginOn();
void PluginOff();
void RestRoutines();
void SummonThings(std::vector<PSPELL>);
void UpdateSearchString();
void UseDiscs();
void VerifyINI(char ININame[MAX_STRING], char Section[MAX_STRING], char Key[MAX_STRING], char Default[MAX_STRING]);

#define TargetIt(X) *(PSPAWNINFO*)ppTarget=X

//End Prototypes