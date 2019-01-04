//variables
bool activated = false;
bool CastDetrimental = true;
bool Debugging = false;
bool GettingEndurance = false;
bool GettingHealth = false;
bool GettingMana = false;
bool useLogOut = false;
bool useEQBC = false;
bool useMerc = false;
bool useDiscs = true;
bool PullRequiresLineOfSight = true;
bool Paused = false;

char IgnoreINI[MAX_STRING] = { 0 };
char FarmMob[MAX_STRING] = { 0 };
char PullAbility[MAX_STRING] = { 0 };
char searchString[MAX_STRING] = { 0 };

DWORD MyTargetID = 0;
unsigned __int64 DiscLastTimeUsed = GetTickCount64();
int HealAt = 0;
int HealTill = 0;
int MedEndAt = 0;
int MedEndTill = 0;
int MedAt;
int MedTill = 0;

int CampRadius = 0;
int GemIndex = 0;
int PullAbilityRange = 0;
int PulseDelay = 30;
int Pulse = 0;
int Radius = 0;
int ZRadius = 0;

std::vector<PSPELL> SingleDetrimental; //used in combat
std::vector<PSPELL> AEDetrimental; //used in combat
std::vector<PSPELL> ToTBeneficial; //used in combat

std::vector<PSPELL> SingleBeneficial; //out of combat
std::vector<PSPELL> SelfBeneficial; //out of combat
std::vector<PSPELL> GroupBeneficial; //out of combat

//End Variables