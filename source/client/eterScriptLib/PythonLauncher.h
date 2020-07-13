#pragma once
#include "../../common/Singleton.h"
#include <python27/frameobject.h>

class CPythonLauncher : public CSingleton<CPythonLauncher>
{
public:
	CPythonLauncher();
	virtual ~CPythonLauncher();

	void Clear();

	bool Create(const char* c_szProgramName);
	bool RunFile(const std::string& filename, const std::string& modName) const;

protected:
	PyObject* m_poModule;
	PyObject* m_poDic;
};