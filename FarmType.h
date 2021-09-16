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

	virtual bool GetMember(MQVarPtr VarPtr, const char* Member, char* Index, MQTypeVar& Dest) override
	{
		MQTypeMember* pMember = FindMember(Member);
		if (!pMember)
			return false;
		switch (pMember->ID)
		{
		case TargetID:
			Dest.DWord = MyTargetID;
			Dest.Type = datatypes::pIntType;
			return true;
		case Version:
            sprintf_s(DataTypeTemp, "%1.2f", MQ2Version);
            Dest.Ptr  = &DataTypeTemp[0];
			Dest.Type = datatypes::pStringType;
			return true;
		}
		return false;
	}
};

inline class MQ2FarmType *pFarmType = nullptr;

inline bool dataFarm(const char* szIndex, MQTypeVar& Ret)
{
	if (szIndex != NULL)
	{
		//${Farm[szIndex]} gets used, the output goes here.
	}

	Ret.DWord = 1;
	Ret.Type = pFarmType;
	return true;
}