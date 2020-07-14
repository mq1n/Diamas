#include "stdafx.h"
#include "nearby_scanner.h"
#include "utils.h"
#include "config.h"
#include "quest_manager.h"
#include "sectree_manager.h"
#include "char.h"
#include "char_manager.h"

/////////////////////////////////////////////////////////////////////
// Packet
/////////////////////////////////////////////////////////////////////
CNearbyScanner::CNearbyScanner() : m_iID(0)
{
}
CNearbyScanner::~CNearbyScanner() = default;

EVENTFUNC(scanner_event)
{
	ScannerInfo * info = dynamic_cast<ScannerInfo *>(event->info);
	if (!info)
	{
		sys_err( "scanner_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER pkChr = CHARACTER_MANAGER::Instance().Find(info->dwVID);
	if (!pkChr) 
		return 0;

	struct FindTarget
	{
		LPCHARACTER m_ch;
		FindTarget(LPCHARACTER ch)
		{
			m_ch = ch;
		}

		void operator() (LPENTITY ent)
		{
			if (ent && ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (CHARACTER *)ent;
				if (ch->IsPC()) 
				{
					//Let's see the distance
					if (DISTANCE_APPROX(m_ch->GetX() - ch->GetX(), m_ch->GetY() - ch->GetY()) > 1000)
						return;

					uint32_t vid = m_ch->GetVID();
					ScannerInfo * info = CNearbyScanner::Instance().GetScannerInfo(vid); //Find the running event
					if (!info)
						return; //Already finished

					info->found(info->dwVID, ch->GetPlayerID()); //Exec callback
					CNearbyScanner::Instance().EndScanner(vid); //Finish
					return;
				}
			}

			return;
		}
	};

	FindTarget f(pkChr);
	pkChr->ForEachSeen(f);

	return passes_per_sec;
}

void CNearbyScanner::BeginScanner(uint32_t dwVID, void(*found) (uint32_t, uint32_t))
{
	sys_log(0, "StartScannerCreation: vid %u", dwVID);

	LPCHARACTER pkChr = CHARACTER_MANAGER::Instance().Find(dwVID);
	if (!pkChr)
	{
		sys_err("Cannot find character ptr by VID %u", dwVID);
		return;
	}
	
	if (m_map_kListEvent.find(dwVID) != m_map_kListEvent.end())
	{
		//TODO: Support several scanners (with different actions of course).
		//Only requirement: different actions when triggered.

		sys_err("Attempted creation of duplicated sourrounding scanner for VID %u", dwVID);
		return;
	}

	ScannerInfo* newInfo = AllocEventInfo<ScannerInfo>();
	newInfo->iID = ++m_iID;
	newInfo->dwVID = dwVID;
	newInfo->found = found;

	LPEVENT event = event_create(scanner_event, newInfo, PASSES_PER_SEC(1));

	if (event)
		m_map_kListEvent[dwVID].push_back(event);
}

void CNearbyScanner::EndScanner(uint32_t dwVID)
{
	const auto it = m_map_kListEvent.find(dwVID);
	if (it == m_map_kListEvent.end())
		return;

	auto it2 = it->second.begin(); 
	while (it2 != it->second.end()) //Iterate over the list of events
	{
		LPEVENT pkEvent = *it2;

		ScannerInfo * info = dynamic_cast<ScannerInfo*>(pkEvent->info);
		if (!info)
		{
			sys_err("Null pointer when trying to end the ScannerInfo for vid ");

			++it2;
			continue;
		}

		event_cancel(&pkEvent);
		it2 = it->second.erase(it2);
	}
}

LPEVENT CNearbyScanner::GetScannerEvent(uint32_t dwVID)
{
	const auto it = m_map_kListEvent.find(dwVID);
	if (it == m_map_kListEvent.end())
		return nullptr;

	auto it2 = it->second.begin();
	while (it2 != it->second.end()) //TODO: This should take some sort of index. Looping and returning always the first is kinda pointless, and is just here to support lists
	{
		LPEVENT pkEvent = *(it2++);

		ScannerInfo * info = dynamic_cast<ScannerInfo*>(pkEvent->info);
		if (!info)
		{
			sys_err("No scanner info when fetching event for vid %u", dwVID);
			continue;
		}

		return pkEvent;
	}

	return nullptr;
}

ScannerInfo * CNearbyScanner::GetScannerInfo(uint32_t dwVID)
{
	const auto it = m_map_kListEvent.find(dwVID);
	if (it == m_map_kListEvent.end())
		return nullptr;

	auto it2 = it->second.begin();
	while (it2 != it->second.end()) //TODO: take index to not always return the first info
	{
		LPEVENT pkEvent = *(it2++);

		ScannerInfo * info = dynamic_cast<ScannerInfo*>(pkEvent->info);
		if (!info)
		{
			sys_err("No scanner info when fetching info for vid %u", dwVID);
			continue;
		}

		return info;
	}

	return nullptr;
}

void CNearbyScanner::Die(uint32_t dwVID)
{
	const auto it = m_map_kListEvent.find(dwVID);
	if (it == m_map_kListEvent.end())
		return;

	auto it2 = it->second.begin();
	while (it2 != it->second.end())
		event_cancel(&(*(it2++)));

	m_map_kListEvent.erase(it);
}

