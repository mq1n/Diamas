#ifndef __POLY_SYMBOL_H__
#define __POLY_SYMBOL_H__

#include "Base.h"

#define ST_UNKNOWN	0
#define ST_PLUS		11
#define ST_MINUS	12
#define ST_MULTIPLY	23
#define ST_DIVIDE	24
#define ST_CARET	35
#define ST_OPEN		06
#define ST_CLOSE	07

#define SY_PLUS		'+'
#define SY_MINUS	'-'
#define SY_MULTIPLY	'*'
#define SY_DIVIDE	'/'
#define SY_CARET	'^'
#define SY_OPEN		'('
#define SY_CLOSE	')'

class CSymbol : public CBase   
{
    private:
	int32_t	iType;

    public:
	CSymbol();
	virtual ~CSymbol();

	static int32_t	issymbol(int32_t ch);
	void		SetType(int32_t Type);
	int32_t		GetType();
	bool		Equal(const CSymbol &dif);
	bool		Less(const CSymbol &dif);
};

#endif 
