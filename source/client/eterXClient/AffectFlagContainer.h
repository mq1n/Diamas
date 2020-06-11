#pragma once

class CAffectFlagContainer
{
	public:
		enum
		{
			BIT_SIZE = 64,
			BYTE_SIZE = BIT_SIZE / 8 + (1 * ((BIT_SIZE & 7) ? 1 : 0))
		};

	public:
		CAffectFlagContainer();
		~CAffectFlagContainer();		

		void Clear();
		void CopyInstance(const CAffectFlagContainer& c_rkAffectContainer);		
		void Set(uint32_t uPos, bool isSet);
		bool IsSet(uint32_t uPos) const;

		void CopyData(uint32_t uPos, uint32_t uByteSize, const void* c_pvData);

		void ConvertToPosition(uint32_t* uRetX, uint32_t* uRetY) const;
		
	private:
		using Element = uint8_t;

		Element m_aElement[BYTE_SIZE];
};