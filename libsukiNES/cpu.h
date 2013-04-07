#pragma once

namespace sukiNES
{
	class Cpu
	{
	public:
		Cpu();
		~Cpu();

		uint32 executeOpcode();
	};
}