#pragma once

// 2004.07.05.myevan.±Ã½ÅÅº¿µ ¸Ê¿¡ ³¢ÀÌ´Â ¹®Á¦ÇØ°á
class IBackground : public CSingleton<IBackground>
{
	public:
		IBackground() {}
		virtual ~IBackground() {}

		virtual bool IsBlock(int32_t x, int32_t y) = 0;
};