// sukiNES includes
#include <cpu.h>
#include <gamepak.h>
#include <inesreader.h>
#include <mainmemory.h>
#include <ppu.h>

// Local includes
#include "test.h"

static const char* RomFilename = "blargg_ppu_tests/palette_ram.nes";

class BlaggPaletteRam : public StressTest::Test
{
public:
	BlaggPaletteRam()
	: _result(0)
	{
		// Setup MainMemory
		_memory.setGamepakMemory(&_gamePak);
		_memory.setPpuMemory(&_ppu);

		// Setup memory in CPU
		_cpu.setMainMemory(&_memory);
		_cpu.setPPU(&_ppu);
	}

	bool run()
	{
		sukiNES::iNESReader nesReader;
		nesReader.setGamePak(&_gamePak);

		if (!nesReader.read(RomFilename))
		{
			_generateFailureMessage("Cannot open NES file %s", RomFilename);
			return false;
		}

		_cpu.powerOn();

		while((int)_cpu.programCounter() != 0xE412)
		{
			_cpu.executeOpcode();
		}

		_result = _memory.read(0xF0);

		if (_result != 1)
		{
			switch (_result)
			{
			case 2:
				_generateFailureMessage("Palette read shouldn't be buffered like other VRAM");
				break;
			case 3:
				_generateFailureMessage("Palette write/read doesn't work");
				break;
			case 4:
				_generateFailureMessage("Palette should be mirrored within $3f00-$3fff");
				break;
			case 5:
				_generateFailureMessage("Write to $10 should be mirrored at $00");
				break;
			case 6:
				_generateFailureMessage("Write to $00 should be mirrored at $10");
				break;
			default:
				break;
			}
			return false;
		}

		return true;
	}

private:
	sukiNES::Cpu _cpu;
	sukiNES::MainMemory _memory;
	sukiNES::GamePak _gamePak;
	sukiNES::PPU _ppu;

	byte _result;
};

STRESSTEST_REGISTER_TEST(BlaggPaletteRam, blagg_palette_ram);