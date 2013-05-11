#include "ppu.h"

namespace sukiNES
{
	static const uint32 CyclesPerScanline = 340;
	static const sint32 ScanlinePerFrame = 260;
	static const uint32 PpuMirroringMask = 0x4000 - 1;
	static const byte PpuRegisterMask = 0x7;

	PPU::PPU()
	: _cycleCountPerScanline(0)
	, _currentScaline(0)
	, _firstWrite(true)
	{
	}

	PPU::~PPU()
	{
	}

	byte PPU::read(word address)
	{
		byte ppuRegister = address & PpuRegisterMask;

		switch(ppuRegister)
		{
			// For now, always return that the VBL is ready
		case 2:
			return 1 << 7;
		case 7:
			return _internalRead(_currentPpuAddress);
		}

		return 0;
	}

	void PPU::write(word address, byte value)
	{
		byte ppuRegister = address & PpuRegisterMask;

		switch(ppuRegister)
		{
		case 6:
			_firstWrite ? _currentPpuAddress.setHighByte(value) : _currentPpuAddress.setLowByte(value);
			_firstWrite = !_firstWrite;
			break;
		case 7:
			_internalWrite(_currentPpuAddress, value);
			break;
		}
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

	byte PPU::_internalRead(word ppuAddress)
	{
		word realAddress = ppuAddress & PpuMirroringMask;

		if (realAddress >= 0x3F00 && realAddress < 0x4000)
		{
			return _palette[(realAddress & 0x1F)];
		}

		return 0;
	}

	void PPU::_internalWrite(word ppuAddress, byte value)
	{
		word realAddress = ppuAddress & PpuMirroringMask;

		if (realAddress >= 0x3F00 && realAddress < 0x4000)
		{
			_palette[(realAddress & 0x1F)] = value;
			if (!((realAddress & 0x1F) & 0x3))
			{
				_palette[(realAddress & 0x1F) ^ 0x10] = value;
			}
		}
	}
}