#pragma once

#include "memory.h"

namespace sukiNES
{
	class PPU : public IMemory
	{
	public:
		PPU();
		~PPU();

		virtual byte read(word address);
		virtual void write(word address, byte value);

		void tick();

		void forceCurrentScanline(sint32 value)
		{
			_cycleCountPerScanline = 0;
			_currentScaline = value;
		}

		uint32 cyclesCountPerScanline() const
		{
			return _cycleCountPerScanline;
		}

		sint32 currentScanline() const
		{
			return _currentScaline;
		}

	private:
		uint32 _cycleCountPerScanline;
		sint32 _currentScaline;
	};
}