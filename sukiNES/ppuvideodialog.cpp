#include "ppuvideodialog.h"

// Qt includes
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

// sukiNES includes
#include <ppu.h>

// Local includes
#include "nespalette.h"

PPUVideoDialog::PPUVideoDialog(QWidget* parent)
: QDialog(parent)
{
	_ui.setupUi(this);

	getNESPalette(_palette, sizeof(_palette) / sizeof(QRgb));
}

PPUVideoDialog::~PPUVideoDialog()
{

}

void PPUVideoDialog::updatePPUInfo(sukiNES::PPU* ppu)
{
	_updateMirroringType(ppu);
	_updatePalette(ppu);
	_updateNametable(ppu);
}

void PPUVideoDialog::_updateMirroringType(sukiNES::PPU* ppu)
{
	const char* ppuMirroringName = nullptr;

	switch(ppu->_nametableMirroring)
	{
	case sukiNES::PPU::NameTableMirroring::Horizontal:
		ppuMirroringName = "Horizontal";
		break;
	case sukiNES::PPU::NameTableMirroring::Vertical:
		ppuMirroringName = "Vertical";
		break;
	case sukiNES::PPU::NameTableMirroring::SingleScreen:
		ppuMirroringName = "Single Screen";
		break;
	case sukiNES::PPU::NameTableMirroring::FourScreen:
		ppuMirroringName = "Four Screen";
		break;
	case sukiNES::PPU::NameTableMirroring::ChrRomMirroring:
		ppuMirroringName = "CHR ROM Mirroring";
		break;
	}

	_ui.labelMirroringType->setText(ppuMirroringName);
}

void PPUVideoDialog::_updatePalette(sukiNES::PPU* ppu)
{
	QImage paletteViewBuffer(256, 32, QImage::Format_RGB32);

	QPainter painter(&paletteViewBuffer);

	int x = 0;
	int y = 0;

	for (uint32 paletteIndex = 0; paletteIndex < 32; ++paletteIndex)
	{
		byte paletteValue = ppu->_palette[paletteIndex];

		painter.fillRect(x, y, 16, 16, QColor(_palette[paletteValue]));
		x += 16;
		if (x >= 256)
		{
			x = 0;
			y = 16;
		}
	}

	_ui.labelPaletteView->setPixmap(QPixmap::fromImage(paletteViewBuffer));
}

void PPUVideoDialog::_updateNametable(sukiNES::PPU* ppu)
{
	static const uint32 AttributeSize = 64;
	static const uint32 NametableWidthInTile = 32;
	static const uint32 NametableHeightInTile = 30;
	static const uint32 NametableSize = NametableWidthInTile*NametableHeightInTile;
	static const uint32 NametableWidth = 256;
	static const uint32 NametableHeight = 240;

	QImage nametableBuffer(NametableWidth*2, NametableHeight*2, QImage::Format_RGB32);

	// We are using _internalRead to fetch PPU data and it affects the
	// read buffer. To make this function non-destruction of the PPU state,
	// I create a copy of the current read buffer that I will restore after rendering the nametable
	byte ppuReadBufferCopy = ppu->_readBuffer;

	word nametableAddress = 0x2000;
	word attributeAddress = 0x23C0;

	union
	{
		byte raw;
		RegBit<0,2> pixelTile;
		RegBit<2,2> paletteNumber;
		RegBit<4> isSpritePalette;
	} paletteIndex;

	sint32 currentLineInTile = 0;
	sint32 currentColumnInTile = 0;
	uint32 currentPixel = 0;

	for(uint32 whichNametable = 0; whichNametable<4; ++whichNametable)
	{
		byte attributeTable[AttributeSize];
		for (uint32 i = 0; i<sizeof(attributeTable)/sizeof(byte); ++i)
		{
			attributeTable[i] = ppu->_internalRead(attributeAddress);
			attributeAddress++;
		}

		sint32 currentLineInTile = 0;
		sint32 currentColumnInTile = 0;
		uint32 currentPixel = 0;

		for(uint32 nametableIndex = 0; nametableIndex<NametableSize; ++nametableIndex)
		{
			byte nametableEntry = ppu->_internalRead(nametableAddress);
			byte attribute = attributeTable[(currentLineInTile/4)*8+(currentColumnInTile/4)];

			for (sint32 y = 0; y < 8; ++y)
			{
				uint16 chrAddress = 0;
				chrAddress = ((unsigned)ppu->_ppuControl.backgroundPatternTable) * 0x1000;
				chrAddress |= (nametableEntry * 16) + y;

				byte lowPatternTable = ppu->_internalRead(chrAddress);

				chrAddress = ((unsigned)ppu->_ppuControl.backgroundPatternTable) * 0x1000;
				chrAddress |= (nametableEntry * 16) + 8 + y;
				byte highPatternTable = ppu->_internalRead(chrAddress);

				for (sint32 x = 0; x < 8; ++x)
				{
					byte column = 7 - x;
					uint32 realX = x+currentColumnInTile*8;
					uint32 realY = y+currentLineInTile*8;
					byte whichAttribute = ((realX & 0x1F) > 0xF ? 1 : 0) + ((realY & 0x1F) > 0xF ? 2 : 0);

					paletteIndex.pixelTile = ((lowPatternTable >> column) & 0x1)
						| (((highPatternTable >> column) & 0x1) << 1);

					paletteIndex.paletteNumber = (attribute >> (whichAttribute * 2)) & 0x3;

					if ( !((paletteIndex.raw & 0x1F) & 0x3) )
					{
						paletteIndex.raw = 0;
					}

					nametableBuffer.setPixel(((whichNametable%2)*NametableWidth)+realX, ((whichNametable/2)*NametableHeight)+realY, _palette[ppu->_palette[paletteIndex.raw]]);
				}
			}

			nametableAddress++;
		
			currentColumnInTile++;
			if (currentColumnInTile >= NametableWidthInTile)
			{
				++currentLineInTile;
				currentColumnInTile = 0;
			}
		}

		nametableAddress += AttributeSize;
		attributeAddress += NametableSize;
	}

	ppu->_readBuffer = ppuReadBufferCopy;

	_ui.labelNametableView->setPixmap(QPixmap::fromImage(nametableBuffer));
}
