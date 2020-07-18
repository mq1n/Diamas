#include "stdafx.h"
#include "guild.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "desc_client.h"
#include "buffer_manager.h"
#include "char_manager.h"
#include "db.h"
#include "guild_manager.h"
#include "affect.h"
#include "p2p.h"
#include "quest_manager.h"
#include "building.h"
#include "locale_service.h"
#include "log.h"
#include "quest_manager.h"

SGuildMember::SGuildMember(LPCHARACTER ch, uint8_t grade, uint32_t offer_exp) :
	pid(ch->GetPlayerID()), grade(grade), is_general(0), job(ch->GetJob()), level(ch->GetLevel()), offer_exp(offer_exp), name(ch->GetName())
{
}
SGuildMember::SGuildMember(uint32_t pid, uint8_t grade, uint8_t is_general, uint8_t job, uint8_t level, uint32_t offer_exp, char* name) :
	pid(pid), grade(grade), is_general(is_general), job(job), level(level), offer_exp(offer_exp), name(name)
{
}

namespace 
{
	struct FGuildNameSender
	{
		FGuildNameSender(uint32_t id, const char* guild_name) : id(id), name(guild_name)
		{
			p.subheader = GUILD_SUBHEADER_GC_GUILD_NAME;
			p.size = sizeof(p) + sizeof(uint32_t) + GUILD_NAME_MAX_LEN;
		}

		void operator() (LPCHARACTER ch)
		{
			LPDESC d = ch->GetDesc();

			if (d)
			{
				d->BufferedPacket(&p, sizeof(p));
				d->BufferedPacket(&id, sizeof(id));
				d->Packet(name, GUILD_NAME_MAX_LEN);
			}
		}

		uint32_t id;
		const char * name;
		SPacketGCGuild p;
	};
}

CGuild::CGuild(TGuildCreateParameter & cp)
{
	Initialize();

	m_general_count = 0;

	m_iMemberCountBonus = 0;

	strlcpy(m_data.name, cp.name, sizeof(m_data.name));
	m_data.master_pid = cp.master->GetPlayerID();
	strlcpy(m_data.grade_array[0].grade_name, LC_TEXT("길드장"), sizeof(m_data.grade_array[0].grade_name));
	m_data.grade_array[0].auth_flag = GUILD_AUTH_ADD_MEMBER | GUILD_AUTH_REMOVE_MEMBER | GUILD_AUTH_NOTICE | GUILD_AUTH_SKILL;

	for (int32_t i = 1; i < GUILD_GRADE_COUNT; ++i)
	{
		strlcpy(m_data.grade_array[i].grade_name, LC_TEXT("길드원"), sizeof(m_data.grade_array[i].grade_name));
		m_data.grade_array[i].auth_flag = 0;
	}

	std::unique_ptr<SQLMsg> pmsg (DBManager::Instance().DirectQuery(
				"INSERT INTO guild(name, master, sp, level, exp, skill_point, skill) "
				"VALUES('%s', %u, 1000, 1, 0, 0, '\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0')", 
				m_data.name, m_data.master_pid));

	// TODO if error occur?
	m_data.guild_id = pmsg->Get()->uiInsertID;

	for (int32_t i = 0; i < GUILD_GRADE_COUNT; ++i)
	{
		DBManager::Instance().Query("INSERT INTO guild_grade VALUES(%u, %d, '%s', %d)",
				m_data.guild_id, 
				i + 1, 
				m_data.grade_array[i].grade_name, 
				m_data.grade_array[i].auth_flag);
	}

	ComputeGuildPoints();
	m_data.power	= m_data.max_power;
	m_data.ladder_point	= 0;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_CREATE, 0, &m_data.guild_id, sizeof(uint32_t));

	TPacketGuildSkillUpdate guild_skill;
	guild_skill.guild_id = m_data.guild_id;
	guild_skill.amount = 0;
	guild_skill.skill_point = 0;
	memset(guild_skill.skill_levels, 0, GUILD_SKILL_COUNT);

	db_clientdesc->DBPacket(HEADER_GD_GUILD_SKILL_UPDATE, 0, &guild_skill, sizeof(guild_skill));

	// TODO GUILD_NAME
	CHARACTER_MANAGER::Instance().for_each_pc(FGuildNameSender(GetID(), GetName()));
	/*
	   TPacketDGGuildMember p;
	   memset(&p, 0, sizeof(p));
	   p.dwPID = cp.master->GetPlayerID();
	   p.bGrade = 15;
	   AddMember(&p);
	 */
	RequestAddMember(cp.master, GUILD_LEADER_GRADE);
}

void CGuild::Initialize()
{
	memset(&m_data, 0, sizeof(m_data));
	m_data.level = 1;

	for (int32_t i = 0; i < GUILD_SKILL_COUNT; ++i)
		abSkillUsable[i] = true;

	m_iMemberCountBonus = 0;
}

CGuild::~CGuild()
{
}

void CGuild::RequestAddMember(LPCHARACTER ch, int32_t grade)
{
	if (ch->GetGuild())
		return;

	TPacketGDGuildAddMember gd;

	if (m_member.find(ch->GetPlayerID()) != m_member.end())
	{
		sys_err("Already a member in guild %s[%d]", ch->GetName(), ch->GetPlayerID());
		return;
	}

	gd.dwPID = ch->GetPlayerID();
	gd.dwGuild = GetID();
	gd.bGrade = grade;

	db_clientdesc->DBPacket(HEADER_GD_GUILD_ADD_MEMBER, 0, &gd, sizeof(TPacketGDGuildAddMember));
}

void CGuild::AddMember(TPacketDGGuildMember * p)
{
	TGuildMemberContainer::iterator it;

	if ((it = m_member.find(p->dwPID)) == m_member.end())
		m_member.insert(std::make_pair(p->dwPID, TGuildMember(p->dwPID, p->bGrade, p->isGeneral, p->bJob, p->bLevel, p->dwOffer, p->szName)));
	else
	{
		TGuildMember & r_gm = it->second;
		r_gm.pid = p->dwPID;
		r_gm.grade = p->bGrade;
		r_gm.job = p->bJob;
		r_gm.offer_exp = p->dwOffer;
		r_gm.is_general = p->isGeneral;
	}

	CGuildManager::Instance().Link(p->dwPID, this);

	SendListOneToAll(p->dwPID);

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(p->dwPID);

	sys_log(0, "GUILD: AddMember PID %u, grade %u, job %u, level %u, offer %u, name %s ptr %p",
			p->dwPID, p->bGrade, p->bJob, p->bLevel, p->dwOffer, p->szName, get_pointer(ch));

	if (ch)
		LoginMember(ch);
	else
		P2PLoginMember(p->dwPID);
}

bool CGuild::RequestRemoveMember(uint32_t pid)
{
	TGuildMemberContainer::iterator it;

	if ((it = m_member.find(pid)) == m_member.end())
		return false;

	if (it->second.grade == GUILD_LEADER_GRADE)
		return false;

	TPacketGuild gd_guild;

	gd_guild.dwGuild = GetID();
	gd_guild.dwInfo = pid;

	db_clientdesc->DBPacket(HEADER_GD_GUILD_REMOVE_MEMBER, 0, &gd_guild, sizeof(TPacketGuild));
	return true;
}

bool CGuild::RemoveMember(uint32_t pid)
{
	sys_log(0, "Receive Guild P2P RemoveMember");
	TGuildMemberContainer::iterator it;

	if ((it = m_member.find(pid)) == m_member.end())
		return false;

	if (it->second.grade == GUILD_LEADER_GRADE)
		return false;

	if (it->second.is_general)
		m_general_count--;

	m_member.erase(it);
	SendOnlineRemoveOnePacket(pid);

	CGuildManager::Instance().Unlink(pid);

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pid);

	if (ch)
	{
		//GuildRemoveAffect(ch);
		m_memberOnline.erase(ch);
		ch->SetGuild(nullptr);
	}

	return true;
}

void CGuild::P2PLoginMember(uint32_t pid)
{
	if (m_member.find(pid) == m_member.end())
	{
		sys_err("GUILD [%d] is not a member of guild.", pid);
		return;
	}

	m_memberP2POnline.insert(pid);

	// Login event occur + Send List
	TGuildMemberOnlineContainer::iterator it;

	for (it = m_memberOnline.begin(); it!=m_memberOnline.end();++it)
		SendLoginPacket(*it, pid);
}

void CGuild::LoginMember(LPCHARACTER ch)
{
	if (m_member.find(ch->GetPlayerID()) == m_member.end())
	{
		sys_err("GUILD %s[%d] is not a member of guild.", ch->GetName(), ch->GetPlayerID());
		return;
	}

	ch->SetGuild(this);

	// Login event occur + Send List
	TGuildMemberOnlineContainer::iterator it;

	for (it = m_memberOnline.begin(); it!=m_memberOnline.end();++it)
		SendLoginPacket(*it, ch);

	m_memberOnline.insert(ch);

	SendAllGradePacket(ch);
	SendGuildInfoPacket(ch);
	SendListPacket(ch);
	SendSkillInfoPacket(ch);
	SendEnemyGuild(ch);

	//GuildUpdateAffect(ch);
}

