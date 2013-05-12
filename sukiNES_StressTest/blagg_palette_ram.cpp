// Local includes
#include "blaggtestrombase.h"

static const char* RomFilename = "blargg_ppu_tests/palette_ram.nes";

class BlaggPaletteRam : public BlaggTestRomBase
{
public:
	BlaggPaletteRam()
	: BlaggTestRomBase()
	{
		setRomFilename(RomFilename);
		setFinalProgramCounter(0xE412);
		setResultRamAddress(0xF0);
	}

	virtual void failureMessage()
	{
		switch (_result)
		{
		case 2:
			_generateFailureMessage("Palette read shouldn't be buffered like other VRAM");
			break;
		case 3:
			_generateFailureMessage("Palette write/read doesn't work");
			break;
		case 4:
			_generateFailureMessage("Palette should be mirrored within $3f00-$3fff");
			break;
		case 5:
			_generateFailureMessage("Write to $10 should be mirrored at $00");
			break;
		case 6:
			_generateFailureMessage("Write to $00 should be mirrored at $10");
			break;
		default:
			break;
		}
	}
};

STRESSTEST_REGISTER_TEST(BlaggPaletteRam, blagg_palette_ram);