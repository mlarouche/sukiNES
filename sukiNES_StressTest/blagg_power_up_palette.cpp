// Local includes
#include "blaggtestrombase.h"

static const char* RomFilename = "blargg_ppu_tests/power_up_palette.nes";

class BlaggPalettePowerUp : public BlaggTestRomBase
{
public:
	BlaggPalettePowerUp()
	: BlaggTestRomBase()
	{
		setRomFilename(RomFilename);
		setFinalProgramCounter(0xE3AC);
		setResultRamAddress(0xF0);
	}

	virtual void failureMessage()
	{
		switch (_result)
		{
		case 2:
			_generateFailureMessage("Palette differs from table");
			break;
		default:
			break;
		}
	}
};

STRESSTEST_REGISTER_TEST(BlaggPalettePowerUp, blagg_power_up_palette);