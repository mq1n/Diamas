#pragma once

class CItemAddonManager : public CSingleton<CItemAddonManager>
{
	public:
		CItemAddonManager();
		virtual ~CItemAddonManager();

		void ApplyAddonTo(int32_t iAddonType, LPITEM pItem);
};
