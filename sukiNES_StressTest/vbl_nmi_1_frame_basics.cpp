// Local includes
#include "blaggtestrombase.h"

static const char* RomFilename = "vbl_nmi_timing/1.frame_basics.nes";

class VblNmiTiming_FrameBasics : public BlaggTestRomBase
{
public:
	VblNmiTiming_FrameBasics()
	: BlaggTestRomBase()
	{
		setRomFilename(RomFilename);
		setFinalProgramCounter(0xE589);
		setResultRamAddress(0xF8);
	}

	virtual void failureMessage()
	{
		switch (_result)
		{
		case 2:
			_generateFailureMessage("VBL flag isn't being set");
			break;
		case 3:
			_generateFailureMessage("VBL flag should be cleared after being read");
			break;
		case 4:
			_generateFailureMessage("PPU frame with BG enabled is too short");
			break;
		case 5:
			_generateFailureMessage("PPU frame with BG enabled is too long");
			break;
		case 6:
			_generateFailureMessage("PPU frame with BG disabled is too short");
			break;
		case 7:
			_generateFailureMessage("PPU frame with BG disabled is too long");
			break;
		default:
			break;
		}
	}
};

STRESSTEST_REGISTER_TEST(VblNmiTiming_FrameBasics, vbl_nmi_timing_1_frame_basics);