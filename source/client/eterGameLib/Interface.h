#pragma once

// 2004.07.05.myevan.�ý�ź�� �ʿ� ���̴� �����ذ�
class IBackground : public CSingleton<IBackground>
{
	public:
		IBackground() {}
		virtual ~IBackground() {}

		virtual bool IsBlock(int32_t x, int32_t y) = 0;
};