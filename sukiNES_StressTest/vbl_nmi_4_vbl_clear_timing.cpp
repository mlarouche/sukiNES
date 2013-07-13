// Local includes
#include "blaggtestrombase.h"

static const char* RomFilename = "vbl_nmi_timing/4.vbl_clear_timing.nes";

class VblNmiTiming_VblClearTiming: public BlaggTestRomBase
{
public:
	VblNmiTiming_VblClearTiming()
	: BlaggTestRomBase()
	{
		setRomFilename(RomFilename);
		setFinalProgramCounter(0xE535);
		setResultRamAddress(0xF8);
	}

	virtual void failureMessage()
	{
		switch (_result)
		{
		case 2:
			_generateFailureMessage("Cleared 3 or more PPU clocks too early");
			break;
		case 3:
			_generateFailureMessage("Cleared 2 PPU clocks too early");
			break;
		case 4:
			_generateFailureMessage("Cleared 1 PPU clock too early ");
			break;
		case 5:
			_generateFailureMessage("Cleared 3 or more PPU clocks too late");
			break;
		case 6:
			_generateFailureMessage("Cleared 2 PPU clocks too late");
			break;
		case 7:
			_generateFailureMessage("Cleared 1 PPU clock too late");
			break;
		default:
			break;
		}
	}
};

STRESSTEST_REGISTER_TEST(VblNmiTiming_VblClearTiming, vbl_nmi_timing_4_vbl_clear_timing);