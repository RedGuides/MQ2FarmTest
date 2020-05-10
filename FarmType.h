class MQ2FarmType : public MQ2Type
{
public:
	enum FarmMembers
	{
		TargetID = 1,
		Version = 2,
	};

	MQ2FarmType() :MQ2Type("Farm")
	{
		TypeMember(TargetID);//1,
		TypeMember(Version);//2,
	}

	~MQ2FarmType()
	{
	}

	bool GetMember(MQ2VARPTR VarPtr, char* Member, char* Index, MQ2TYPEVAR &Dest)
	{
		PMQ2TYPEMEMBER pMember = MQ2FarmType::FindMember(Member);
		if (!pMember)
			return false;
		switch ((FarmMembers)pMember->ID)
		{
		case TargetID:
			Dest.DWord = MyTargetID;
			Dest.Type = pIntType;
			return true;
		case Version:
			Dest.Ptr = VERSION;
			Dest.Type = pStringType;
			return true;
		}
		return false;
	}

	bool ToString(MQ2VARPTR VarPtr, char* Destination)
	{
		return true;
	}

	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source)
	{
		return false;
	}

	bool FromString(MQ2VARPTR &VarPtr, char* Source)
	{
		return false;
	}
};

class MQ2FarmType *pFarmType = 0;

BOOL dataFarm(char* szIndex, MQ2TYPEVAR &Ret)
{
	if (szIndex != NULL)
	{
		//${Farm[szIndex]} gets used, the output goes here.
	}

	Ret.DWord = 1;
	Ret.Type = pFarmType;
	return true;
}