void CGuild::P2PLogoutMember(uint32_t pid)
{
	if (m_member.find(pid)==m_member.end())
	{
		sys_err("GUILD [%d] is not a member of guild.", pid);
		return;
	}

	m_memberP2POnline.erase(pid);

	// Logout event occur
	TGuildMemberOnlineContainer::iterator it;
	for (it = m_memberOnline.begin(); it!=m_memberOnline.end();++it)
	{
		SendLogoutPacket(*it, pid);
	}
}

void CGuild::LogoutMember(LPCHARACTER ch)
{
	if (m_member.find(ch->GetPlayerID())==m_member.end())
	{
		sys_err("GUILD %s[%d] is not a member of guild.", ch->GetName(), ch->GetPlayerID());
		return;
	}

	//GuildRemoveAffect(ch);

	//ch->SetGuild(nullptr);
	m_memberOnline.erase(ch);

	// Logout event occur
	TGuildMemberOnlineContainer::iterator it;
	for (it = m_memberOnline.begin(); it!=m_memberOnline.end();++it)
	{
		SendLogoutPacket(*it, ch);
	}
}

void CGuild::SendOnlineRemoveOnePacket(uint32_t pid)
{
	SPacketGCGuild pack;
	pack.size = sizeof(pack)+4;
	pack.subheader = GUILD_SUBHEADER_GC_REMOVE;

	TEMP_BUFFER buf;
	buf.write(&pack,sizeof(pack));
	buf.write(&pid, sizeof(pid));

	TGuildMemberOnlineContainer::iterator it;

	for (it = m_memberOnline.begin(); it!=m_memberOnline.end();++it)
	{
		LPDESC d = (*it)->GetDesc();

		if (d)
			d->Packet(buf.read_peek(), buf.size());
	}
}

void CGuild::SendAllGradePacket(LPCHARACTER ch)
{
	LPDESC d = ch->GetDesc();
	if (!d)
		return;

	SPacketGCGuild pack;
	pack.size = sizeof(pack)+1+GUILD_GRADE_COUNT*(sizeof(TGuildGrade)+1);
	pack.subheader = GUILD_SUBHEADER_GC_GRADE;

	TEMP_BUFFER buf;

	buf.write(&pack, sizeof(pack));
	uint8_t n = 15;
	buf.write(&n, 1);

	for (int32_t i=0;i<GUILD_GRADE_COUNT;i++)
	{
		uint8_t j = i+1;
		buf.write(&j, 1);
		buf.write(&m_data.grade_array[i], sizeof(TGuildGrade));
	}

	d->Packet(buf.read_peek(), buf.size());
}

void CGuild::SendListOneToAll(LPCHARACTER ch)
{
	SendListOneToAll(ch->GetPlayerID());
}

void CGuild::SendListOneToAll(uint32_t pid)
{
	SPacketGCGuild pack;
	pack.size = sizeof(SPacketGCGuild);
	pack.subheader = GUILD_SUBHEADER_GC_LIST;

	pack.size += sizeof(TGuildMemberPacketData);

	char c[CHARACTER_NAME_MAX_LEN+1];
	memset(c, 0, sizeof(c));

	TGuildMemberContainer::iterator cit = m_member.find(pid);
	if (cit == m_member.end())
		return;

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it!= m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();
		if (!d) 
			continue;

		TEMP_BUFFER buf;

		buf.write(&pack, sizeof(pack));

		cit->second._dummy = 1;

		buf.write(&(cit->second), sizeof(uint32_t) * 3 +1);
		buf.write(cit->second.name.c_str(), cit->second.name.length());
		buf.write(c, CHARACTER_NAME_MAX_LEN + 1 - cit->second.name.length());
		d->Packet(buf.read_peek(), buf.size());
	}
}

void CGuild::SendListPacket(LPCHARACTER ch)
{
	/*
	   List Packet

	   Header
	   Count (byte)
	   [
	   ...
	   name_flag 1 - 이름을 보내느냐 안보내느냐
	   name CHARACTER_NAME_MAX_LEN+1
	   ] * Count

	 */
	if (!ch)
		return;

	LPDESC d;
	if (!(d=ch->GetDesc()))
		return;

	SPacketGCGuild pack;
	pack.size = sizeof(SPacketGCGuild);
	pack.subheader = GUILD_SUBHEADER_GC_LIST;

	pack.size += sizeof(TGuildMemberPacketData) * m_member.size();

	TEMP_BUFFER buf;

	buf.write(&pack,sizeof(pack));

	char c[CHARACTER_NAME_MAX_LEN+1];

	for (auto it = m_member.begin(); it != m_member.end(); ++it)
	{
		it->second._dummy = 1;

		buf.write(&(it->second), sizeof(uint32_t)*3+1);

		strlcpy(c, it->second.name.c_str(), MIN(sizeof(c), it->second.name.length() + 1));

		buf.write(c, CHARACTER_NAME_MAX_LEN+1 );

		if ( g_bIsTestServer )
			sys_log(0 ,"name %s job %d  ", it->second.name.c_str(), it->second.job );
	}

	d->Packet(buf.read_peek(), buf.size());

	for (auto it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		SendLoginPacket(ch, *it);
	}

	for (auto it = m_memberP2POnline.begin(); it != m_memberP2POnline.end(); ++it)
	{
		SendLoginPacket(ch, *it);
	}

}

void CGuild::SendLoginPacket(LPCHARACTER ch, LPCHARACTER chLogin)
{
	SendLoginPacket(ch, chLogin->GetPlayerID());
}

void CGuild::SendLoginPacket(LPCHARACTER ch, uint32_t pid)
{
	/*
	   Login Packet
	   header 4
	   pid 4
	 */
	if (!ch->GetDesc())
		return;

	SPacketGCGuild pack;
	pack.size = sizeof(pack)+4;
	pack.subheader = GUILD_SUBHEADER_GC_LOGIN;

	TEMP_BUFFER buf;

	buf.write(&pack, sizeof(pack));

	buf.write(&pid, 4);

	ch->GetDesc()->Packet(buf.read_peek(), buf.size());
}

void CGuild::SendLogoutPacket(LPCHARACTER ch, LPCHARACTER chLogout)
{
	SendLogoutPacket(ch, chLogout->GetPlayerID());
}

void CGuild::SendLogoutPacket(LPCHARACTER ch, uint32_t pid)
{
	/*
	   Logout Packet
	   header 4
	   pid 4
	 */
	if (!ch->GetDesc())
		return;

	SPacketGCGuild pack;
	pack.size = sizeof(pack)+4;
	pack.subheader = GUILD_SUBHEADER_GC_LOGOUT;

	TEMP_BUFFER buf;

	buf.write(&pack, sizeof(pack));
	buf.write(&pid, 4);

	ch->GetDesc()->Packet(buf.read_peek(), buf.size());
}

void CGuild::LoadGuildMemberData(SQLMsg* pmsg)
{
	if (pmsg->Get()->uiNumRows == 0)
		return;

	m_general_count = 0;

	m_member.clear();

	for (size_t i = 0; i < pmsg->Get()->uiNumRows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

		uint32_t pid = strtoul(row[0], (char**) nullptr, 10);
		uint8_t grade = (uint8_t) strtoul(row[1], (char**) nullptr, 10);
		uint8_t is_general = 0;

		if (row[2] && *row[2] == '1')
			is_general = 1;

		uint32_t offer = strtoul(row[3], (char**) nullptr, 10);
		uint8_t level = (uint8_t)strtoul(row[4], (char**) nullptr, 10);
		uint8_t job = (uint8_t)strtoul(row[5], (char**) nullptr, 10);
		char * name = row[6];

		if (is_general)
			m_general_count++;

		m_member.insert(std::make_pair(pid, TGuildMember(pid, grade, is_general, job, level, offer, name)));
		CGuildManager::Instance().Link(pid, this);
	}
}

