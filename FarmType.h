class MQ2FarmType : public MQ2Type
{
public:
	enum FarmMembers
	{
		TargetID = 1,
	};
	MQ2FarmType() :MQ2Type("farm")
	{
		TypeMember(TargetID);//1,
	}
	~MQ2FarmType()
	{
	}
	bool MQ2FarmType::GETMEMBER()
	{
		PMQ2TYPEMEMBER pMember = MQ2FarmType::FindMember(Member);
		if (pMember)
		{
			WriteChatf("There was a pMember");
			switch ((FarmMembers)pMember->ID)
			{
			case 1:
				return true;
			}
		}
		return false;
	}

};