// Local includes
#include "blaggtestrombase.h"

static const char* RomFilename = "vbl_nmi_timing/3.even_odd_frames.nes";

class VblNmiTiming_EvenOddFrame: public BlaggTestRomBase
{
public:
	VblNmiTiming_EvenOddFrame()
	: BlaggTestRomBase()
	{
		setRomFilename(RomFilename);
		setFinalProgramCounter(0xE59F);
		setResultRamAddress(0xF8);
	}

	virtual void failureMessage()
	{
		switch (_result)
		{
		case 2:
			_generateFailureMessage("Pattern ----- should not skip any clocks");
			break;
		case 3:
			_generateFailureMessage("Pattern BB--- should skip 1 clock");
			break;
		case 4:
			_generateFailureMessage("Pattern B--B- (one even, one odd) should skip 1 clock");
			break;
		case 5:
			_generateFailureMessage("Pattern -B--B (one odd, one even) should skip 1 clock");
			break;
		case 6:
			_generateFailureMessage("Pattern BB-BB (two pairs) should skip 2 clocks");
			break;
		default:
			break;
		}
	}
};

STRESSTEST_REGISTER_TEST(VblNmiTiming_EvenOddFrame, vbl_nmi_timing_3_even_odd_frames);