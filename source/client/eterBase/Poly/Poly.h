#ifndef __POLY_POLY_H__
#define __POLY_POLY_H__

#include "SymTable.h"

#pragma warning ( push, 3 )

#include <string>
#include <vector>
#include <list>

#pragma warning ( pop )

#define POLY_MAXVALUE	0
#define POLY_NONE		POLY_MAXVALUE
#define POLY_ROOT		POLY_MAXVALUE + 1
#define POLY_MUL		POLY_MAXVALUE + 2
#define POLY_PLU		POLY_MAXVALUE + 3
#define POLY_POW		POLY_MAXVALUE + 4
#define POLY_MIN		POLY_MAXVALUE + 5
#define POLY_DIV		POLY_MAXVALUE + 6
#define POLY_OPEN		POLY_MAXVALUE + 7
#define POLY_CLOSE		POLY_MAXVALUE + 8
#define POLY_NUM		POLY_MAXVALUE + 9
#define POLY_ID			POLY_MAXVALUE + 10
#define POLY_EOS		POLY_MAXVALUE + 11
#define POLY_COS		POLY_MAXVALUE + 12
#define POLY_SIN		POLY_MAXVALUE + 13
#define POLY_TAN		POLY_MAXVALUE + 14
#define POLY_COSEC		POLY_MAXVALUE + 15
#define POLY_CSC		POLY_COSEC
#define POLY_SEC		POLY_MAXVALUE + 16
#define POLY_COT		POLY_MAXVALUE + 17
#define POLY_PI			POLY_ID
#define POLY_EXP		POLY_ID
#define POLY_LOG		POLY_MAXVALUE + 18
#define POLY_LN			POLY_MAXVALUE + 19
#define POLY_LOG10		POLY_MAXVALUE + 20

#define POLY_ABS		POLY_MAXVALUE + 21
#define POLY_MINF		POLY_MAXVALUE + 22
#define POLY_MAXF		POLY_MAXVALUE + 23
#define POLY_IRAND		POLY_MAXVALUE + 24
#define POLY_FRAND		POLY_MAXVALUE + 25
#define POLY_MOD		POLY_MAXVALUE + 26
#define POLY_FLOOR		POLY_MAXVALUE + 27
#define POLY_CEIL		POLY_MAXVALUE + 28

#define POLY_MAXSTACK	100

class CPoly
{
public:
	enum ERandomType
	{
		RANDOM_TYPE_FREELY,
		RANDOM_TYPE_FORCE_MIN,
		RANDOM_TYPE_FORCE_MAX,
	};

public:
	CPoly();
	virtual ~CPoly();

	bool	Analyze(const char * pszStr = nullptr);
	float	Eval();
	void	SetRandom(int32_t iRandomType);
	void	SetStr(const std::string & str);
	bool	SetVar(const std::string & strName, double dVar);
	int32_t		GetVarCount();
	const char * GetVarName(uint32_t dwIndex);
	void	Clear();

    protected:
	int32_t		my_irandom(double start, double end);
	double		my_frandom(double start, double end);

	void		init();
	int32_t		insert(const std::string & s, int32_t tok);
	int32_t		find(const std::string & s);
	void		emit(int32_t t,int32_t tval);
	void		match(int32_t t); 
	void		expo(); 
	void		factor(); 
	void		term(); 
	int32_t		iToken;
	double		iNumToken;
	int32_t		iLookAhead;
	int32_t		lexan();
	int32_t		iErrorPos;
	void		error();
	void		expr();
	bool		ErrorOccur;
	uint32_t	uiLookPos;

	// NOTE: list is slight faster than vector, why?!
	std::vector<int32_t>		tokenBase;
	std::vector<double>		numBase;
	std::vector<CSymTable *>	lSymbol;
	std::vector<int32_t>		SymbolIndex;
	int32_t					STSize;
	int32_t					MathSymbolCount;
	std::string			strData;
	int32_t					m_iRandomType;
};

#endif 
