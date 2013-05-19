// Local includes
#include "blaggtestrombase.h"

static const char* RomFilename = "blargg_ppu_tests/vbl_clear_time.nes";

class BlaggVblClearTime : public BlaggTestRomBase
{
public:
	BlaggVblClearTime()
	: BlaggTestRomBase()
	{
		setRomFilename(RomFilename);
		setFinalProgramCounter(0xE3B3);
		setResultRamAddress(0xF0);
	}

	virtual void failureMessage()
	{
		switch (_result)
		{
		case 2:
			_generateFailureMessage("VBL flag cleared too soon");
			break;
		case 3:
			_generateFailureMessage("VBL flag cleared too late");
			break;
		default:
			break;
		}
	}
};

// TODO: Renable when we got a proper debugger
//STRESSTEST_REGISTER_TEST(BlaggVblClearTime, blagg_vbl_clear_time);