void CGuild::LoadGuildGradeData(SQLMsg* pmsg)
{
	/*
    // 15개 아닐 가능성 존재
	if (pmsg->Get()->iNumRows != 15)
	{
		sys_err("Query failed: getting guild grade data. GuildID(%d)", GetID());
		return;
	}
	*/
	for (size_t i = 0; i < pmsg->Get()->uiNumRows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
		uint8_t grade = 0;
		str_to_number(grade, row[0]);
		char * name = row[1];
		uint32_t auth = strtoul(row[2], nullptr, 10);

		if (grade >= 1 && grade <= 15)
		{
			//sys_log(0, "GuildGradeLoad %s", name);
			strlcpy(m_data.grade_array[grade-1].grade_name, name, sizeof(m_data.grade_array[grade-1].grade_name));
			m_data.grade_array[grade-1].auth_flag = auth;
		}
	}
}
void CGuild::LoadGuildData(SQLMsg* pmsg)
{
	if (pmsg->Get()->uiNumRows == 0)
	{
		sys_err("Query failed: getting guild data %s", pmsg->stQuery.c_str());
		return;
	}

	MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
	m_data.master_pid = strtoul(row[0], (char **)nullptr, 10);
	m_data.level = (uint8_t)strtoul(row[1], (char **)nullptr, 10);
	m_data.exp = strtoul(row[2], (char **)nullptr, 10);
	strlcpy(m_data.name, row[3], sizeof(m_data.name));

	m_data.skill_point = (uint8_t) strtoul(row[4], (char **) nullptr, 10);
	if (row[5])
		memcpy(m_data.abySkill, row[5], sizeof(uint8_t) * GUILD_SKILL_COUNT);
	else
		memset(m_data.abySkill, 0, sizeof(uint8_t) * GUILD_SKILL_COUNT);

	m_data.power = MAX(0, strtoul(row[6], (char **) nullptr, 10));

	str_to_number(m_data.ladder_point, row[7]);

	if (m_data.ladder_point < 0)
		m_data.ladder_point = 0;

	str_to_number(m_data.win, row[8]);
	str_to_number(m_data.draw, row[9]);
	str_to_number(m_data.loss, row[10]);
	str_to_number(m_data.gold, row[11]);

	ComputeGuildPoints();
}

void CGuild::Load(uint32_t guild_id)
{
	Initialize();

	m_data.guild_id = guild_id;

	DBManager::Instance().FuncQuery(std::bind(&CGuild::LoadGuildData, this, std::placeholders::_1),
			"SELECT master, level, exp, name, skill_point, skill, sp, ladder_point, win, draw, loss, gold FROM guild WHERE id = %u", m_data.guild_id);

	sys_log(0, "GUILD: loading guild id %12s %u", m_data.name, guild_id);

	DBManager::Instance().FuncQuery(std::bind(&CGuild::LoadGuildGradeData, this, std::placeholders::_1),
			"SELECT grade, name, auth+0 FROM guild_grade WHERE guild_id = %u", m_data.guild_id);

	DBManager::Instance().FuncQuery(std::bind(&CGuild::LoadGuildMemberData, this, std::placeholders::_1),
			"SELECT pid, grade, is_general, offer, level, job, name FROM guild_member, player WHERE guild_id = %u and pid = id", guild_id);
}

void CGuild::SaveLevel()
{
	DBManager::Instance().Query("UPDATE guild SET level=%d, exp=%u, skill_point=%d WHERE id = %u", m_data.level,m_data.exp, m_data.skill_point,m_data.guild_id);
}

void CGuild::SendDBSkillUpdate(int32_t amount)
{
	TPacketGuildSkillUpdate guild_skill;
	guild_skill.guild_id = m_data.guild_id;
	guild_skill.amount = amount;
	guild_skill.skill_point = m_data.skill_point;
	memcpy(guild_skill.skill_levels, m_data.abySkill, sizeof(uint8_t) * GUILD_SKILL_COUNT);

	db_clientdesc->DBPacket(HEADER_GD_GUILD_SKILL_UPDATE, 0, &guild_skill, sizeof(guild_skill));
}

void CGuild::SaveSkill()
{
	char text[GUILD_SKILL_COUNT * 2 + 1];

	DBManager::Instance().EscapeString(text, sizeof(text), (const char *) m_data.abySkill, sizeof(m_data.abySkill));
	DBManager::Instance().Query("UPDATE guild SET sp = %d, skill_point=%d, skill='%s' WHERE id = %u",
			m_data.power, m_data.skill_point, text, m_data.guild_id);
}

TGuildMember* CGuild::GetMember(uint32_t pid)
{
	TGuildMemberContainer::iterator it = m_member.find(pid);
	if (it==m_member.end())
		return nullptr;

	return &it->second;
}

uint32_t CGuild::GetMemberPID(const std::string& strName)
{
	for ( TGuildMemberContainer::iterator iter = m_member.begin();
			iter != m_member.end(); iter++ )
	{
		if ( iter->second.name == strName ) return iter->first;
	}

	return 0;
}

void CGuild::__P2PUpdateGrade(SQLMsg* pmsg)
{
	if (pmsg->Get()->uiNumRows)
	{
		MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
		
		int32_t grade = 0;
		const char* name = row[1];
		int32_t auth = 0;

		str_to_number(grade, row[0]);
		str_to_number(auth, row[2]);

		if (grade <= 0)
			return;

		grade--;

		// 등급 명칭이 현재와 다르다면 업데이트
		if (0 != strcmp(m_data.grade_array[grade].grade_name, name))
		{
			strlcpy(m_data.grade_array[grade].grade_name, name, sizeof(m_data.grade_array[grade].grade_name));

			SPacketGCGuild pack;
			
			pack.size = sizeof(pack);
			pack.subheader = GUILD_SUBHEADER_GC_GRADE_NAME;

			TOneGradeNamePacket pack2;

			pack.size += sizeof(pack2);
			pack2.grade = grade + 1;
			strlcpy(pack2.grade_name, name, sizeof(pack2.grade_name));

			TEMP_BUFFER buf;

			buf.write(&pack,sizeof(pack));
			buf.write(&pack2,sizeof(pack2));

			for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it!=m_memberOnline.end(); ++it)
			{
				LPDESC d = (*it)->GetDesc();

				if (d)
					d->Packet(buf.read_peek(), buf.size());
			}
		}

		if (m_data.grade_array[grade].auth_flag != auth)
		{
			m_data.grade_array[grade].auth_flag = auth;

			SPacketGCGuild pack;
			pack.size = sizeof(pack);
			pack.subheader = GUILD_SUBHEADER_GC_GRADE_AUTH;

			TOneGradeAuthPacket pack2;
			pack.size+=sizeof(pack2);
			pack2.grade = grade+1;
			pack2.auth = auth;

			TEMP_BUFFER buf;
			buf.write(&pack,sizeof(pack));
			buf.write(&pack2,sizeof(pack2));

			for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it!=m_memberOnline.end(); ++it)
			{
				LPDESC d = (*it)->GetDesc();
				if (d)
				{
					d->Packet(buf.read_peek(), buf.size());
				}
			}
		}
	}
}

void CGuild::P2PChangeGrade(uint8_t grade)
{
	DBManager::Instance().FuncQuery(std::bind(&CGuild::__P2PUpdateGrade, this, std::placeholders::_1),
			"SELECT grade, name, auth+0 FROM guild_grade WHERE guild_id = %u and grade = %d", m_data.guild_id, grade);
}

namespace 
{
	struct FSendChangeGrade
	{
		uint8_t grade;
		TPacketGuild p;

		FSendChangeGrade(uint32_t guild_id, uint8_t grade) : grade(grade)
		{
			p.dwGuild = guild_id;
			p.dwInfo = grade;
		}

		void operator()()
		{
			db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_GRADE, 0, &p, sizeof(p));
		}
	};
}

void CGuild::ChangeGradeName(uint8_t grade, const char* grade_name)
{
	if (grade == 1)
		return;

	if (grade < 1 || grade > 15)
	{
		sys_err("Wrong guild grade value %d", grade);
		return;
	}

	if (strlen(grade_name) > GUILD_NAME_MAX_LEN)
		return;

	if (!*grade_name)
		return;

	char text[GUILD_NAME_MAX_LEN * 2 + 1];

	DBManager::Instance().EscapeString(text, sizeof(text), grade_name, strlen(grade_name));
	DBManager::Instance().FuncAfterQuery(FSendChangeGrade(GetID(), grade), "UPDATE guild_grade SET name = '%s' where guild_id = %u and grade = %d", text, m_data.guild_id, grade);

	grade--;
	strlcpy(m_data.grade_array[grade].grade_name, grade_name, sizeof(m_data.grade_array[grade].grade_name));

	SPacketGCGuild pack;
	pack.size = sizeof(pack);
	pack.subheader = GUILD_SUBHEADER_GC_GRADE_NAME;

	TOneGradeNamePacket pack2;
	pack.size+=sizeof(pack2);
	pack2.grade = grade+1;
	strlcpy(pack2.grade_name,grade_name, sizeof(pack2.grade_name));

	TEMP_BUFFER buf;
	buf.write(&pack,sizeof(pack));
	buf.write(&pack2,sizeof(pack2));

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it!=m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();

		if (d)
			d->Packet(buf.read_peek(), buf.size());
	}
}

