#pragma once

#include "memory.h"

namespace sukiNES
{
	class GamePak;
	class PPUIO;

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

	private:
		bool _isRenderingEnabled() const;
		bool _isOutsideRendering() const;

		void _memoryAccess();
		void _incrementCycleAndScanline();
		void _incrementPpuAddressHorizontal();
		void _incrementPpuAddressVertical();
		void _resetHorizontalPpuAddress();
		void _resetVerticalPpuAddress();
		void _incrementPpuAddressOnReadWrite();

		void _renderPixel();

		byte _internalRead(word ppuAddress);
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
			RegBit<0> grayscale;
			RegBit<1> showBackgroundLeftmost;
			RegBit<2> showSpritesLeftmost;
			RegBit<3> showBackground;
			RegBit<4> showSprites;
			RegBit<5> intensityRed;
			RegBit<6> intensityGreen;
			RegBit<7> intensityBlue;
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
			u16 raw;
			RegBit<0, 5, u16> coarseXScroll;
			RegBit<5, 5, u16> coarseYScroll;
			RegBit<10, 2, u16> nametableSelect;
			RegBit<12, 3, u16> fineYScroll;

			// Special fields used by PPU register PpuAddress
			RegBit<8, 6, u16> highByteAddress;
			RegBit<0, 8, u16> lowByteAddress;
			RegBit<14, 1, u16> clearBit14;
		} _temporaryPpuAddress;

		// Aka Loopy_V
		union
		{
			u16 raw;
			RegBit<0, 5, u16> coarseXScroll;
			RegBit<5, 5, u16> coarseYScroll;
			RegBit<10, 2, u16> nametableSelect;
			RegBit<12, 3, u16> fineYScroll;
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

		byte _readBuffer;

		byte _palette[32];

		NameTableMirroring _nametableMirroring;
		byte _nametable[SUKINES_KB(2)];

		byte _lastPaletteIndex;

		GamePak* _gamePak;
		PPUIO* _io;
	};
}