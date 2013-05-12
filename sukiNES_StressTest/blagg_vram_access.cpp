#include "blaggtestrombase.h"

static const char* RomFilename= "blargg_ppu_tests/vram_access.nes";

class BlaggVramAccess : public BlaggTestRomBase
{
public:
	BlaggVramAccess()
	: BlaggTestRomBase()
	{
		setRomFilename(RomFilename);
		setFinalProgramCounter(0xE48D);
		setResultRamAddress(0xF0);
	}

	virtual void failureMessage()
	{
		switch (_result)
		{
		case 2:
			_generateFailureMessage("VRAM reads should be delayed in a buffer");
			break;
		case 3:
			_generateFailureMessage("Basic Write/read doesn't work");
			break;
		case 4:
			_generateFailureMessage("Read buffer shouldn't be affected by VRAM write");
			break;
		case 5:
			_generateFailureMessage("Read buffer shouldn't be affected by palette write");
			break;
		case 6:
			_generateFailureMessage("Palette read should also read VRAM into read buffer");
			break;
		case 7:
			_generateFailureMessage("\"Shadow\" VRAM read unaffected by palette transparent color mirroring");
			break;
		default:
			break;
		}
	}
};

STRESSTEST_REGISTER_TEST(BlaggVramAccess, blagg_vram_access);