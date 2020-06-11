#include "../StdAfx.h"
#include "SymTable.h"

using namespace std;

CSymTable::CSymTable(int32_t aTok, string aStr) : dVal(0), token(aTok), strlex(aStr)
{
}

CSymTable::~CSymTable()
{
}

