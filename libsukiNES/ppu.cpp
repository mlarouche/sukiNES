#include "ppu.h"

// Local includes
#include "gamepak.h"
#include "ppuio.h"

namespace sukiNES
{
	static const uint32 CyclesPerScanline = 340;
	static const sint32 ScanlinePerFrame = 260;
	static const sint32 PostRenderScanline = 240;
	static const sint32 PreRenderScanline = -1;

	static const uint32 PpuMirroringMask = 0x4000 - 1;
	static const byte PpuRegisterMask = 0x7;
	static const byte PaletteMask = 0x1F;
	static const byte OamDataAttributeReadMask = 0xE3;
	static const uint32 NametableAddressMask = 0xFFF;
	static const uint32 NametableHorizontalMask = 0x3FF;
	static const uint32 NametableVerticalMask = 0x7FF;
	static const uint32 NametableSingleScreenMask = 0x3FF;

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
	: _fineXScroll(0)
	, _rawOAM(nullptr)
	, _oamAddress(0)
	, _cycleCountPerScanline(0)
	, _currentScanline(PreRenderScanline)
	, _firstWrite(true)
	, _readBuffer(0xFF)
	, _lastPaletteIndex(0)
	, _gamePak(nullptr)
	, _io(nullptr)
	{
		_ppuControl.raw = 0;
		_ppuMask.raw = 0;
		_ppuStatus.raw = 0;
		_temporaryPpuAddress.raw = 0;
		_currentPpuAddress.raw = 0;

		_rawOAM = reinterpret_cast<byte*>(_sprites);
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
			case PpuRegister::PpuStatus:
			{
				_firstWrite = true;
				auto ppuStatus = _ppuStatus.raw;
				_ppuStatus.vblankStarted = false;
				return ppuStatus;
			}
			case PpuRegister::OamData:
				return ((_oamAddress+1) % 3 == 0) ? _rawOAM[_oamAddress] & OamDataAttributeReadMask : _rawOAM[_oamAddress];
			case PpuRegister::PpuData:
				byte readValue = _internalRead(_currentPpuAddress.raw);
				_incrementPpuAddressOnReadWrite();
				return readValue;
		}

