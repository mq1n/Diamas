#ifndef __MILESLIB_CSOUNDMANAGERSTREAM_H__
#define __MILESLIB_CSOUNDMANAGERSTREAM_H__

#include "SoundBase.h"
#include "SoundInstance.h"

class CSoundManagerStream : public CSoundBase
{
	public:
		enum
		{
			MUSIC_INSTANCE_MAX_NUM = 3
		};

	public:
		CSoundManagerStream();
		virtual ~CSoundManagerStream();

		bool					Initialize();
		void					Destroy();

		bool					SetInstance(uint32_t dwIndex, const char* filename);
		CSoundInstanceStream *	GetInstance(uint32_t dwIndex);

	protected:
		bool					CheckInstanceIndex(uint32_t dwIndex);

	protected:
		CSoundInstanceStream	m_Instances[MUSIC_INSTANCE_MAX_NUM];
};

#endif