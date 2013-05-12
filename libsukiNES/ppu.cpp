#include "ppu.h"

namespace sukiNES
{
	static const uint32 CyclesPerScanline = 340;
	static const sint32 ScanlinePerFrame = 260;

	static const uint32 PpuMirroringMask = 0x4000 - 1;
	static const byte PpuRegisterMask = 0x7;
	static const byte PaletteMask = 0x1F;
	static const uint32 NametableAddressMask = 0xFFF;
	static const uint32 NametableHorizontalMask = 0x3FF;
	static const uint32 NametableVerticalMask = 0x7FF;

	static byte PaletteAtPowerOn[32] = {
		0x9,0x1,0x0,0x1,0x0,0x2,0x2,0xD,0x8,0x10,0x8,0x24,0x0,0x0,0x4,0x2C,
		0x9,0x1,0x34,0x3,0x0,0x4,0x0,0x14,0x8,0x3A,0x0,0x2,0x0,0x20,0x2C,0x8
	};

	enum class PpuRegister
	{
		PpuControl = 0,
		PpuMask,
		PpuStatus,
		OamAddress,
		OamData,
		Scroll,
		PpuAddress,
		PpuData
	};

	PPU::PPU()
	: _cycleCountPerScanline(0)
	, _currentScaline(0)
	, _firstWrite(true)
	, _readBuffer(0xFF)
	{
		memcpy(_palette, PaletteAtPowerOn, sizeof(PaletteAtPowerOn) / sizeof(byte));
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
		case PpuRegister::PpuStatus:
			_firstWrite = true;
			return 1 << 7;
		case PpuRegister::PpuData:
			byte readValue = _internalRead(_currentPpuAddress);
			_currentPpuAddress++;
			return readValue;
		}

		return 0;
	}

	void PPU::write(word address, byte value)
	{
		byte ppuRegister = address & PpuRegisterMask;

		switch(ppuRegister)
		{
		case PpuRegister::PpuAddress:
			_firstWrite ? _currentPpuAddress.setHighByte(value) : _currentPpuAddress.setLowByte(value);
			_firstWrite = !_firstWrite;
			break;
		case PpuRegister::PpuData:
			_internalWrite(_currentPpuAddress, value);
			_currentPpuAddress++;
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

		byte returnValue = _readBuffer;

		if (realAddress >= 0x2000 && realAddress < 0x3F00)
		{
			uint32 nameTableAddress = realAddress & NametableAddressMask;
			switch(_nametableMirroring)
			{
				case PPU::NameTableMirroring::Horizontal:
				{
					uint32 inSecondNametable =  ((nameTableAddress & 0x800) >> 1);
					_readBuffer = _nametable[(nameTableAddress & NametableHorizontalMask) | inSecondNametable];
				}
				case PPU::NameTableMirroring::Vertical:
					_readBuffer = _nametable[nameTableAddress & NametableVerticalMask];
				case PPU::NameTableMirroring::FourScreen:
					break;
				case PPU::NameTableMirroring::SingleScreen:
					break;
			}
		}
		else if (realAddress >= 0x3F00 && realAddress < 0x4000)
		{
			// When reading palette data, we need to update 
			// the read buffer but the data read into the buffer
			// is the data found at 0x2F[lowerbyte]
			// This is like we were reading the nametable at the same address
			_internalRead(0x2F00 | (realAddress & 0xFF));
			return _palette[(realAddress & PaletteMask)];
		}

		return returnValue;
	}

	void PPU::_internalWrite(word ppuAddress, byte value)
	{
		word realAddress = ppuAddress & PpuMirroringMask;

		if (realAddress >= 0x2000 && realAddress < 0x3F00)
		{
			uint32 nameTableAddress = realAddress & NametableAddressMask;
			switch(_nametableMirroring)
			{
				case PPU::NameTableMirroring::Horizontal:
				{
					uint32 inSecondNametable =  ((nameTableAddress & 0x800) >> 1);
					_nametable[(nameTableAddress & NametableHorizontalMask) | inSecondNametable] = value;
				}
				case PPU::NameTableMirroring::Vertical:
					_nametable[nameTableAddress & NametableVerticalMask] = value;
				case PPU::NameTableMirroring::FourScreen:
					break;
				case PPU::NameTableMirroring::SingleScreen:
					break;
			}
		}
		else if (realAddress >= 0x3F00 && realAddress < 0x4000)
		{
			_palette[(realAddress & PaletteMask)] = value;
			if (!((realAddress & PaletteMask) & 0x3))
			{
				_palette[(realAddress & PaletteMask) ^ 0x10] = value;
			}
		}
	}
}