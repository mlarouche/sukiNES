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
		struct OAMEntry
		{
			OAMEntry()
			: y(0)
			, tileIndex(0)
			, x(0)
			{
				attributes.raw = 0;
			}

			byte y;
			byte tileIndex;
			union
			{
				byte raw;
				RegBit<0, 2> palette;
				RegBit<2, 3> unimplemented;
				RegBit<5> priority;
				RegBit<6> flipHorizontal;
				RegBit<7> flipVertical;
			} attributes;
			byte x;
		} _sprites[64];

		byte* _rawOAM;
		byte _oamAddress;

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