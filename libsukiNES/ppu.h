#pragma once

// STL includes
#include <queue>

// Local includes
#include "memory.h"

namespace sukiNES
{
	class GamePak;
	class PPUIO;

	class PPU : public IMemory
	{
	public:
		PPU();
		virtual ~PPU();

		virtual byte read(word address);
		virtual void write(word address, byte value);

		void powerOn();

		void tick();

		void forceCurrentScanline(sint32 value)
		{
			_cycleCountPerScanline = 0;
			_currentScanline = value;
		}

		uint32 cyclesCountPerScanline() const
		{
			return _cycleCountPerScanline;
		}

		sint32 currentScanline() const
		{
			return _currentScanline;
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

		void setGamePak(GamePak* gamePak)
		{
			_gamePak = gamePak;
		}

		void setIO(PPUIO* io)
		{
			_io = io;
		}

		bool hasVBlankOccured()
		{
			if (_irqNotRead && (unsigned)_ppuControl.generateNmi && (unsigned)_ppuStatus.vblankStarted)
			{
				_irqNotRead = false;
				return true;
			}
			else
			{
				return false;
			}
		}

		friend class PPUDebugInfoDialog;
		friend class PPUVideoDialog;

	private:
		bool _isRenderingEnabled() const;
		bool _isOutsideRendering() const;

		void _memoryAccess();
		void _spriteEvaluation();

		void _nametableFetch();
		void _attributeFetch();
		void _lowBackgroundByteFetch();
		void _highBackgroundByteFetch();
		void _lowSpriteByteFetch();
		void _highSpriteByteFetch();

		void _prepareNextTile();

		void _incrementCycleAndScanline();

		void _incrementPpuAddressHorizontal();
		void _incrementPpuAddressVertical();
		void _resetHorizontalPpuAddress();
		void _resetVerticalPpuAddress();
		void _incrementPpuAddressOnReadWrite();

		void _renderPixel();
		void _renderBackground();
		void _drawPixel(byte paletteValue);

		enum class ReadSource
		{
			FromPPU,
			FromRegister
		};

		byte _internalRead(word ppuAddress, ReadSource readSource = ReadSource::FromPPU);
		void _internalWrite(word ppuAddress, byte value);

	private:
		union
		{
			byte raw;
			RegBit<0, 2> baseNametableAddress;
			RegBit<2> addressIncrement;
			RegBit<3> spritePatternTable;
			RegBit<4> backgroundPatternTable;
			RegBit<5> spriteSize;
			RegBit<6> ppuMasterSlave;
			RegBit<7> generateNmi;
		} _ppuControl;

		union
		{
			byte raw;
			RegBit<0> greyscale;
			RegBit<1> showBackgroundLeftmost;
			RegBit<2> showSpritesLeftmost;
			RegBit<3> showBackground;
			RegBit<4> showSprites;
			RegBit<5> intensifyRed;
			RegBit<6> intensifyGreen;
			RegBit<7> intensifyBlue;
		} _ppuMask;

		union
		{
			byte raw;
			RegBit<0, 5> leastBits;
			RegBit<5> spriteOverflow;
			RegBit<6> sprite0Hit;
			RegBit<7> vblankStarted;
		} _ppuStatus;

		// Aka Loopy_T
		union
		{
			uint16 raw;
			RegBit<0, 5, uint16> coarseXScroll;
			RegBit<5, 5, uint16> coarseYScroll;
			RegBit<10, 2, uint16> nametableSelect;
			RegBit<12, 3, uint16> fineYScroll;

			// Special fields used by PPU register PpuAddress
			RegBit<8, 6, uint16> highByteAddress;
			RegBit<0, 8, uint16> lowByteAddress;
			RegBit<14, 1, uint16> clearBit14;
		} _temporaryPpuAddress;

		// Aka Loopy_V
		union
		{
			uint16 raw;
			RegBit<0, 5, uint16> coarseXScroll;
			RegBit<5, 5, uint16> coarseYScroll;
			RegBit<10, 2, uint16> nametableSelect;
			RegBit<12, 3, uint16> fineYScroll;
		} _currentPpuAddress;

		// Aka Loopy_W
		bool _firstWrite;
		// Aka Loopy_X
		byte _fineXScroll;

		struct PPUPattern
		{
			PPUPattern()
			: lowByte(0)
			, highByte(0)
			{}

			byte lowByte;
			byte highByte;

			void clear()
			{
				lowByte = 0;
				highByte = 0;
			}

			byte pixel(uint32 column) const
			{
				return ((lowByte >> column) & 0x1)
					| (((highByte >> column) & 0x1) << 1);
			}
		};

		union SpriteAttribute
		{
			byte raw;
			RegBit<0, 2> palette;
			RegBit<2, 3> unimplemented;
			RegBit<5> priority;
			RegBit<6> flipHorizontal;
			RegBit<7> flipVertical;
		};

		struct OAMEntry
		{
			OAMEntry()
			: y(0)
			, tileIndex(0)
			, x(0)
			{
				attributes.raw = 0;
			}

			OAMEntry &operator=(const OAMEntry& other)
			{
				if (this != &other)
				{
					y = other.y;
					tileIndex = other.tileIndex;
					attributes = other.attributes;
					x = other.x;
				}

				return *this;
			}

			bool isNull() const
			{
				return y == 0xFF
					&& tileIndex == 0xFF
					&& attributes.raw == 0xFF
					&& x == 0xFF;
			}

			byte y;
			byte tileIndex;
			SpriteAttribute attributes;
			byte x;
		} _sprites[64], _secondaryOAM[8];

		struct SpriteRenderingEntry
		{
			SpriteRenderingEntry()
			{
				clear();
			}

			sint32 x;
			SpriteAttribute attribute;
			PPUPattern pattern;

			void clear()
			{
				x = -1;
				attribute.raw = 0;
				pattern.clear();
			}

			byte pixel(sint32 screenX) const
			{
				auto column = 0;
				if ((unsigned)attribute.flipHorizontal)
				{
					column = screenX - x;
				}
				else
				{
					column = 7 - (screenX - x);
				}

				return pattern.pixel(column);
			}

			bool inRange(sint32 screenX) const
			{
				if (x < 0)
				{
					return false;
				}

				auto range = screenX - x;
				return range >= 0 && range < 8;
			}

		} _spritesToRender[8];

		byte* _rawOAM;
		byte* _rawSecondaryOAM;
		byte _oamAddress;
		byte _secondaryOAMIndex;

		uint32 _cycleCountPerScanline;
		sint32 _currentScanline;
		bool _isEvenFrame;
		bool _skipNmi;
		bool _irqNotRead;

		byte _readBuffer;

		byte _palette[32];

		NameTableMirroring _nametableMirroring;
		byte _nametable[SUKINES_KB(2)];

		GamePak* _gamePak;
		PPUIO* _io;

		byte _lastReadNametableByte;
		PPUPattern _tempBackgroundPattern;

		std::queue<PPUPattern> _backgroundPatternQueue;
		std::queue<byte> _backgroundAttributeQueue;
		std::queue<byte> _attributeBitsQueue;

		PPUPattern _currentBackgroundPattern;
		byte _currentAttribute;
		byte _currentAttributeBits;

		struct SpriteEvaluation
		{
			enum CurrentState
			{
				CheckSpriteInRange,
				GotoNextSprite,
				CheckSpriteOverflow,
				Done
			} currentState;

			byte spritesFound;
			byte oamIndex;

			SpriteEvaluation()
			{
				clear();
			}

			void clear()
			{
				currentState = CheckSpriteInRange;
				spritesFound = 0;
				oamIndex = 0;
			}
		} _spriteEval;

		byte _currentSpriteFetched;
	};
}