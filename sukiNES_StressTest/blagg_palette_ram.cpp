#include "test.h"

class BlaggPaletteRam : public StressTest::Test
{
public:
	bool run()
	{
		return true;
	}
};

STRESSTEST_REGISTER_TEST(BlaggPaletteRam, blagg_palette_ram);