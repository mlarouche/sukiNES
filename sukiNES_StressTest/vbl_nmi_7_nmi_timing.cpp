// Local includes
#include "blaggtestrombase.h"

static const char* RomFilename = "vbl_nmi_timing/7.nmi_timing.nes";

class VblNmiTiming_NmiTiming: public BlaggTestRomBase
{
public:
	VblNmiTiming_NmiTiming()
	: BlaggTestRomBase()
	{
		setRomFilename(RomFilename);
		setFinalProgramCounter(0xE58E);
		setResultRamAddress(0xF8);
	}

	virtual void failureMessage()
	{
		switch (_result)
		{
		case 2:
			_generateFailureMessage("NMI occurred 3 or more PPU clocks too early");
			break;
		case 3:
			_generateFailureMessage("NMI occurred 2 PPU clocks too early");
			break;
		case 4:
			_generateFailureMessage("NMI occurred 1 PPU clock too early");
			break;
		case 5:
			_generateFailureMessage("NMI occurred 3 or more PPU clocks too late");
			break;
		case 6:
			_generateFailureMessage("NMI occurred 2 PPU clocks too late");
			break;
		case 7:
			_generateFailureMessage("NMI occurred 1 PPU clock too late");
			break;
		case 8:
			_generateFailureMessage("NMI should occur if enabled when VBL already set");
			break;
		case 9:
			_generateFailureMessage("NMI enabled when VBL already set should delay 1 instruction");
			break;
		case 10:
			_generateFailureMessage("NMI should be possible multiple times in VBL");
			break;
		default:
			break;
		}
	}
};

STRESSTEST_REGISTER_TEST(VblNmiTiming_NmiTiming, vbl_nmi_timing_7_nmi_timing);