		return 0;
	}

	void PPU::write(word address, byte value)
	{
		byte ppuRegister = address & PpuRegisterMask;

		switch(ppuRegister)
		{
			case PpuRegister::PpuControl:
				_ppuControl.raw = value;
				_temporaryPpuAddress.nametableSelect = (unsigned)_ppuControl.baseNametableAddress;
				break;
			case PpuRegister::PpuMask:
				_ppuMask.raw = value;
				break;
			case PpuRegister::OamAddress:
				_oamAddress = value;
				break;
			case PpuRegister::OamData:
				_rawOAM[_oamAddress] = value;
				++_oamAddress;
				break;
			case PpuRegister::Scroll:
				if (_firstWrite)
				{
					_temporaryPpuAddress.coarseXScroll = (value & 0xF8) >> 3;
					_fineXScroll = value & 0x7;
					_firstWrite = !_firstWrite;
				}
				else
				{
					_temporaryPpuAddress.coarseYScroll = (value & 0xF8) >> 3;
					_temporaryPpuAddress.fineYScroll = value & 0x7;
					_firstWrite = !_firstWrite;
				}
				break;
			case PpuRegister::PpuAddress:
				if (_firstWrite)
				{
					_temporaryPpuAddress.highByteAddress = value & 0x3F;
					_temporaryPpuAddress.clearBit14 = 0;
					_firstWrite = !_firstWrite;
				}
				else
				{
					_temporaryPpuAddress.lowByteAddress = value;
					_currentPpuAddress.raw = _temporaryPpuAddress.raw;
					_firstWrite = !_firstWrite;
				}
				break;
			case PpuRegister::PpuData:
				_internalWrite(_currentPpuAddress.raw, value);
				_incrementPpuAddressOnReadWrite();
				break;
			}
	}

	void PPU::tick()
	{
		// See http://wiki.nesdev.com/w/images/d/d1/Ntsc_timing.png
		// and http://wiki.nesdev.com/w/index.php/PPU_rendering
		// for more details on how this works.

		if (_currentScanline >= 0 && _currentScanline < PostRenderScanline)
		{
			if (_isRenderingEnabled())
			{
				if (_cycleCountPerScanline >= 0 && _cycleCountPerScanline < 256)
				{
					_renderPixel();
				}

				_memoryAccess();
			}
		}
		else if (_currentScanline == PostRenderScanline)
		{
			// Do nothing
		}
		else if (_currentScanline >= 241 && _currentScanline < 260)
		{
			// In VBlank
			if (_currentScanline == 241 && _cycleCountPerScanline == 1)
			{
				_ppuStatus.vblankStarted = true;
				// TODO: Start NMI in CPU
				if (_io && _isRenderingEnabled())
				{
					_io->onVBlank();
				}
			}
		}
		else if(_currentScanline == PreRenderScanline)
		{
			if (_isRenderingEnabled())
			{
				if (_cycleCountPerScanline == 1)
				{
					_ppuStatus.raw = 0;
				}
				if (_cycleCountPerScanline >= 280 && _cycleCountPerScanline < 305)
				{
					_resetVerticalPpuAddress();
				}

				_memoryAccess();
			}
		}

		_incrementCycleAndScanline();
	}

	void PPU::_memoryAccess()
	{
		if (_cycleCountPerScanline == 0)
		{
		}
		else if(_cycleCountPerScanline >= 1 && _cycleCountPerScanline < 256)
		{
			if (_cycleCountPerScanline % 8 == 0)
			{
				_incrementPpuAddressHorizontal();
			}
		}
		else if(_cycleCountPerScanline == 256)
		{
			_incrementPpuAddressVertical();
		}
		else if(_cycleCountPerScanline == 257)
		{
			_resetHorizontalPpuAddress();
		}
		else if(_cycleCountPerScanline >= 258 && _cycleCountPerScanline < 321)
		{
			
		}
		else if(_cycleCountPerScanline >= 321 && _cycleCountPerScanline < 337)
		{
			if (_cycleCountPerScanline % 8 == 0)
			{
				_incrementPpuAddressHorizontal();
			}
		}
		else
		{
			
		}
	}

	bool PPU::_isRenderingEnabled() const
	{
		return (unsigned)_ppuMask.showBackground || (unsigned)_ppuMask.showSprites;
	}

	bool PPU::_isOutsideRendering() const
	{
		if ((_currentScanline >= 240 && _currentScanline < 260) || (_currentScanline == -1))
		{
			return true;
		}
		else
		{
			return !_isRenderingEnabled();
		}
	}

	void PPU::_incrementCycleAndScanline()
	{
		_cycleCountPerScanline++;
		if (_cycleCountPerScanline > CyclesPerScanline)
		{
			_cycleCountPerScanline = 0;
			_currentScanline++;
			if (_currentScanline > ScanlinePerFrame)
			{
				_currentScanline = -1;
			}
		}
	}

	void PPU::_incrementPpuAddressHorizontal()
	{
		if (((unsigned)_currentPpuAddress.coarseXScroll & 0x1F) == 31)
		{
			_currentPpuAddress.coarseXScroll = 0;
			// Switch horizontal nametable
			_currentPpuAddress.nametableSelect = (unsigned)_currentPpuAddress.nametableSelect ^ SUKINES_BIT(0);
		}
		else
		{
			_currentPpuAddress.coarseXScroll = (unsigned)_currentPpuAddress.coarseXScroll + 1;
		}
	}

	void PPU::_incrementPpuAddressVertical()
	{
		if ((unsigned)_currentPpuAddress.fineYScroll < 7)
		{
			_currentPpuAddress.fineYScroll = (unsigned)_currentPpuAddress.fineYScroll + 1;
		}
		else
		{
			_currentPpuAddress.raw &= 0x0FFF;
			sint32 y = (unsigned)_currentPpuAddress.coarseYScroll;
			if (y == 29)
			{
				y = 0;
				// Switch vertical nametable
				_currentPpuAddress.nametableSelect = (unsigned)_currentPpuAddress.nametableSelect ^ SUKINES_BIT(1);
			}
			else if (y == 31)
			{
				y = 0;
			}
			else
			{
				++y;
			}

			_currentPpuAddress.coarseYScroll = y;
		}
	}

	void PPU::_resetHorizontalPpuAddress()
	{
		_currentPpuAddress.coarseXScroll = (unsigned)_temporaryPpuAddress.coarseXScroll;
		_currentPpuAddress.nametableSelect = ((unsigned)_currentPpuAddress.nametableSelect & ~SUKINES_BIT(0)) | ((unsigned)_temporaryPpuAddress.nametableSelect & SUKINES_BIT(0));
	}

	void PPU::_resetVerticalPpuAddress()
	{
		_currentPpuAddress.coarseYScroll = (unsigned)_temporaryPpuAddress.coarseYScroll;
		_currentPpuAddress.fineYScroll = (unsigned)_temporaryPpuAddress.fineYScroll;
		_currentPpuAddress.nametableSelect = ((unsigned)_currentPpuAddress.nametableSelect & ~SUKINES_BIT(1)) | ((unsigned)_temporaryPpuAddress.nametableSelect & SUKINES_BIT(1));
	}

	void PPU::_incrementPpuAddressOnReadWrite()
	{
		if (_isOutsideRendering())
		{
			(unsigned)_ppuControl.addressIncrement ? _currentPpuAddress.raw += 32 : ++_currentPpuAddress.raw;
		}
		else
		{
			_incrementPpuAddressVertical();
		}
	}

	void PPU::_renderPixel()
	{
		if (_io)
		{
			_io->putPixel(_cycleCountPerScanline, _currentScanline, _lastPaletteIndex);
		}
	}

	byte PPU::_internalRead(word ppuAddress)
	{
		word realAddress = ppuAddress & PpuMirroringMask;

		byte returnValue = _readBuffer;

		if (realAddress < 0x2000)
		{
			return _gamePak->readChr(realAddress);
		}
		else if (realAddress >= 0x2000 && realAddress < 0x3F00)
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
					_readBuffer = _nametable[nameTableAddress & NametableSingleScreenMask];
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
					_nametable[nameTableAddress & NametableSingleScreenMask] = value;
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