void CGuild::ChangeGradeAuth(uint8_t grade, uint8_t auth)
{
	if (grade == 1)
		return;

	if (grade < 1 || grade > 15)
	{
		sys_err("Wrong guild grade value %d", grade);
		return;
	}

	DBManager::Instance().FuncAfterQuery(FSendChangeGrade(GetID(),grade), "UPDATE guild_grade SET auth = %d where guild_id = %u and grade = %d", auth, m_data.guild_id, grade);

	grade--;

	m_data.grade_array[grade].auth_flag=auth;

	SPacketGCGuild pack;
	pack.size = sizeof(pack);
	pack.subheader = GUILD_SUBHEADER_GC_GRADE_AUTH;

	TOneGradeAuthPacket pack2;
	pack.size += sizeof(pack2);
	pack2.grade = grade + 1;
	pack2.auth = auth;

	TEMP_BUFFER buf;
	buf.write(&pack, sizeof(pack));
	buf.write(&pack2, sizeof(pack2));

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();

		if (d)
			d->Packet(buf.read_peek(), buf.size());
	}
}

void CGuild::SendGuildInfoPacket(LPCHARACTER ch)
{
	LPDESC d = ch->GetDesc();

	if (!d)
		return;

	SPacketGCGuild pack;
	pack.size = sizeof(SPacketGCGuild) + sizeof(TPacketGCGuildInfo);
	pack.subheader = GUILD_SUBHEADER_GC_INFO;

	TPacketGCGuildInfo pack_sub;

	memset(&pack_sub, 0, sizeof(TPacketGCGuildInfo));
	pack_sub.member_count = GetMemberCount(); 
	pack_sub.max_member_count = GetMaxMemberCount();
	pack_sub.guild_id = m_data.guild_id;
	pack_sub.master_pid = m_data.master_pid;
	pack_sub.exp	= m_data.exp;
	pack_sub.level	= m_data.level;
	strlcpy(pack_sub.name, m_data.name, sizeof(pack_sub.name));
	pack_sub.gold	= m_data.gold;
	pack_sub.hasLand	= HasLand();

	sys_log(0, "GMC guild_name %s", m_data.name);
	sys_log(0, "GMC master %d", m_data.master_pid);

	d->BufferedPacket(&pack, sizeof(SPacketGCGuild));
	d->Packet(&pack_sub, sizeof(TPacketGCGuildInfo));
}

bool CGuild::OfferExp(LPCHARACTER ch, int32_t amount)
{
	TGuildMemberContainer::iterator cit = m_member.find(ch->GetPlayerID());

	if (cit == m_member.end())
		return false;

	if (m_data.exp+amount < m_data.exp)
		return false;

	if (amount < 0)
		return false;

	if (ch->IsExpBlocked()) 
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Anti Exp Aktif"));
		return false;
	}

	if (ch->GetExp() < (uint32_t) amount)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 제공하고자 하는 경험치가 남은 경험치보다 많습니다."));
		return false;
	}

	if (ch->GetExp() - (uint32_t) amount > ch->GetExp())
	{
		sys_err("Wrong guild offer amount %d by %s[%u]", amount, ch->GetName(), ch->GetPlayerID());
		return false;
	}

	ch->PointChange(POINT_EXP, -amount);

	TPacketGuildExpUpdate guild_exp;
	guild_exp.guild_id = GetID();
	guild_exp.amount = amount / 100;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_EXP_UPDATE, 0, &guild_exp, sizeof(guild_exp));
	GuildPointChange(POINT_EXP, amount / 100, true);

	cit->second.offer_exp += amount / 100;
	cit->second._dummy = 0;

	SPacketGCGuild pack;

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();
		if (d)
		{
			pack.subheader = GUILD_SUBHEADER_GC_LIST;
			pack.size = sizeof(pack) + 13;
			d->BufferedPacket(&pack, sizeof(pack));
			d->Packet(&(cit->second), sizeof(uint32_t) * 3 + 1);
		}
	}

	SaveMember(ch->GetPlayerID());

	TPacketGuildChangeMemberData gd_guild;

	gd_guild.guild_id = GetID();
	gd_guild.pid = ch->GetPlayerID();
	gd_guild.offer = cit->second.offer_exp;
	gd_guild.level = ch->GetLevel();
	gd_guild.grade = cit->second.grade;

	db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_MEMBER_DATA, 0, &gd_guild, sizeof(gd_guild));
	return true;
}

void CGuild::Disband()
{
	sys_log(0, "GUILD: Disband %s:%u", GetName(), GetID());

	//building::CLand* pLand = building::CManager::Instance().FindLandByGuild(GetID());
	//if (pLand)
	//pLand->SetOwner(0);

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPCHARACTER ch = *it;
		ch->SetGuild(nullptr);
		SendOnlineRemoveOnePacket(ch->GetPlayerID());

		ch->SetQuestFlag("guild_manage.new_disband_time", get_global_time());
	}

	for (TGuildMemberContainer::iterator it = m_member.begin(); it != m_member.end(); ++it)
	{
		CGuildManager::Instance().Unlink(it->first);
	}

}

void CGuild::RequestDisband(uint32_t pid)
{
	if (m_data.master_pid != pid)
		return;

	TPacketGuild gd_guild;
	gd_guild.dwGuild = GetID();
	gd_guild.dwInfo = 0;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_DISBAND, 0, &gd_guild, sizeof(TPacketGuild));

	// LAND_CLEAR
	building::CManager::Instance().ClearLandByGuildID(GetID());
	// END_LAND_CLEAR
}

void CGuild::AddComment(LPCHARACTER ch, const std::string& str)
{
    if (str.length() > GUILD_COMMENT_MAX_LEN || str.length() == 0) // Added string null verification
		return;

    if (m_guildPostCommentPulse > thecore_pulse()) {
        int32_t deltaInSeconds = ((m_guildPostCommentPulse / PASSES_PER_SEC(1)) - (thecore_pulse() / PASSES_PER_SEC(1)));
        int32_t minutes = deltaInSeconds / 60;
        int32_t seconds = (deltaInSeconds - (minutes * 60));
        ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can post a new comment in: %02d minutes and %02d seconds!"), minutes, seconds);
        return;
    }
	char text[GUILD_COMMENT_MAX_LEN * 2 + 1];
	DBManager::Instance().EscapeString(text, sizeof(text), str.c_str(), str.length());

	DBManager::Instance().FuncAfterQuery(std::bind(&CGuild::RefreshCommentForce, this, ch->GetPlayerID()),
			"INSERT INTO guild_comment(guild_id, name, notice, content, time) VALUES(%u, '%s', %d, '%s', NOW())",
			m_data.guild_id, ch->GetName(), (str[0] == '!') ? 1 : 0, text);
    m_guildPostCommentPulse = thecore_pulse() + PASSES_PER_SEC(10*60);
}

void CGuild::DeleteComment(LPCHARACTER ch, uint32_t comment_id)
{
	SQLMsg * pmsg;

	if (GetMember(ch->GetPlayerID())->grade == GUILD_LEADER_GRADE)
		pmsg = DBManager::Instance().DirectQuery("DELETE FROM guild_comment WHERE id = %u AND guild_id = %u", comment_id, m_data.guild_id);
	else
		pmsg = DBManager::Instance().DirectQuery("DELETE FROM guild_comment WHERE id = %u AND guild_id = %u AND name = '%s'", comment_id, m_data.guild_id, ch->GetName());

	if (pmsg->Get()->uiAffectedRows == 0 || pmsg->Get()->uiAffectedRows == (uint32_t)-1)
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 삭제할 수 없는 글입니다."));
	else
		RefreshCommentForce(ch->GetPlayerID());

	M2_DELETE(pmsg);
}

void CGuild::RefreshComment(LPCHARACTER ch)
{
	RefreshCommentForce(ch->GetPlayerID());
}

void CGuild::RefreshCommentForce(uint32_t player_id)
{
	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(player_id);
	if (ch == nullptr) {
		return;
	}

	std::unique_ptr<SQLMsg> pmsg (DBManager::Instance().DirectQuery("SELECT id, name, content FROM guild_comment WHERE guild_id = %u ORDER BY notice DESC, id DESC LIMIT %d", m_data.guild_id, GUILD_COMMENT_MAX_COUNT));

	SPacketGCGuild pack;
	pack.size = sizeof(pack)+1;
	pack.subheader = GUILD_SUBHEADER_GC_COMMENTS;

	uint8_t count = pmsg->Get()->uiNumRows;

	LPDESC d = ch->GetDesc();

	if (!d) 
		return;

	pack.size += (sizeof(uint32_t)+CHARACTER_NAME_MAX_LEN+1+GUILD_COMMENT_MAX_LEN+1)*(uint16_t)count;
	d->BufferedPacket(&pack,sizeof(pack));
	d->BufferedPacket(&count, 1);
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	char szContent[GUILD_COMMENT_MAX_LEN + 1];
	memset(szName, 0, sizeof(szName));
	memset(szContent, 0, sizeof(szContent));

	for (size_t i = 0; i < pmsg->Get()->uiNumRows; i++)
	{
		MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
		uint32_t id = strtoul(row[0], nullptr, 10);

		strlcpy(szName, row[1], sizeof(szName));
		strlcpy(szContent, row[2], sizeof(szContent));

		d->BufferedPacket(&id, sizeof(id));
		d->BufferedPacket(szName, sizeof(szName));

		if (i == pmsg->Get()->uiNumRows - 1)
			d->Packet(szContent, sizeof(szContent)); // 마지막 줄이면 보내기
		else
			d->BufferedPacket(szContent, sizeof(szContent));
	}
}

