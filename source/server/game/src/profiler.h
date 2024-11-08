#pragma once
#include <unordered_map>

class CProfiler : public CSingleton<CProfiler>
{
	public:
		enum
		{
			STACK_DATA_MAX_NUM = 64,
		};

	public:
		typedef struct SProfileStackData
		{
			int32_t iCallStep;
			int32_t iStartTime;
			int32_t iEndTime;

			std::string strName;
		} TProfileStackData;

		typedef struct SProfileAccumData
		{
			int32_t iStartTime;
			int32_t iCallingCount;
			int32_t iCollapsedTime;
			int32_t iDepth;

			std::string strName;
		} TProfileAccumData;

		struct stringhash
		{
			size_t operator () (const std::string& str) const
			{
				const uint8_t * s = (const uint8_t *) str.c_str();
				const uint8_t * end = s + str.size();
				size_t h = 0;

				while (s < end)
				{
					h *= 16777619;
					h ^= (uint8_t) *(uint8_t *) (s++);
				}

				return h;

			}
		};

		typedef std::unordered_map<std::string, TProfileAccumData> TProfileAccumDataMap;

	public:
		CProfiler()
		{
			Initialize();
		}

		virtual ~CProfiler()
		{
		}

		void Initialize()
		{
			m_iAccumDepth = 0;
			m_ProfileStackDataCount = 0;
			m_iCallStep = 0;
		}

		void Clear()
		{
			TProfileAccumDataMap::iterator it = m_ProfileAccumDataMap.begin();

			for (; it != m_ProfileAccumDataMap.end(); ++it)
			{
				TProfileAccumData & rData = it->second;
				rData.iCallingCount = 0;
				rData.iCollapsedTime = 0;
			}

			Initialize();
		}

		void Push(const char * c_szName)
		{
			assert(m_ProfileStackDataCount < STACK_DATA_MAX_NUM);
			TProfileStackData & rProfileStackData = m_ProfileStackDatas[m_ProfileStackDataCount++];

			rProfileStackData.iCallStep = m_iCallStep;
			rProfileStackData.iStartTime = get_unix_ms_time();
			rProfileStackData.strName = c_szName;

			++m_iCallStep;
		}

		void Pop(const char * c_szName)
		{
			TProfileStackData * pProfileStackData;

			if (!GetProfileStackDataPointer(c_szName, &pProfileStackData))
			{
				assert(!"The name doesn't exist");
				return;
			}

			pProfileStackData->iEndTime = get_unix_ms_time();
			--m_iCallStep;
		}

		__inline void PushAccum(const char * c_szName)
		{
			TProfileAccumDataMap::iterator it = m_ProfileAccumDataMap.find(c_szName);

			if (it == m_ProfileAccumDataMap.end())
			{
				TProfileAccumData ProfileAccumData;

				ProfileAccumData.iCollapsedTime = 0;
				ProfileAccumData.iCallingCount = 0;
				ProfileAccumData.strName = c_szName;
				ProfileAccumData.iDepth = m_iAccumDepth;

				m_ProfileAccumDataMap.emplace(c_szName, ProfileAccumData);
				it = m_ProfileAccumDataMap.find(c_szName);
				m_vec_Accum.push_back(&it->second);
			}

			TProfileAccumData & rData = it->second;
			rData.iStartTime = get_unix_ms_time();
			++m_iAccumDepth;
		}

		__inline void PopAccum(const char * c_szName)
		{
			TProfileAccumDataMap::iterator it = m_ProfileAccumDataMap.find(c_szName);

			if (it == m_ProfileAccumDataMap.end())
				return;

			TProfileAccumData & rData = it->second;
			rData.iCollapsedTime += get_unix_ms_time() - rData.iStartTime;
			++rData.iCallingCount;
			--m_iAccumDepth;
		}

		void Log(const char * c_pszFileName)
		{
			auto fp = msl::file_ptr(c_pszFileName, "w");
			if (!fp)
				return;

			Print(fp.get());
		}

		void Print(FILE * fp = stderr)
		{
			for (int32_t i = 0; i < m_ProfileStackDataCount; ++i)
			{
				TProfileStackData & rProfileStackData = m_ProfileStackDatas[i];

				for (int32_t j = 0; j < rProfileStackData.iCallStep; ++j)
					fprintf(fp, "\t");

				fprintf(fp, "%-24s: %d\n", rProfileStackData.strName.c_str(), rProfileStackData.iEndTime - rProfileStackData.iStartTime);
			}

			fprintf(fp, "Name                                 Call/Load       Ratio\n");

			std::string stLine;

			for (uint32_t k = 0; k < m_vec_Accum.size(); ++k)
			{
				TProfileAccumData * p = m_vec_Accum[k];

				stLine.assign(p->iDepth * 5, ' ');
				stLine += p->strName;

				fprintf(fp, "%-30s %10d/%-10d %.2f\n", 
						stLine.c_str(),
						p->iCallingCount, p->iCollapsedTime,
						p->iCallingCount != 0 ? (float) p->iCollapsedTime / p->iCallingCount : 0.0f);
			}
		}

		void PrintOneStackData(const char * c_szName)
		{
			TProfileStackData * pProfileStackData;

			if (!GetProfileStackDataPointer(c_szName, &pProfileStackData))
				return;

			sys_log(0, "%-10s: %3d", pProfileStackData->strName.c_str(), pProfileStackData->iEndTime - pProfileStackData->iStartTime);
		}

		void PrintOneAccumData(const char * c_szName)
		{
			TProfileAccumDataMap::iterator it = m_ProfileAccumDataMap.find(c_szName);

			if (it == m_ProfileAccumDataMap.end())
				return;

			TProfileAccumData & rData = it->second;

			sys_log(0, "%-10s : [CollapsedTime : %3d] / [CallingCount : %3d]",
					rData.strName.c_str(),
					rData.iCollapsedTime,
					rData.iCallingCount);
		}

	protected:
		bool GetProfileStackDataPointer(const char * c_szName, TProfileStackData ** ppProfileStackData)
		{
			for (int32_t i = 0; i < m_ProfileStackDataCount; ++i)
			{
				if (0 == m_ProfileStackDatas[i].strName.compare(c_szName))
				{
					*ppProfileStackData = &m_ProfileStackDatas[i];

					return true;
				}
			}

			return false;
		}

	protected:
		// Profile Stack Data
		int32_t m_ProfileStackDataCount;
		TProfileStackData m_ProfileStackDatas[STACK_DATA_MAX_NUM];

		// Profile Accum Data
		TProfileAccumDataMap			m_ProfileAccumDataMap;
		std::vector<TProfileAccumData *>	m_vec_Accum;
		int32_t					m_iAccumDepth;

		int32_t					m_iCallStep;
};

template <typename T> class CProfileUnit
{
	public:
		CProfileUnit(const char * c_pszName)
		{
			m_stName = c_pszName;
			CProfiler::Instance().PushAccum(m_stName.c_str());
			m_bPushed = true;
		}

		~CProfileUnit()
		{
			if (m_bPushed)
				Pop();
		}

		void Pop()
		{
			if (m_bPushed)
			{
				CProfiler::Instance().PopAccum(m_stName.c_str());
				m_bPushed = false;
			}
		}

		std::string	m_stName;
		bool		m_bPushed;
};

#define PROF_UNIT CProfileUnit<void>
