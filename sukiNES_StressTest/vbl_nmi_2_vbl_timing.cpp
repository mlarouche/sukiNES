// Local includes
#include "blaggtestrombase.h"

static const char* RomFilename = "vbl_nmi_timing/2.vbl_timing.nes";

class VblNmiTiming_VblTiming: public BlaggTestRomBase
{
public:
	VblNmiTiming_VblTiming()
	: BlaggTestRomBase()
	{
		setRomFilename(RomFilename);
		setFinalProgramCounter(0xE54F);
		setResultRamAddress(0xF8);
	}

	virtual void failureMessage()
	{
		switch (_result)
		{
		case 2:
			_generateFailureMessage("Flag should read as clear 3 PPU clocks before VBL");
			break;
		case 3:
			_generateFailureMessage("Flag should read as set 0 PPU clocks after VBL");
			break;
		case 4:
			_generateFailureMessage("Flag should read as clear 2 PPU clocks before VBL");
			break;
		case 5:
			_generateFailureMessage("Flag should read as set 1 PPU clock after VBL");
			break;
		case 6:
			_generateFailureMessage("Flag should read as clear 1 PPU clock before VBL");
			break;
		case 7:
			_generateFailureMessage("Flag should read as set 2 PPU clocks after VBL");
			break;
		case 8:
			_generateFailureMessage("Reading 1 PPU clock before VBL should suppress setting");
			break;
		default:
			break;
		}
	}
};

STRESSTEST_REGISTER_TEST(VblNmiTiming_VblTiming, vbl_nmi_timing_2_vbl_timing);