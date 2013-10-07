#include "ppu.h"

// STL includes
#include <algorithm>

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
	: _rawOAM(nullptr)
	, _rawSecondaryOAM(nullptr)
	, _gamePak(nullptr)
	, _io(nullptr)
	{
		_rawOAM = reinterpret_cast<byte*>(_sprites);
		_rawSecondaryOAM = reinterpret_cast<byte*>(_secondaryOAM);

		powerOn();
	}

	PPU::~PPU()
	{
	}

	void PPU::powerOn()
	{
		_ppuControl.raw = 0;
		_ppuMask.raw = 0;
		_ppuStatus.raw = 0;
		_temporaryPpuAddress.raw = 0;
		_currentPpuAddress.raw = 0;

		_fineXScroll = 0;
		_oamAddress = 0;
		_secondaryOAMIndex = 0;
		_cycleCountPerScanline = 0;
		_currentScanline = PreRenderScanline;
		_isEvenFrame = true;
		_skipNmi = false;
		_irqNotRead = false;
		_firstWrite = true;
		_readBuffer = 0xFF;
		_lastReadNametableByte = 0;
		_currentAttribute = 0;
		_currentAttributeBits = 0;
		_currentSpriteFetched = 0;

		_spriteEval.clear();
		for(uint32 i = 0; i < 8; ++i)
		{
			_spritesToRender[i].clear();
		}

		memcpy(_palette, PaletteAtPowerOn, sizeof(PaletteAtPowerOn) / sizeof(byte));

		std::fill(std::begin(_nametable), std::end(_nametable), 0);
	}

	byte PPU::read(word address)
	{
		byte ppuRegister = address & PpuRegisterMask;

		switch(ppuRegister)
		{
			case PpuRegister::PpuStatus:
			{
				_firstWrite = true;
				
				if (_currentScanline == 241 && _cycleCountPerScanline == 0)
				{
					_ppuStatus.vblankStarted = false;
					_skipNmi = true;
				}
				else if (_currentScanline == 241 && _cycleCountPerScanline == 1)
				{
					_ppuStatus.vblankStarted = true;
					_skipNmi = true;
				}
				
				auto ppuStatus = _ppuStatus.raw;
				_ppuStatus.vblankStarted = false;
				return ppuStatus;
			}
			case PpuRegister::OamData:
				if (_isRenderingEnabled()
					&& (_cycleCountPerScanline >= 1 && _cycleCountPerScanline <= 64)
					&& (_currentScanline >= 0 && _currentScanline < PostRenderScanline)
					)
				{
					return 0xFF;
				}
				else
				{
					return ((_oamAddress+1) % 3 == 0) ? _rawOAM[_oamAddress] & OamDataAttributeReadMask : _rawOAM[_oamAddress];
				}
			case PpuRegister::PpuData:
				byte readValue = _internalRead(_currentPpuAddress.raw, PPU::ReadSource::FromRegister);
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
			if (_cycleCountPerScanline >= 0 && _cycleCountPerScanline < 256)
			{
				if (_isRenderingEnabled())
				{
					auto currentPixel = (_cycleCountPerScanline+_fineXScroll) & 7;
					if (_cycleCountPerScanline == 0)
					{
						_prepareNextTile();
					}
					else if (currentPixel == 0)
					{
						_prepareNextTile();
					}

					_renderPixel();
				}
				else
				{
					_renderBackground();
				}
			}

			if (_isRenderingEnabled())
			{
				_memoryAccess();
			}
		}
		else if (_currentScanline == PostRenderScanline)
		{
			// Do nothing
		}
		else if (_currentScanline >= 241 && _currentScanline <= 260)
		{
			// In VBlank
			if (_currentScanline == 241 && _cycleCountPerScanline == 1)
			{
				if (!_skipNmi)
				{
					_ppuStatus.vblankStarted = true;
					_irqNotRead = true;
				}
				else
				{
					_skipNmi = false;
				}

				if (_io)
				{
					_io->onVBlank();
				}
			}
		}
		else if(_currentScanline == PreRenderScanline)
		{
			if (_cycleCountPerScanline == 339)
			{
				if (!_isEvenFrame && _isRenderingEnabled())
				{
					++_cycleCountPerScanline;
				}
				_isEvenFrame = !_isEvenFrame;
			}

			if (_cycleCountPerScanline == 1)
			{
				_ppuStatus.raw = 0;
			}

			if (_isRenderingEnabled())
			{
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
			if (_cycleCountPerScanline >= 1 && _cycleCountPerScanline <= 64)
			{
				_rawSecondaryOAM[_secondaryOAMIndex] = 0xFF;
				++_secondaryOAMIndex;
				if (_secondaryOAMIndex > 32)
				{
					_secondaryOAMIndex = 0;
					_spriteEval.oamIndex = _oamAddress / 4;
				}
			}
			else
			{
				_spriteEvaluation();
			}

			auto whichAction = static_cast<PPU::MemoryAccessAction>(_cycleCountPerScanline % 8);
			switch(whichAction)
			{
				case MemoryAccessAction::NametableFetch:
					_nametableFetch();
					break;
				case MemoryAccessAction::AttributeFetch:
					_attributeFetch();
					break;
				case MemoryAccessAction::LowTileFetch:
					_backgroundByteFetch(whichAction);
					break;
				case MemoryAccessAction::HighTileFetch:
					_backgroundByteFetch(whichAction);

					_incrementPpuAddressHorizontal();
					break;
				default:
					break;
			}
		}
		else if(_cycleCountPerScanline == 256)
		{
			_spriteEvaluation();

			_backgroundByteFetch(MemoryAccessAction::HighTileFetch);

			_incrementPpuAddressHorizontal();
			_incrementPpuAddressVertical();
		}
		else if(_cycleCountPerScanline >= 257 && _cycleCountPerScanline < 321)
		{
			if (_cycleCountPerScanline == 257)
			{
				_resetHorizontalPpuAddress();

				_spriteEval.clear();
				_currentSpriteFetched = 0;
				for (uint32 i = 0; i < 8; ++i)
				{
					_spritesToRender[i].clear();
				}
			}

			_oamAddress = 0;

			auto whichAction = static_cast<PPU::MemoryAccessAction>(_cycleCountPerScanline % 8);
			switch(whichAction)
			{
				case MemoryAccessAction::NametableFetch:
					{
						if (!_secondaryOAM[_currentSpriteFetched].isNull())
						{
							_spritesToRender[_currentSpriteFetched].x = _secondaryOAM[_currentSpriteFetched].x;
							_spritesToRender[_currentSpriteFetched].attribute = _secondaryOAM[_currentSpriteFetched].attributes;
							_spritesToRender[_currentSpriteFetched].isFirstSprite = (unsigned)_secondaryOAM[_currentSpriteFetched].attributes.unimplemented;
						}
						break;
					}
				case MemoryAccessAction::LowTileFetch:
					if (!_secondaryOAM[_currentSpriteFetched].isNull())
					{
						_spriteByteFetch(whichAction);
					}
					break;
				case MemoryAccessAction::HighTileFetch:
					if (!_secondaryOAM[_currentSpriteFetched].isNull())
					{
						_spriteByteFetch(whichAction);

						++_currentSpriteFetched;
					}
					break;
				default:
					break;
			}
		}
		else if(_cycleCountPerScanline >= 321 && _cycleCountPerScanline < 337)
		{
			if (_cycleCountPerScanline == 321)
			{
				while (!_backgroundAttributeQueue.empty())
				{
					_backgroundAttributeQueue.pop();
				}
				while (!_backgroundPatternQueue.empty())
				{
					_backgroundPatternQueue.pop();
				}
				while(!_attributeBitsQueue.empty())
				{
					_attributeBitsQueue.pop();
				}
			}

			auto whichAction = _cycleCountPerScanline % 8;
			switch(whichAction)
			{
				case MemoryAccessAction::NametableFetch:
					_nametableFetch();
					break;
				case MemoryAccessAction::AttributeFetch:
					_attributeFetch();
					break;
				case MemoryAccessAction::LowTileFetch:
					_backgroundByteFetch(MemoryAccessAction::LowTileFetch);
					break;
				case MemoryAccessAction::HighTileFetch:
					_backgroundByteFetch(MemoryAccessAction::HighTileFetch);

					_incrementPpuAddressHorizontal();
					break;
				default:
					break;
			}
		}
		else
		{
			switch(_cycleCountPerScanline)
			{
				case 338:
				case 340:
					_nametableFetch();
					break;
				default:
					break;
			}
		}
	}

	void PPU::_spriteEvaluation()
	{
		auto spriteSize = (unsigned)_ppuControl.spriteSize ? 16 : 8;

		switch(_spriteEval.currentState)
		{
		case SpriteEvaluation::CheckSpriteInRange:
			{
				byte y = _sprites[_spriteEval.oamIndex].y;
				auto range = _currentScanline - y;
				if (range >= 0 && range < spriteSize)
				{
					if (_spriteEval.spritesFound < 8)
					{
						_secondaryOAM[_spriteEval.spritesFound] = _sprites[_spriteEval.oamIndex];
						_secondaryOAM[_spriteEval.spritesFound].attributes.unimplemented = static_cast<byte>((_spriteEval.oamIndex == 0));

						++_spriteEval.spritesFound;
					}
				}
				_spriteEval.currentState = SpriteEvaluation::GotoNextSprite;
				break;
			}
		case SpriteEvaluation::GotoNextSprite:
			{
				++_spriteEval.oamIndex;
				if (_spriteEval.oamIndex >= 64)
				{
					_spriteEval.oamIndex = 0;
				}

				if (_spriteEval.oamIndex == 0)
				{
					_spriteEval.currentState = SpriteEvaluation::Done;
				}
				else if (_spriteEval.spritesFound < 8)
				{
					_spriteEval.currentState = SpriteEvaluation::CheckSpriteInRange;
				}
				else if (_spriteEval.spritesFound == 8)
				{
					_spriteEval.currentState = SpriteEvaluation::CheckSpriteOverflow;
				}
				break;
			}
		case SpriteEvaluation::CheckSpriteOverflow:
			{
				byte y = _sprites[_spriteEval.oamIndex].y;
				auto range = _currentScanline - y;
				if (range >= 0 && range < spriteSize)
				{
					_ppuStatus.spriteOverflow = true;
				}

				++_spriteEval.oamIndex;
				if (_spriteEval.oamIndex > 64)
				{
					_spriteEval.oamIndex = 0;
					_spriteEval.currentState = SpriteEvaluation::Done;
				}
				break;
			}
		case SpriteEvaluation::Done:
			break;
		}
	}

	bool PPU::_isRenderingEnabled() const
	{
		return (unsigned)_ppuMask.showBackground || (unsigned)_ppuMask.showSprites;
	}

	bool PPU::_isOutsideRendering() const
	{
		if ((_currentScanline >= PostRenderScanline && _currentScanline <= ScanlinePerFrame) || (_currentScanline == PreRenderScanline))
		{
			return true;
		}
		else
		{
			return !_isRenderingEnabled();
		}
	}

	void PPU::_nametableFetch()
	{
		word nametableAddress = 0x2000 | (_currentPpuAddress.raw & 0x0FFF);
		_lastReadNametableByte = _internalRead(nametableAddress);
	}

	void PPU::_attributeFetch()
	{
		word attributeAddress = 0x23C0
			| (_currentPpuAddress.raw & 0x0C00) // Nametable select
			| ((_currentPpuAddress.raw >> 4) & 0x38) // High 3 bits of Coarse Y (y/4)
			| ((_currentPpuAddress.raw >> 2) & 0x07); // High 3 bits of Coarse X (x/4)
		_backgroundAttributeQueue.push( _internalRead(attributeAddress) );

		auto attributeX = (unsigned)_currentPpuAddress.coarseXScroll % 4;
		auto attributeY = (unsigned)_currentPpuAddress.coarseYScroll % 4;
		byte whichAttributeBits = (attributeX >> 1) | (attributeY & 2);

		_attributeBitsQueue.push(whichAttributeBits);
	}

	void PPU::_backgroundByteFetch(PPU::MemoryAccessAction memoryAccess)
	{
		byte readTile = _readTile((unsigned)_ppuControl.backgroundPatternTable, _lastReadNametableByte, (unsigned)_currentPpuAddress.fineYScroll, memoryAccess);

		switch(memoryAccess)
		{
		case MemoryAccessAction::LowTileFetch:
			_tempBackgroundPattern.lowByte = readTile;
			break;
		case MemoryAccessAction::HighTileFetch:
			_tempBackgroundPattern.highByte = readTile;

			_backgroundPatternQueue.push(_tempBackgroundPattern);
			break;
		default:
			break;
		}
	}

	void PPU::_spriteByteFetch(PPU::MemoryAccessAction memoryAccess)
	{
		auto spriteSize = (unsigned)_ppuControl.spriteSize ? 16 : 8;

		byte bank = 0;
		byte tileNumber = 0;

		byte fineY = _currentScanline - _secondaryOAM[_currentSpriteFetched].y;
		if ((unsigned)_secondaryOAM[_currentSpriteFetched].attributes.flipVertical)
		{
			fineY ^= 0xF;
		}

		if (spriteSize == 16)
		{
			bank = _secondaryOAM[_currentSpriteFetched].tileIndex & 1;
			tileNumber = (_secondaryOAM[_currentSpriteFetched].tileIndex & 0xFE);

			if (fineY & 8)
			{
				tileNumber++;
			}
		}
		else
		{
			bank = (unsigned)_ppuControl.spritePatternTable;
			tileNumber = _secondaryOAM[_currentSpriteFetched].tileIndex;
		}

		fineY &= 7;

		byte readTile = _readTile(bank, tileNumber, fineY, memoryAccess);
		switch(memoryAccess)
		{
		case MemoryAccessAction::LowTileFetch:
			_spritesToRender[_currentSpriteFetched].pattern.lowByte = readTile;
			break;
		case MemoryAccessAction::HighTileFetch:
			_spritesToRender[_currentSpriteFetched].pattern.highByte = readTile;
			break;
		default:
			break;
		}
	}

	byte PPU::_readTile(byte patternBank, byte tileNumber, byte fineY, PPU::MemoryAccessAction memoryAccess)
	{
		byte highTileOffset = (memoryAccess == MemoryAccessAction::HighTileFetch) ? 8 : 0;

		uint16 chrAddress = patternBank*0x1000 | (tileNumber*16 + fineY + highTileOffset);

		return _internalRead(chrAddress);
	}

	void PPU::_prepareNextTile()
	{
		if (!_backgroundPatternQueue.empty())
		{
			_currentBackgroundPattern = _backgroundPatternQueue.front();
			_backgroundPatternQueue.pop();
		}

		if (!_backgroundAttributeQueue.empty())
		{
			_currentAttribute = _backgroundAttributeQueue.front();
			_backgroundAttributeQueue.pop();
		}

		if (!_attributeBitsQueue.empty())
		{
			_currentAttributeBits = _attributeBitsQueue.front();
			_attributeBitsQueue.pop();
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
		if ((unsigned)_currentPpuAddress.coarseXScroll == 31)
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
			_currentPpuAddress.fineYScroll = ((unsigned)_currentPpuAddress.fineYScroll) + 1;
		}
		else
		{
			_currentPpuAddress.fineYScroll = 0;
			unsigned y = (unsigned)_currentPpuAddress.coarseYScroll;
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
			if ((unsigned)_ppuControl.addressIncrement)
			{
				_currentPpuAddress.raw += 32;
			}
			else
			{
				++_currentPpuAddress.raw;
			}
		}
		else
		{
			_incrementPpuAddressVertical();
		}
	}

	void PPU::_renderPixel()
	{
		union
		{
			byte raw;
			RegBit<0,2> pixelTile;
			RegBit<2,2> paletteNumber;
			RegBit<4> isSpritePalette;
		} paletteIndex;

		paletteIndex.raw = 0;

		bool renderSprite = false;
		byte backgroundPixel = 0;
		byte backgroundAttribute = 0;
		byte spritePixel = 0;
		byte spriteAttribute = 0;

		if ((unsigned)_ppuMask.showBackground)
		{
			uint32 backgroundColumn = 7 - ((_cycleCountPerScanline+_fineXScroll) % 8);
			backgroundPixel = _currentBackgroundPattern.pixel(backgroundColumn);
			backgroundAttribute = (_currentAttribute >> (_currentAttributeBits * 2)) & 0x3;
		}

		if ((unsigned)_ppuMask.showSprites)
		{
			if (!((unsigned)_ppuMask.showSpritesLeftmost) && _cycleCountPerScanline < 8)
			{
			}
			else
			{
				for (uint32 spriteIndex = 0; spriteIndex < 8; ++spriteIndex)
				{
					if (_spritesToRender[spriteIndex].inRange(_cycleCountPerScanline))
					{
						spritePixel = _spritesToRender[spriteIndex].pixel(_cycleCountPerScanline);
						spriteAttribute = (unsigned)_spritesToRender[spriteIndex].attribute.palette;

						if (_spritesToRender[spriteIndex].isFirstSprite
							&& backgroundPixel > 0
							&& spritePixel > 0
							&& _cycleCountPerScanline < 255
							&& (unsigned)_ppuMask.showBackground)
						{
							_ppuStatus.sprite0Hit = true;
						}

						if (backgroundPixel == 0 && spritePixel > 0)
						{
							renderSprite = true;
							break;
						}
						if (backgroundPixel > 0 && spritePixel > 0 && !(unsigned)_spritesToRender[spriteIndex].attribute.priority)
						{
							renderSprite = true;
							break;
						}
					}
				}
			}
		}

		if (renderSprite)
		{
			paletteIndex.pixelTile = spritePixel;
			paletteIndex.paletteNumber = spriteAttribute;
		}
		else if ((unsigned)_ppuMask.showBackground)
		{
			paletteIndex.pixelTile = backgroundPixel;
			paletteIndex.paletteNumber = backgroundAttribute;

			if (!((unsigned)_ppuMask.showBackgroundLeftmost) && _cycleCountPerScanline < 8)
			{
				paletteIndex.raw = 0;
			}
		}

		paletteIndex.isSpritePalette = renderSprite;

		if ( !((paletteIndex.raw & 0x1F) & 0x3) )
		{
			paletteIndex.raw = 0;
		}

		_drawPixel(_palette[paletteIndex.raw]);
	}

	void PPU::_renderBackground()
	{
		if (_currentPpuAddress.raw >= 0x3F00 && _currentPpuAddress.raw  < 0x4000)
		{
			auto temp = _internalRead(_currentPpuAddress.raw);
			_drawPixel(temp);
		}
		else
		{
			_drawPixel(_palette[0]);
		}
	}

	void PPU::_drawPixel(byte paletteValue)
	{
		if (_io)
		{
			if ((unsigned)_ppuMask.greyscale)
			{
				paletteValue &= 0x30;
			}

			_io->putPixel(_cycleCountPerScanline, _currentScanline, paletteValue);
		}
	}

	byte PPU::_internalRead(word ppuAddress, PPU::ReadSource readSource)
	{
		word realAddress = ppuAddress & PpuMirroringMask;

		byte returnValue = _readBuffer;

		if (realAddress < 0x2000)
		{
			_readBuffer = _gamePak->readChr(realAddress);
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
					break;
				}
				case PPU::NameTableMirroring::Vertical:
					_readBuffer = _nametable[nameTableAddress & NametableVerticalMask];
					break;
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

		switch(readSource)
		{
		case PPU::ReadSource::FromPPU:
			return _readBuffer;
		default:
			return returnValue;
		}
	}

	void PPU::_internalWrite(word ppuAddress, byte value)
	{
		word realAddress = ppuAddress & PpuMirroringMask;
		if (realAddress < 0x2000)
		{
			_gamePak->writeChr(ppuAddress, value);
		}
		else if (realAddress >= 0x2000 && realAddress < 0x3F00)
		{
			uint32 nameTableAddress = realAddress & NametableAddressMask;
			switch(_nametableMirroring)
			{
				case PPU::NameTableMirroring::Horizontal:
				{
					uint32 inSecondNametable =  ((nameTableAddress & 0x800) >> 1);
					_nametable[(nameTableAddress & NametableHorizontalMask) | inSecondNametable] = value;
					break;
				}
				case PPU::NameTableMirroring::Vertical:
					_nametable[nameTableAddress & NametableVerticalMask] = value;
					break;
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