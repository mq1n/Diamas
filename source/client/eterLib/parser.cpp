#include "StdAfx.h"
#include "parser.h"
#include "../eterBase/stl_utils.h"

using namespace script;


int32_t LocaleString_FindChar(const std::string &base, int32_t start, int32_t until, char test)
{
	until = std::min<int32_t>(until, base.size());
	for (int32_t j = start; j < until; ++j) {
		if (base[j] == test)
			return j;
	}

	return -1;
}

/*
* Advance position to the next non-whitespace (or line break) character.
*/
void LocaleString_Skip(const std::string &base, int32_t& pos)
{
	for (; pos < base.size(); ++pos) {
		if (!isspace((uint8_t)base[pos]) && base[pos] != '\n' && base[pos] != '\r')
			break;
	}
}

bool Group::GetArg(const std::string &argString, TArgList & argList)
{
    std::string szName = "";
	std::string szValue = "";
	
    bool isValue = false;

	//Read args on a string of format NAME;VALUE|NAME;VALUE
	for (size_t pos = 0, arg_len = argString.size(); pos < arg_len; ++pos)
    {
		const char c = argString[pos];
		if (!c)
			break;
		
		if (c == '|')
		{
			if (szName.empty())
			{
				TraceError("no argument name");
				return false;
			}

			// Store the arg/value pair, and start looking for a name again
			isValue = false;
			argList.push_back(TArg(rtrim(szName), rtrim(szValue)));

			szValue.clear();
			szName.clear();
		}
		else if (c == ';')
		{
			// We expect a value next
			isValue = true;
		}
		else if ((isValue || szName.size() > 0 || !isspace((uint8_t)c)) && c != '\r' && c != '\n')
		{
			if (isValue)
			{
				if (szValue.size() >= 64)
				{
					//Note: Obviously not a real overflow anymore, but will just keep this here for possible
					//backwards compatibility with other places that might use it.
					TraceError("argument value overflow: must be shorter than 64 letters");
					return false;
				}

				szValue.append(&c, 1);
			}
			else
			{
				if (szName.size() >= 32)
				{
					//Note: Look on note about value overflow.
					TraceError("argument name overflow: must be shorter than 32 letters");
					return false;
				}

				szName.append(&c, 1);
			}
		}
    }

	//Store unsaved args (e.g if we didn't reach any | in the string to store them previously)
    if (!szName.empty() && !szValue.empty()) {
		argList.push_back(TArg(rtrim(szName), rtrim(szValue)));
    }

    return true;
}


bool Group::Create(const std::string & stSource)
{
	m_cmdList.clear();

	if (stSource.empty())
	{
        TraceError("Source string has no content");
        return false;
    }

	static std::string stLetter;

	for (size_t i = 0, strLen = stSource.length(); i < strLen - 1; ++i)
	{
		TCmd cmd;
		const char cur = stSource[i];

		if (cur == '[')
		{
			int32_t advanceTo = LocaleString_FindChar(stSource, i, strLen, ']');
			if (advanceTo < 0)
			{
				TraceError(" !! PARSING ERROR - Syntax Error : %s\n", stSource.c_str());
				return false;
			}
			
			// Skip spaces following '[' and prior to the command name
			// pos will hold the position of the character
			int32_t pos = i + 1;
			LocaleString_Skip(stSource, pos);
			
			int32_t cmdArgsStart = LocaleString_FindChar(stSource, pos, advanceTo, ' ');
			if (cmdArgsStart < 0)
			{
				//There are no arguments.
				cmd.name = stSource.substr(pos, advanceTo - pos);
				cmd.argList.clear();
			}
			else
			{
				cmd.name = stSource.substr(pos, cmdArgsStart - pos);
				
				// Ignore spaces
				LocaleString_Skip(stSource, cmdArgsStart);

				if (!GetArg(stSource.substr(cmdArgsStart, advanceTo - cmdArgsStart), cmd.argList))
				{
					TraceError(" !! PARSING ERROR - Unknown Arguments : %d, %s\n", advanceTo - cmdArgsStart, stSource.c_str());
					return false;
				}
			}

			m_cmdList.push_back(cmd);

			// Continue looping after this character
			i = advanceTo;
		}
		else if (cur != '\r' && cur != '\n')
		{
			stLetter = cur;

			cmd.name = "LETTER";
			cmd.argList.push_back(TArg("value", stLetter));
			m_cmdList.push_back(cmd);
		}
	}

    return true;
}

bool Group::GetCmd(TCmd & cmd)
{
    if (m_cmdList.empty())
        return false;

    cmd = m_cmdList.front();
    m_cmdList.pop_front();
    return true;
}

bool Group::ReadCmd(TCmd & cmd)
{
    if (m_cmdList.empty())
        return false;

    cmd = m_cmdList.front();
    return true;
}

std::string & Group::GetError()
{
    return m_stError;
}

void Group::SetError(const char * c_pszError)
{
    m_stError.assign(c_pszError);
}

Group::Group()
{
}

Group::~Group()
{
}