bool CGuild::ChangeMemberGeneral(uint32_t pid, uint8_t is_general)
{
	if (is_general && GetGeneralCount() >= GetMaxGeneralCount())
		return false;

	TGuildMemberContainer::iterator it = m_member.find(pid);
	if (it == m_member.end())
	{
		return true;
	}

	is_general = is_general?1:0;

	if (it->second.is_general == is_general)
		return true;

	if (is_general)
		++m_general_count;
	else
		--m_general_count;

	it->second.is_general = is_general;

	TGuildMemberOnlineContainer::iterator itOnline = m_memberOnline.begin();

	SPacketGCGuild pack;
	pack.size = sizeof(pack)+5;
	pack.subheader = GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL;

	while (itOnline != m_memberOnline.end())
	{
		LPDESC d = (*(itOnline++))->GetDesc();

		if (!d)
			continue;

		d->BufferedPacket(&pack, sizeof(pack));
		d->BufferedPacket(&pid, sizeof(pid));
		d->Packet(&is_general, sizeof(is_general));
	}

	SaveMember(pid);
	return true;
}

void CGuild::ChangeMemberGrade(uint32_t pid, uint8_t grade)
{
	if (grade == 1)
		return;

	TGuildMemberContainer::iterator it = m_member.find(pid);

	if (it == m_member.end())
		return;

	it->second.grade = grade;

	TGuildMemberOnlineContainer::iterator itOnline = m_memberOnline.begin();

	SPacketGCGuild pack;
	pack.size = sizeof(pack)+5;
	pack.subheader = GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE;

	while (itOnline != m_memberOnline.end())
	{
		LPDESC d = (*(itOnline++))->GetDesc();

		if (!d)
			continue;

		d->BufferedPacket(&pack, sizeof(pack));
		d->BufferedPacket(&pid, sizeof(pid));
		d->Packet(&grade, sizeof(grade));
	}

	SaveMember(pid);

	TPacketGuildChangeMemberData gd_guild;

	gd_guild.guild_id = GetID();
	gd_guild.pid = pid;
	gd_guild.offer = it->second.offer_exp;
	gd_guild.level = it->second.level;
	gd_guild.grade = grade;

	db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_MEMBER_DATA, 0, &gd_guild, sizeof(gd_guild));
}

void CGuild::SkillLevelUp(uint32_t dwVnum)
{
	uint32_t dwRealVnum = dwVnum - GUILD_SKILL_START;

	if (dwRealVnum >= GUILD_SKILL_COUNT)
		return;

	CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);

	if (!pkSk)
	{
		sys_err("There is no such guild skill by number %u", dwVnum);
		return;
	}

	if (m_data.abySkill[dwRealVnum] >= pkSk->bMaxLevel)
		return;

	if (m_data.skill_point <= 0)
		return;
	m_data.skill_point --;

	m_data.abySkill[dwRealVnum] ++;

	ComputeGuildPoints();
	SaveSkill();
	SendDBSkillUpdate();

	/*switch (dwVnum)
	  {
	  case GUILD_SKILL_GAHO:
	  {
	  TGuildMemberOnlineContainer::iterator it;

	  for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	  (*it)->PointChange(POINT_DEF_GRADE, 1);
	  }
	  break;
	  case GUILD_SKILL_HIM:
	  {
	  TGuildMemberOnlineContainer::iterator it;

	  for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	  (*it)->PointChange(POINT_ATT_GRADE, 1);
	  }
	  break;
	  }*/

	std::for_each(m_memberOnline.begin(), m_memberOnline.end(), std::bind(&CGuild::SendSkillInfoPacket, this, std::placeholders::_1));

	sys_log(0, "Guild SkillUp: %s %d level %d type %u", GetName(), pkSk->dwVnum, m_data.abySkill[dwRealVnum], pkSk->dwType);
}

void CGuild::UseSkill(uint32_t dwVnum, LPCHARACTER ch, uint32_t pid)
{
	if (!GetMember(ch->GetPlayerID()) || !HasGradeAuth(GetMember(ch->GetPlayerID())->grade, GUILD_AUTH_SKILL))
		return;

	sys_log(0,"GUILD_USE_SKILL : cname(%s), skill(%d)", ch ? ch->GetName() : "", dwVnum);

	uint32_t dwRealVnum = dwVnum - GUILD_SKILL_START;

	if (!ch->CanMove())
		return;

	if (dwRealVnum >= GUILD_SKILL_COUNT)
		return;

	CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);

	if (!pkSk)
	{
		sys_err("There is no such guild skill by number %u", dwVnum);
		return;
	}

	if (m_data.abySkill[dwRealVnum] == 0)
		return;

	if ((pkSk->dwFlag & SKILL_FLAG_SELFONLY))
	{
		// 이미 걸려 있으므로 사용하지 않음.
		if (ch->FindAffect(pkSk->dwVnum))
			return;
	}

	if (ch->IsAffectFlag(AFF_REVIVE_INVISIBLE))
		ch->RemoveAffect(AFFECT_REVIVE_INVISIBLE);

	if (ch->IsAffectFlag(AFF_EUNHYUNG))
		ch->RemoveAffect(SKILL_EUNHYUNG);

	double k =1.0*m_data.abySkill[dwRealVnum]/pkSk->bMaxLevel;
	pkSk->kSPCostPoly.SetVar("k", k);
	int32_t iNeededSP = (int32_t) pkSk->kSPCostPoly.Eval();

	if (GetSP() < iNeededSP)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 용신력이 부족합니다. (%d, %d)"), GetSP(), iNeededSP);
		return;
	}

	pkSk->kCooldownPoly.SetVar("k", k);
	int32_t iCooltime = (int32_t) pkSk->kCooldownPoly.Eval();

	if (!abSkillUsable[dwRealVnum])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 쿨타임이 끝나지 않아 길드 스킬을 사용할 수 없습니다."));
		return;
	}

	{
		TPacketGuildUseSkill p;
		p.dwGuild = GetID();
		p.dwSkillVnum = pkSk->dwVnum;
		p.dwCooltime = iCooltime;
		db_clientdesc->DBPacket(HEADER_GD_GUILD_USE_SKILL, 0, &p, sizeof(p));
	}
	abSkillUsable[dwRealVnum] = false;
	//abSkillUsed[dwRealVnum] = true;
	//adwSkillNextUseTime[dwRealVnum] = get_dword_time() + iCooltime * 1000;

	//PointChange(POINT_SP, -iNeededSP);
	//GuildPointChange(POINT_SP, -iNeededSP);

	if (g_bIsTestServer)
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> %d 스킬을 사용함 (%d, %d) to %u"), dwVnum, GetSP(), iNeededSP, pid);

	/*if (ch->GetPlayerID() != GetMasterPID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드장만 길드 스킬을 사용할 수 있습니다."));
		return;
	}*/

	if (!UnderAnyWar())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드 스킬은 길드전 중에만 사용할 수 있습니다."));
		return;
	}

	SendDBSkillUpdate(-iNeededSP);

	for (auto & member : m_memberOnline)
	{
		member->RemoveAffect(dwVnum);
		ch->ComputeSkill(dwVnum, member, m_data.abySkill[dwRealVnum]);
	}
}

void CGuild::SendSkillInfoPacket(LPCHARACTER ch) const
{
	LPDESC d = ch->GetDesc();

	if (!d)
		return;

	SPacketGCGuild pack;

	pack.size		= sizeof(pack) + 6 + GUILD_SKILL_COUNT;
	pack.subheader	= GUILD_SUBHEADER_GC_SKILL_INFO;

	d->BufferedPacket(&pack, sizeof(pack));
	d->BufferedPacket(&m_data.skill_point,	1);
	d->BufferedPacket(&m_data.abySkill,		GUILD_SKILL_COUNT);
	d->BufferedPacket(&m_data.power,		2);
	d->Packet(&m_data.max_power,	2);
}

void CGuild::ComputeGuildPoints()
{
	m_data.max_power = GUILD_BASE_POWER + (m_data.level-1) * GUILD_POWER_PER_LEVEL;

	m_data.power = MINMAX(0, m_data.power, m_data.max_power);
}

int32_t CGuild::GetSkillLevel(uint32_t vnum)
{
	uint32_t dwRealVnum = vnum - GUILD_SKILL_START;

	if (dwRealVnum >= GUILD_SKILL_COUNT)
		return 0;

	return m_data.abySkill[dwRealVnum];
}

