// Local includes
#include "blaggtestrombase.h"

static const char* RomFilename = "blargg_ppu_tests/sprite_ram.nes";

class BlaggSpriteRam : public BlaggTestRomBase
{
public:
	BlaggSpriteRam()
	: BlaggTestRomBase()
	{
		setRomFilename(RomFilename);
		setFinalProgramCounter(0xE467);
		setResultRamAddress(0xF0);
	}

	virtual void failureMessage()
	{
		switch (_result)
		{
		case 2:
			_generateFailureMessage("Basic read/write doesn't work");
			break;
		case 3:
			_generateFailureMessage("Address should increment on $2004 write");
			break;
		case 4:
			_generateFailureMessage("Address should not increment on $2004 read");
			break;
		case 5:
			_generateFailureMessage("Third sprite bytes should be masked with $e3 on read ");
			break;
		case 6:
			_generateFailureMessage("$4014 DMA copy doesn't work at all");
			break;
		case 7:
			_generateFailureMessage("$4014 DMA copy should start at value in $2003 and wrap");
			break;
		case 8:
			_generateFailureMessage("$4014 DMA copy should leave value in $2003 intact");
			break;
		default:
			break;
		}
	}
};

STRESSTEST_REGISTER_TEST(BlaggSpriteRam, blagg_sprite_ram);