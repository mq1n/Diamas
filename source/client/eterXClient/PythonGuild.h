#pragma once

#include "Packet.h"

class CPythonGuild : public CSingleton<CPythonGuild>
{
	public:
		enum
		{
			GUILD_SKILL_MAX_NUM = 12,
			ENEMY_GUILD_SLOT_MAX_COUNT = 6,
		};

		typedef struct SGulidInfo
		{
			uint32_t dwGuildID;
			char szGuildName[GUILD_NAME_MAX_LEN+1];
			uint32_t dwMasterPID;
			uint32_t dwGuildLevel;
			uint32_t dwCurrentExperience;
			uint32_t dwCurrentMemberCount;
			uint32_t dwMaxMemberCount;
			uint32_t dwGuildMoney;
			BOOL bHasLand;
		} TGuildInfo;

		typedef struct SGuildGradeData
		{
			SGuildGradeData(){}
			SGuildGradeData(uint8_t byAuthorityFlag_, const char * c_szName_) : byAuthorityFlag(byAuthorityFlag_), strName(c_szName_) {}
			uint8_t byAuthorityFlag;
			std::string strName;
		} TGuildGradeData;
		typedef std::map<uint8_t, TGuildGradeData> TGradeDataMap;

		typedef struct SGuildMemberData
		{
			uint32_t dwPID;

			std::string strName;
			uint8_t byGrade;
			uint8_t byJob;
			uint8_t byLevel;
			uint8_t byGeneralFlag;
			uint32_t dwOffer;
		} TGuildMemberData;
		typedef std::vector<TGuildMemberData> TGuildMemberDataVector;

		typedef struct SGuildBoardCommentData
		{
			uint32_t dwCommentID;
			std::string strName;
			std::string strComment;
		} TGuildBoardCommentData;
		typedef std::vector<TGuildBoardCommentData> TGuildBoardCommentDataVector;

		typedef struct SGuildSkillData
		{
			uint8_t bySkillPoint;
			uint8_t bySkillLevel[GUILD_SKILL_MAX_NUM];
			uint16_t wGuildPoint;
			uint16_t wMaxGuildPoint;
		} TGuildSkillData;

		typedef std::map<uint32_t, std::string> TGuildNameMap;

	public:
		CPythonGuild();
		virtual ~CPythonGuild();

		void Destroy();

		void EnableGuild();
		void SetGuildMoney(uint32_t dwMoney);
		void SetGuildEXP(uint8_t byLevel, uint32_t dwEXP);
		void SetGradeData(uint8_t byGradeNumber, TGuildGradeData & rGuildGradeData);
		void SetGradeName(uint8_t byGradeNumber, const char * c_szName);
		void SetGradeAuthority(uint8_t byGradeNumber, uint8_t byAuthority);
		void ClearComment();
		void RegisterComment(uint32_t dwCommentID, const char * c_szName, const char * c_szComment);
		void RegisterMember(TGuildMemberData & rGuildMemberData);
		void ChangeGuildMemberGrade(uint32_t dwPID, uint8_t byGrade);
		void ChangeGuildMemberGeneralFlag(uint32_t dwPID, uint8_t byFlag);
		void RemoveMember(uint32_t dwPID);
		void RegisterGuildName(uint32_t dwID, const char * c_szName);

		BOOL IsMainPlayer(uint32_t dwPID);
		BOOL IsGuildEnable();
		TGuildInfo & GetGuildInfoRef();
		BOOL GetGradeDataPtr(uint32_t dwGradeNumber, TGuildGradeData ** ppData);
		const TGuildBoardCommentDataVector & GetGuildBoardCommentVector();
		uint32_t GetMemberCount();
		BOOL GetMemberDataPtr(uint32_t dwIndex, TGuildMemberData ** ppData);
		BOOL GetMemberDataPtrByPID(uint32_t dwPID, TGuildMemberData ** ppData);
		BOOL GetMemberDataPtrByName(const char * c_szName, TGuildMemberData ** ppData);
		uint32_t GetGuildMemberLevelSummary();
		uint32_t GetGuildMemberLevelAverage();
		uint32_t GetGuildExperienceSummary();
		TGuildSkillData & GetGuildSkillDataRef();
		bool GetGuildName(uint32_t dwID, std::string * pstrGuildName);
		uint32_t GetGuildID();
		BOOL HasGuildLand();

		void StartGuildWar(uint32_t dwEnemyGuildID);
		void EndGuildWar(uint32_t dwEnemyGuildID);
		uint32_t GetEnemyGuildID(uint32_t dwIndex);
		BOOL IsDoingGuildWar();

	protected:
		void __CalculateLevelAverage();
		void __SortMember();
		BOOL __IsGradeData(uint8_t byGradeNumber);

		void __Initialize();

	protected:
		TGuildInfo m_GuildInfo;
		TGradeDataMap m_GradeDataMap;
		TGuildMemberDataVector m_GuildMemberDataVector;
		TGuildBoardCommentDataVector m_GuildBoardCommentVector;
		TGuildSkillData m_GuildSkillData;
		TGuildNameMap m_GuildNameMap;
		uint32_t m_adwEnemyGuildID[ENEMY_GUILD_SLOT_MAX_COUNT];

		uint32_t m_dwMemberLevelSummary;
		uint32_t m_dwMemberLevelAverage;
		uint32_t m_dwMemberExperienceSummary;

		BOOL m_bGuildEnable;
};