//Prototypes
bool AmIReady();
bool atob(char* x);
bool DiscReady(PSPELL);
bool HaveAggro();
bool IHaveBuff(PSPELL);
bool MeshLoaded();
bool NavActive();
bool NavPaused();
bool PathExists(unsigned long SpawnID);
bool SpellsMemorized();

unsigned long getFirstAggroed();
unsigned long SearchSpawns(char* szIndex);

float AmFacing(unsigned long ID);
float PathLength(unsigned long SpawnID);

inline bool Casting();
inline bool InGame();
inline float PercentMana(PlayerClient* pSpawn);
inline float PercentHealth(PlayerClient* pSpawn);
inline float PercentEndurance(PlayerClient* pSpawn);

static MQPlugin* FindMQ2NavPlugin();

void CastDetrimentalSpells();
void CheckAlias();
void ClearTarget();
void DiscSetup();
void DoINIThings();
void FarmCommand(PSPAWNINFO pChar, char* Line);
void IgnoreThisCommand(PSPAWNINFO pChar, char* szLine);
void IgnoreTheseCommand(PSPAWNINFO pChar, char* szLine);
void ListCommands();
void NavCommand(char* szLine);
void NavEnd();
void NavigateToID(unsigned long ID);
void PermIgnoreCommand(PSPAWNINFO pchar, char* szline);
void PluginOn();
void PluginOff();
void RestRoutines();
void SummonThings(std::vector<PSPELL>);
void ShowSettings();
void UpdateSearchString();
void UseDiscs();
void VerifyINI(char* ININame, char* Section, char* Key, char* Default);

//End Prototypes