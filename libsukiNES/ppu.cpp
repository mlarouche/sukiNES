#include "ppu.h"

namespace sukiNES
{
	static const uint32 CyclesPerScanline = 340;
	static const sint32 ScanlinePerFrame = 260;

	PPU::PPU()
	: _cycleCountPerScanline(0)
	, _currentScaline(0)
	{
	}

	PPU::~PPU()
	{
	}

	byte PPU::read(word address)
	{
		return 0;
	}

	void PPU::write(word address, byte value)
	{
	}

	void PPU::tick()
	{
		_cycleCountPerScanline++;
		if (_cycleCountPerScanline > CyclesPerScanline)
		{
			_cycleCountPerScanline = 0;
			_currentScaline++;
			if (_currentScaline > ScanlinePerFrame)
			{
				_currentScaline = -1;
			}
		}
	}
}