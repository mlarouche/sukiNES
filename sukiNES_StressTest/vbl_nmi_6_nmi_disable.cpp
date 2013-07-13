// Local includes
#include "blaggtestrombase.h"

static const char* RomFilename = "vbl_nmi_timing/6.nmi_disable.nes";

class VblNmiTiming_NmiDisable: public BlaggTestRomBase
{
public:
	VblNmiTiming_NmiDisable()
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
			_generateFailureMessage("NMI shouldn't occur when disabled 0 PPU clocks after VBL");
			break;
		case 3:
			_generateFailureMessage("NMI should occur when disabled 3 PPU clocks after VBL");
			break;
		case 4:
			_generateFailureMessage("NMI shouldn't occur when disabled 1 PPU clock after VBL");
			break;
		case 5:
			_generateFailureMessage("NMI should occur when disabled 4 PPU clocks after VBL");
			break;
		case 6:
			_generateFailureMessage("NMI shouldn't occur when disabled 1 PPU clock before VBL");
			break;
		case 7:
			_generateFailureMessage("NMI should occur when disabled 2 PPU clocks after VBL");
			break;
		default:
			break;
		}
	}
};

STRESSTEST_REGISTER_TEST(VblNmiTiming_NmiDisable, vbl_nmi_timing_6_nmi_disable);