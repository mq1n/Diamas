#pragma once

EVENTINFO(ScannerInfo)
{
	// <Factor> Removed unsafe copy of CHARACTER* here
	//LPCHARACTER pkChr;

	int32_t         iID;
	int32_t         dwVID;

	void		(*found) (uint32_t, uint32_t); //Callback!

	ScannerInfo(): 
	iID(0),
	dwVID(0),
	found(nullptr){
	}
};

class CNearbyScanner : public singleton<CNearbyScanner>
{
	public:
		CNearbyScanner();
		virtual ~CNearbyScanner();

		void BeginScanner(uint32_t dwVID, void(*found) (uint32_t, uint32_t));
		void EndScanner(uint32_t dwVID);

		void Die(uint32_t dwVID);
		ScannerInfo * GetScannerInfo(uint32_t dwVID);

		LPEVENT GetScannerEvent(uint32_t dwVID);

	protected:
		// <vid, event>
		std::map<uint32_t, std::list<LPEVENT> > m_map_kListEvent;

		//Internal id counter
		uint32_t m_iID; 
};
