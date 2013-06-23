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

		void _nametableFetch();
		void _attributeFetch();
		void _lowBackgroundByteFetch();
		void _highBackgroundByteFetch();

		void _prepareNextTile();

		void _incrementCycleAndScanline();

		void _incrementPpuAddressHorizontal();
		void _incrementPpuAddressVertical();
		void _resetHorizontalPpuAddress();
		void _resetVerticalPpuAddress();
		void _incrementPpuAddressOnReadWrite();

		void _renderPixel();

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
		std::queue<uint16> _backgroundPatternQueue;
		std::queue<byte> _backgroundAttributeQueue;

		word _currentBackgroundPattern;
		byte _currentAttribute;
		word _tempBackgroundPattern;
	};
}