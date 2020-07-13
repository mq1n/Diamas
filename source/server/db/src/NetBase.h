#pragma once

class CNetBase
{
public:
	CNetBase();
	virtual ~CNetBase();

protected:
	static LPFDWATCH	m_fdWatcher;
};

class CNetPoller : public CNetBase, public CSingleton<CNetPoller>
{
public:
	CNetPoller();
	virtual ~CNetPoller();

	bool	Create();
	void	Destroy();
};