/*void CGuild::GuildUpdateAffect(LPCHARACTER ch)
  {
  if (GetSkillLevel(GUILD_SKILL_GAHO))
  ch->PointChange(POINT_DEF_GRADE, GetSkillLevel(GUILD_SKILL_GAHO));

  if (GetSkillLevel(GUILD_SKILL_HIM))
  ch->PointChange(POINT_ATT_GRADE, GetSkillLevel(GUILD_SKILL_HIM));
  }*/

/*void CGuild::GuildRemoveAffect(LPCHARACTER ch)
  {
  if (GetSkillLevel(GUILD_SKILL_GAHO))
  ch->PointChange(POINT_DEF_GRADE, -(int32_t) GetSkillLevel(GUILD_SKILL_GAHO));

  if (GetSkillLevel(GUILD_SKILL_HIM))
  ch->PointChange(POINT_ATT_GRADE, -(int32_t) GetSkillLevel(GUILD_SKILL_HIM));
  }*/

void CGuild::UpdateSkill(uint8_t skill_point, uint8_t* skill_levels)
{
	//int32_t iDefMoreBonus = 0;
	//int32_t iAttMoreBonus = 0;

	m_data.skill_point = skill_point;
	/*if (skill_levels[GUILD_SKILL_GAHO - GUILD_SKILL_START]!=GetSkillLevel(GUILD_SKILL_GAHO))
	  {
	  iDefMoreBonus = skill_levels[GUILD_SKILL_GAHO - GUILD_SKILL_START]-GetSkillLevel(GUILD_SKILL_GAHO);
	  }
	  if (skill_levels[GUILD_SKILL_HIM - GUILD_SKILL_START]!=GetSkillLevel(GUILD_SKILL_HIM))
	  {
	  iAttMoreBonus = skill_levels[GUILD_SKILL_HIM  - GUILD_SKILL_START]-GetSkillLevel(GUILD_SKILL_HIM);
	  }

	  if (iDefMoreBonus || iAttMoreBonus)
	  {
	  for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	  {
	  (*it)->PointChange(POINT_ATT_GRADE, iAttMoreBonus);
	  (*it)->PointChange(POINT_DEF_GRADE, iDefMoreBonus);
	  }
	  }*/

	memcpy(m_data.abySkill, skill_levels, sizeof(uint8_t) * GUILD_SKILL_COUNT);
	ComputeGuildPoints();
}

static uint32_t __guild_levelup_exp(int32_t level)
{
	return guild_exp_table2[level];
}

void CGuild::GuildPointChange(uint8_t type, int32_t amount, bool save)
{
	switch (type)
	{
		case POINT_SP:
			m_data.power += amount;

			m_data.power = MINMAX(0, m_data.power, m_data.max_power);

			if (save)
			{
				SaveSkill();
			}

			std::for_each(m_memberOnline.begin(), m_memberOnline.end(), std::bind(&CGuild::SendSkillInfoPacket, this, std::placeholders::_1));
			break;

		case POINT_EXP:
			if (amount < 0 && m_data.exp < (uint32_t) - amount)
			{
				m_data.exp = 0;
			}
			else
			{
				m_data.exp += amount;

				while (m_data.exp >= __guild_levelup_exp(m_data.level))
				{

					if (m_data.level < GUILD_MAX_LEVEL)
					{
						m_data.exp -= __guild_levelup_exp(m_data.level);
						++m_data.level;
						++m_data.skill_point;

						if (m_data.level > GUILD_MAX_LEVEL)
							m_data.level = GUILD_MAX_LEVEL;

						ComputeGuildPoints();
						GuildPointChange(POINT_SP, m_data.max_power-m_data.power);

						if (save)
							ChangeLadderPoint(GUILD_LADDER_POINT_PER_LEVEL);

						// NOTIFY_GUILD_EXP_CHANGE
						std::for_each(m_memberOnline.begin(), m_memberOnline.end(), std::bind(&CGuild::SendGuildInfoPacket, this, std::placeholders::_1));
						// END_OF_NOTIFY_GUILD_EXP_CHANGE
					}

					if (m_data.level == GUILD_MAX_LEVEL)
					{
						m_data.exp = 0;
					}
				}
			}

			SPacketGCGuild pack;
			pack.size = sizeof(pack)+5;
			pack.subheader = GUILD_SUBHEADER_GC_CHANGE_EXP;

			TEMP_BUFFER buf;
			buf.write(&pack,sizeof(pack));
			buf.write(&m_data.level,1);
			buf.write(&m_data.exp,4);

			for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
			{
				LPDESC d = (*it)->GetDesc();

				if (d)
					d->Packet(buf.read_peek(), buf.size());
			}

			if (save)
				SaveLevel();

			break;
	}
}

void CGuild::SkillRecharge()
{
	//GuildPointChange(POINT_SP, m_data.max_power / 2);
	//GuildPointChange(POINT_SP, 10);
}

void CGuild::SaveMember(uint32_t pid)
{
	TGuildMemberContainer::iterator it = m_member.find(pid);

	if (it == m_member.end())
		return;

	DBManager::Instance().Query(
			"UPDATE guild_member SET grade = %d, offer = %u, is_general = %d WHERE pid = %u and guild_id = %u",
			it->second.grade, it->second.offer_exp, it->second.is_general, pid, m_data.guild_id);
}

void CGuild::LevelChange(uint32_t pid, uint8_t level)
{
	TGuildMemberContainer::iterator cit = m_member.find(pid);

	if (cit == m_member.end())
		return;

	cit->second.level = level;

	TPacketGuildChangeMemberData gd_guild;

	gd_guild.guild_id = GetID();
	gd_guild.pid = pid;
	gd_guild.offer = cit->second.offer_exp;
	gd_guild.grade = cit->second.grade;
	gd_guild.level = level;

	db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_MEMBER_DATA, 0, &gd_guild, sizeof(gd_guild));

	SPacketGCGuild pack;
	cit->second._dummy = 0;

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();

		if (d)
		{
			pack.subheader = GUILD_SUBHEADER_GC_LIST;
			pack.size = sizeof(pack) + 13;
			d->BufferedPacket(&pack, sizeof(pack));
			d->Packet(&(cit->second), sizeof(uint32_t) * 3 + 1);
		}
	}
}

void CGuild::ChangeMemberData(uint32_t pid, uint32_t offer, uint8_t level, uint8_t grade)
{
	TGuildMemberContainer::iterator cit = m_member.find(pid);

	if (cit == m_member.end())
		return;

	cit->second.offer_exp = offer;
	cit->second.level = level;
	cit->second.grade = grade;
	cit->second._dummy = 0;

	SPacketGCGuild pack;

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();
		if (d)
		{
			pack.subheader = GUILD_SUBHEADER_GC_LIST;
			pack.size = sizeof(pack) + 13;
			d->BufferedPacket(&pack, sizeof(pack));
			d->Packet(&(cit->second), sizeof(uint32_t) * 3 + 1);
		}
	}
}

namespace
{
	struct FGuildChat
	{
		const char* c_pszText;

		FGuildChat(const char* c_pszText)
			: c_pszText(c_pszText)
			{}

		void operator()(LPCHARACTER ch)
		{
			ch->ChatPacket(CHAT_TYPE_GUILD, "%s", c_pszText);
		}
	};
}

void CGuild::P2PChat(const char* c_pszText)
{
	std::for_each(m_memberOnline.begin(), m_memberOnline.end(), FGuildChat(c_pszText));
}

void CGuild::Chat(const char* c_pszText)
{
	std::for_each(m_memberOnline.begin(), m_memberOnline.end(), FGuildChat(c_pszText));

	TPacketGGGuild p1;
	TPacketGGGuildChat p2;

	p1.bHeader = HEADER_GG_GUILD;
	p1.bSubHeader = GUILD_SUBHEADER_GG_CHAT;
	p1.dwGuild = GetID();
	strlcpy(p2.szText, c_pszText, sizeof(p2.szText));

	P2P_MANAGER::Instance().Send(&p1, sizeof(TPacketGGGuild));
	P2P_MANAGER::Instance().Send(&p2, sizeof(TPacketGGGuildChat));
}

LPCHARACTER CGuild::GetMasterCharacter()
{ 
	return CHARACTER_MANAGER::Instance().FindByPID(GetMasterPID()); 
}

void CGuild::Packet(const void* buf, int32_t size)
{
	for (auto it = m_memberOnline.begin(); it!=m_memberOnline.end();++it)
	{
		LPDESC d = (*it)->GetDesc();

		if (d)
			d->Packet(buf, size);
	}
}

int32_t CGuild::GetTotalLevel() const
{
	int32_t total = 0;

	for (auto it = m_member.begin(); it != m_member.end(); ++it)
	{
		total += it->second.level;
	}

	return total;
}

