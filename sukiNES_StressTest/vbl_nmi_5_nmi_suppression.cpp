// Local includes
#include "blaggtestrombase.h"

static const char* RomFilename = "vbl_nmi_timing/5.nmi_suppression.nes";

class VblNmiTiming_NmiSuppression: public BlaggTestRomBase
{
public:
	VblNmiTiming_NmiSuppression()
	: BlaggTestRomBase()
	{
		setRomFilename(RomFilename);
		setFinalProgramCounter(0xE54C);
		setResultRamAddress(0xF8);
	}

	virtual void failureMessage()
	{
		switch (_result)
		{
		case 2:
			_generateFailureMessage("Reading flag 3 PPU clocks before set shouldn't suppress NMI");
			break;
		case 3:
			_generateFailureMessage("Reading flag when it's set should suppress NMI");
			break;
		case 4:
			_generateFailureMessage("Reading flag 3 PPU clocks after set shouldn't suppress NMI");
			break;
		case 5:
			_generateFailureMessage("Reading flag 2 PPU clocks before set shouldn't suppress NMI");
			break;
		case 6:
			_generateFailureMessage("Reading flag 1 PPU clock after set should suppress NMI");
			break;
		case 7:
			_generateFailureMessage("Reading flag 4 PPU clocks after set shouldn't suppress NMI");
			break;
		case 8:
			_generateFailureMessage("Reading flag 4 PPU clocks before set shouldn't suppress NMI");
			break;
		case 9:
			_generateFailureMessage("Reading flag 1 PPU clock before set should suppress NMI");
			break;
		case 10:
			_generateFailureMessage("Reading flag 2 PPU clocks after set shouldn't suppress NMI");
			break;
		default:
			break;
		}
	}
};

STRESSTEST_REGISTER_TEST(VblNmiTiming_NmiSuppression, vbl_nmi_timing_5_nmi_suppression);