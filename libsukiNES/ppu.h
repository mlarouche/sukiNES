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

		enum class NameTableMirroring
		{
			Horizontal,
			Vertical,
			FourScreen,
			SingleScreen,
			ChrRomMirroring
		};

		void setNametableMirroring(NameTableMirroring value)
		{
			_nametableMirroring = value;
		}

	private:
		byte _internalRead(word ppuAddress);
		void _internalWrite(word ppuAddress, byte value);

	private:
		uint32 _cycleCountPerScanline;
		sint32 _currentScaline;

		word _currentPpuAddress;
		bool _firstWrite;
		byte _readBuffer;

		byte _palette[32];

		NameTableMirroring _nametableMirroring;
		byte _nametable[SUKINES_KB(2)];
	};
}