bool CGuild::ChargeSP(LPCHARACTER ch, int32_t iSP)
{
	int32_t gold = iSP * 100;

	if (gold < iSP || ch->GetGold() < gold)
		return false;

	int32_t iRemainSP = m_data.max_power - m_data.power;

	if (iSP > iRemainSP)
	{
		iSP = iRemainSP;
		gold = iSP * 100;
	}

	ch->PointChange(POINT_GOLD, -gold);
	LogManager::Instance().MoneyLog(MONEY_LOG_GUILD, 1, -gold);

	SendDBSkillUpdate(iSP);
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> %u의 용신력을 회복하였습니다."), iSP);
	}
	return true;
}

void CGuild::SkillUsableChange(uint32_t dwSkillVnum, bool bUsable)
{
	uint32_t dwRealVnum = dwSkillVnum - GUILD_SKILL_START;

	if (dwRealVnum >= GUILD_SKILL_COUNT)
		return; 

	abSkillUsable[dwRealVnum] = bUsable;

	// GUILD_SKILL_COOLTIME_BUG_FIX
	sys_log(0, "CGuild::SkillUsableChange(guild=%s, skill=%d, usable=%d)", GetName(), dwSkillVnum, bUsable);
	// END_OF_GUILD_SKILL_COOLTIME_BUG_FIX
}

// GUILD_MEMBER_COUNT_BONUS
void CGuild::SetMemberCountBonus(int32_t iBonus)
{
	m_iMemberCountBonus = iBonus;
	sys_log(0, "GUILD_IS_FULL_BUG : Bonus set to %d(val:%d)", iBonus, m_iMemberCountBonus);
}

void CGuild::BroadcastMemberCountBonus()
{
	TPacketGGGuild p1;

	p1.bHeader = HEADER_GG_GUILD;
	p1.bSubHeader = GUILD_SUBHEADER_GG_SET_MEMBER_COUNT_BONUS;
	p1.dwGuild = GetID();

	P2P_MANAGER::Instance().Send(&p1, sizeof(TPacketGGGuild));
	P2P_MANAGER::Instance().Send(&m_iMemberCountBonus, sizeof(int32_t));
}

int32_t CGuild::GetMaxMemberCount()
{
	// GUILD_IS_FULL_BUG_FIX
	if ( m_iMemberCountBonus < 0 || m_iMemberCountBonus > 18 )
		m_iMemberCountBonus = 0;
	// END_GUILD_IS_FULL_BUG_FIX

	if (g_bGuildInfiniteMembers)
		return INT_MAX;

	return 32 + 2 * (m_data.level-1) + m_iMemberCountBonus;
}
// END_OF_GUILD_MEMBER_COUNT_BONUS

void CGuild::AdvanceLevel(int32_t iLevel)
{
	if (m_data.level == iLevel)
		return;

	m_data.level = MIN(GUILD_MAX_LEVEL, iLevel);
}

void CGuild::RequestDepositMoney(LPCHARACTER ch, int32_t iGold)
{
	if (false==ch->CanDeposit())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 잠시후에 이용해주십시오"));
		return;
	}

	if (ch->GetGold() < iGold)
		return;


	ch->PointChange(POINT_GOLD, -iGold);

	TPacketGDGuildMoney p;
	p.dwGuild = GetID();
	p.iGold = iGold;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_DEPOSIT_MONEY, 0, &p, sizeof(p));

	char buf[64+1];
	snprintf(buf, sizeof(buf), "%u %s", GetID(), GetName());
	LogManager::Instance().CharLog(ch, iGold, "GUILD_DEPOSIT", buf);

	ch->UpdateDepositPulse();
	sys_log(0, "GUILD: DEPOSIT %s:%u player %s[%u] gold %d", GetName(), GetID(), ch->GetName(), ch->GetPlayerID(), iGold);
}

void CGuild::RequestWithdrawMoney(LPCHARACTER ch, int32_t iGold)
{
	if (false==ch->CanDeposit())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 잠시후에 이용해주십시오"));
		return;
	}

	if (ch->GetPlayerID() != GetMasterPID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드 금고에선 길드장만 출금할 수 있습니다."));
		return;
	}

	if (m_data.gold < iGold)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 가지고 있는 돈이 부족합니다."));
		return;
	}

	TPacketGDGuildMoney p;
	p.dwGuild = GetID();
	p.iGold = iGold;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_WITHDRAW_MONEY, 0, &p, sizeof(p));

	ch->UpdateDepositPulse();
}

void CGuild::RecvMoneyChange(int32_t iGold)
{
	m_data.gold = iGold;

	SPacketGCGuild p;
	p.size = sizeof(p) + sizeof(int32_t);
	p.subheader = GUILD_SUBHEADER_GC_MONEY_CHANGE;

	for (auto it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPCHARACTER ch = *it;
		LPDESC d = ch->GetDesc();
		d->BufferedPacket(&p, sizeof(p));
		d->Packet(&iGold, sizeof(int32_t));
	}
}

void CGuild::RecvWithdrawMoneyGive(int32_t iChangeGold)
{
	LPCHARACTER ch = GetMasterCharacter();

	if (ch)
	{
		ch->PointChange(POINT_GOLD, iChangeGold);
		sys_log(0, "GUILD: WITHDRAW %s:%u player %s[%u] gold %d", GetName(), GetID(), ch->GetName(), ch->GetPlayerID(), iChangeGold);
	}

	TPacketGDGuildMoneyWithdrawGiveReply p;
	p.dwGuild = GetID();
	p.iChangeGold = iChangeGold;
	p.bGiveSuccess = ch ? 1 : 0;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_WITHDRAW_MONEY_GIVE_REPLY, 0, &p, sizeof(p));
}

bool CGuild::HasLand()
{
	return building::CManager::Instance().FindLandByGuild(GetID()) != nullptr;
}

// GUILD_JOIN_BUG_FIX
/// 길드 초대 event 정보
EVENTINFO(TInviteGuildEventInfo)
{
	uint32_t	dwInviteePID;		///< 초대받은 character 의 PID
	uint32_t	dwGuildID;		///< 초대한 Guild 의 ID

	TInviteGuildEventInfo()
	: dwInviteePID( 0 )
	, dwGuildID( 0 )
	{
	}
};

/**
 * 길드 초대 event callback 함수.
 * event 가 발동하면 초대 거절로 처리한다.
 */
EVENTFUNC( GuildInviteEvent )
{
	TInviteGuildEventInfo *pInfo = dynamic_cast<TInviteGuildEventInfo*>( event->info );

	if ( pInfo == nullptr )
	{
		sys_err( "GuildInviteEvent> <Factor> Null pointer" );
		return 0;
	}

	CGuild* pGuild = CGuildManager::Instance().FindGuild( pInfo->dwGuildID );

	if ( pGuild ) 
	{
		sys_log( 0, "GuildInviteEvent %s", pGuild->GetName() );
		pGuild->InviteDeny( pInfo->dwInviteePID );
	}

	return 0;
}

