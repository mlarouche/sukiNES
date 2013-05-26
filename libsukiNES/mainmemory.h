#pragma once

#include "memory.h"

namespace sukiNES
{
	class MainMemory : public IMemory
	{
	public:
		MainMemory();
		~MainMemory();

		virtual byte read(word address) ;
		virtual void write(word address, byte value);

		void setPpuMemory(IMemory* memory)
		{
			_ppuMemory = memory;
		}

		void setApuMemory(IMemory* memory)
		{
			_apuMemory = memory;
		}

		void setGamepakMemory(IMemory* memory)
		{
			_gamepakMemory = memory;
		}

	private:
		byte _ram[SUKINES_KB(2)];
		byte _sram[SUKINES_KB(8)];

		IMemory* _ppuMemory;
		IMemory* _apuMemory;
		IMemory* _gamepakMemory;
	};
}