void CGuild::Invite( LPCHARACTER pchInviter, LPCHARACTER pchInvitee )
{
	if (quest::CQuestManager::Instance().GetPCForce(pchInviter->GetPlayerID())->IsRunning() == true)
	{
	    pchInviter->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방이 초대 신청을 받을 수 없는 상태입니다."));
	    return;
	}

	
	if (quest::CQuestManager::Instance().GetPCForce(pchInvitee->GetPlayerID())->IsRunning() == true)
		return;

	if ( pchInvitee->IsBlockMode( BLOCK_GUILD_INVITE ) ) 
	{
		pchInviter->ChatPacket( CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방이 길드 초대 거부 상태입니다.") );
		return;
	} 
	else if ( !HasGradeAuth( GetMember( pchInviter->GetPlayerID() )->grade, GUILD_AUTH_ADD_MEMBER ) ) 
	{
		pchInviter->ChatPacket( CHAT_TYPE_INFO, LC_TEXT("<길드> 길드원을 초대할 권한이 없습니다.") );
		return;
	} 
	else if ( pchInvitee->GetEmpire() != pchInviter->GetEmpire() ) 
	{
		pchInviter->ChatPacket( CHAT_TYPE_INFO, LC_TEXT("<길드> 다른 제국 사람을 길드에 초대할 수 없습니다.") );
		return;
	}
	else if (pchInvitee->GetLevel() < 5)
	{
		pchInviter->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Oyuncu yeterli seviyede degil"));
		return;
	}

	GuildJoinErrCode errcode = VerifyGuildJoinableCondition( pchInvitee );
	switch ( errcode ) 
	{
		case GERR_NONE: break;
		case GERR_WITHDRAWPENALTY:
						pchInviter->ChatPacket( CHAT_TYPE_INFO, 
								LC_TEXT("<길드> 탈퇴한 후 %d일이 지나지 않은 사람은 길드에 초대할 수 없습니다."), 
								quest::CQuestManager::Instance().GetEventFlag( "guild_withdraw_delay" ) );
						return;
		case GERR_COMMISSIONPENALTY:
						pchInviter->ChatPacket( CHAT_TYPE_INFO, 
								LC_TEXT("<길드> 길드를 해산한 지 %d일이 지나지 않은 사람은 길드에 초대할 수 없습니다."), 
								quest::CQuestManager::Instance().GetEventFlag( "guild_disband_delay") );
						return;
		case GERR_ALREADYJOIN:	pchInviter->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방이 이미 다른 길드에 속해있습니다.")); return;
		case GERR_GUILDISFULL:	pchInviter->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 최대 길드원 수를 초과했습니다.")); return;
		case GERR_GUILD_IS_IN_WAR : pchInviter->ChatPacket( CHAT_TYPE_INFO, LC_TEXT("<길드> 현재 길드가 전쟁 중 입니다.") ); return;
		case GERR_INVITE_LIMIT : pchInviter->ChatPacket( CHAT_TYPE_INFO, LC_TEXT("<길드> 현재 신규 가입 제한 상태 입니다.") ); return;

		default: sys_err( "ignore guild join error(%d)", errcode ); return;
	}

	if ( m_GuildInviteEventMap.end() != m_GuildInviteEventMap.find( pchInvitee->GetPlayerID() ) )
		return;

	//
	// 이벤트 생성
	// 
	TInviteGuildEventInfo* pInfo = AllocEventInfo<TInviteGuildEventInfo>();
	pInfo->dwInviteePID = pchInvitee->GetPlayerID();
	pInfo->dwGuildID = GetID();

	m_GuildInviteEventMap.insert(EventMap::value_type(pchInvitee->GetPlayerID(), event_create(GuildInviteEvent, pInfo, PASSES_PER_SEC(10))));

	//
	// 초대 받는 character 에게 초대 패킷 전송
	// 

	uint32_t gid = GetID();

	SPacketGCGuild p;
	p.size	= sizeof(p) + sizeof(uint32_t) + GUILD_NAME_MAX_LEN + 1;
	p.subheader	= GUILD_SUBHEADER_GC_GUILD_INVITE;

	TEMP_BUFFER buf;
	buf.write( &p, sizeof(p) );
	buf.write( &gid, sizeof(uint32_t) );
	buf.write( GetName(), GUILD_NAME_MAX_LEN + 1 );

	pchInvitee->GetDesc()->Packet( buf.read_peek(), buf.size() );
}

void CGuild::InviteAccept( LPCHARACTER pchInvitee )
{
	EventMap::iterator itFind = m_GuildInviteEventMap.find( pchInvitee->GetPlayerID() );
	if ( itFind == m_GuildInviteEventMap.end() ) 
	{
		sys_log( 0, "GuildInviteAccept from not invited character(invite guild: %s, invitee: %s)", GetName(), pchInvitee->GetName() );
		return;
	}

	event_cancel( &itFind->second );
	m_GuildInviteEventMap.erase( itFind );

	GuildJoinErrCode errcode = VerifyGuildJoinableCondition( pchInvitee );
	switch ( errcode ) 
	{
		case GERR_NONE: break;
		case GERR_WITHDRAWPENALTY:
						pchInvitee->ChatPacket( CHAT_TYPE_INFO, 
								LC_TEXT("<길드> 탈퇴한 후 %d일이 지나지 않은 사람은 길드에 초대할 수 없습니다."), 
								quest::CQuestManager::Instance().GetEventFlag( "guild_withdraw_delay" ) );
						return;
		case GERR_COMMISSIONPENALTY:
						pchInvitee->ChatPacket( CHAT_TYPE_INFO, 
								LC_TEXT("<길드> 길드를 해산한 지 %d일이 지나지 않은 사람은 길드에 초대할 수 없습니다."), 
								quest::CQuestManager::Instance().GetEventFlag( "guild_disband_delay") );
						return;
		case GERR_ALREADYJOIN:	pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방이 이미 다른 길드에 속해있습니다.")); return;
		case GERR_GUILDISFULL:	pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 최대 길드원 수를 초과했습니다.")); return;
		case GERR_GUILD_IS_IN_WAR : pchInvitee->ChatPacket( CHAT_TYPE_INFO, LC_TEXT("<길드> 현재 길드가 전쟁 중 입니다.") ); return;
		case GERR_INVITE_LIMIT : pchInvitee->ChatPacket( CHAT_TYPE_INFO, LC_TEXT("<길드> 현재 신규 가입 제한 상태 입니다.") ); return;

		default: sys_err( "ignore guild join error(%d)", errcode ); return;
	}

	RequestAddMember( pchInvitee, 15 );
}

void CGuild::InviteDeny( uint32_t dwPID )
{
	EventMap::iterator itFind = m_GuildInviteEventMap.find( dwPID );
	if ( itFind == m_GuildInviteEventMap.end() ) 
	{
		sys_log( 0, "GuildInviteDeny from not invited character(invite guild: %s, invitee PID: %d)", GetName(), dwPID );
		return;
	}

	event_cancel( &itFind->second );
	m_GuildInviteEventMap.erase( itFind );
}

CGuild::GuildJoinErrCode CGuild::VerifyGuildJoinableCondition( const LPCHARACTER pchInvitee )
{
	if ( get_global_time() - pchInvitee->GetQuestFlag( "guild_manage.new_withdraw_time" )
			< CGuildManager::Instance().GetWithdrawDelay() )
		return GERR_WITHDRAWPENALTY;
	else if ( get_global_time() - pchInvitee->GetQuestFlag( "guild_manage.new_disband_time" )
			< CGuildManager::Instance().GetDisbandDelay() )
		return GERR_COMMISSIONPENALTY;
	else if ( pchInvitee->GetGuild() )
		return GERR_ALREADYJOIN;
	else if ( GetMemberCount() >= GetMaxMemberCount() )
	{
		sys_log(1, "GuildName = %s, GetMemberCount() = %d, GetMaxMemberCount() = %d (32 + MAX(level(%d)-10, 0) * 2 + bonus(%d)", 
				GetName(), GetMemberCount(), GetMaxMemberCount(), m_data.level, m_iMemberCountBonus);
		return GERR_GUILDISFULL;
	}
	else if ( UnderAnyWar() != 0 )
	{
		return GERR_GUILD_IS_IN_WAR;
	}

	return GERR_NONE;
}
// END_OF_GUILD_JOIN_BUG_FIX

bool CGuild::ChangeMasterTo(uint32_t dwPID)
{
	if ( GetMember(dwPID) == nullptr ) return false;

	TPacketChangeGuildMaster p;
	p.dwGuildID = GetID();
	p.idFrom = GetMasterPID();
	p.idTo = dwPID;

	db_clientdesc->DBPacket(HEADER_GD_REQ_CHANGE_GUILD_MASTER, 0, &p, sizeof(p));

	return true;
}

void CGuild::SendGuildDataUpdateToAllMember()
{
	TGuildMemberOnlineContainer::iterator iter = m_memberOnline.begin();

	for (; iter != m_memberOnline.end(); iter++ )
	{
		SendGuildInfoPacket(*iter);
		SendAllGradePacket(*iter);
	}
}

void CGuild::SetDungeon_for_Only_guild(LPDUNGEON pDungeon)
{
	m_pkDungeon_for_Only_guild = pDungeon;
}

LPDUNGEON CGuild::GetDungeon_for_Only_guild()
{
	return m_pkDungeon_for_Only_guild;
}


void CGuild::SetSkillLevel(uint32_t dwVnum, uint8_t level, uint8_t point)
{
	uint32_t dwRealVnum = dwVnum - GUILD_SKILL_START;

	if (dwRealVnum >= GUILD_SKILL_COUNT)
		return;

	CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);

	if (!pkSk)
	{
		sys_err("There is no such guild skill by number %u", dwVnum);
		return;
	}

	if (level > pkSk->bMaxLevel)
		return;

	if (point)
	{
		if (m_data.skill_point < point)
			return;
		m_data.skill_point -= point;
	}

	m_data.abySkill[dwRealVnum] = level;

	ComputeGuildPoints();
	SaveSkill();
	SendDBSkillUpdate();

	std::for_each(m_memberOnline.begin(), m_memberOnline.end(), std::bind(&CGuild::SendSkillInfoPacket, this, std::placeholders::_1));

	sys_log(0, "Guild SetSkillLevel: %s %d level %d type %u", GetName(), pkSk->dwVnum, m_data.abySkill[dwRealVnum], pkSk->dwType);
}

uint32_t CGuild::GetSkillPoint()
{
	return m_data.skill_point;
}

void CGuild::SetSkillPoint(uint8_t point)
{
	m_data.skill_point = point;
	SendDBSkillUpdate();
	std::for_each(m_memberOnline.begin(), m_memberOnline.end(), std::bind(&CGuild::SendSkillInfoPacket, this, std::placeholders::_1));
}



