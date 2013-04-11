#include "mainmemory.h"

// Local includes
#include "assert.h"

namespace sukiNES
{
	static const uint16 RamSize = SUKINES_KB(2);
	static const uint32 SramBaseAddress = 0x6000;

	MainMemory::MainMemory()
	: _ppuMemory(nullptr)
	, _apuMemory(nullptr)
	, _inputMemory(nullptr)
	, _gamepakMemory(nullptr)
	{
		memset(_ram, 0, RamSize);
		memset(_sram, 0, RamSize);
	}

	MainMemory::~MainMemory()
	{
	}

	byte MainMemory::read(word address)
	{
		if (address < 0x2000)
		{
			return _ram[static_cast<uint32>(address) % RamSize];
		}
		else if (address >= 0x2000 && address < 0x4000)
		{
			sukiAssertWithMessage(_ppuMemory, "Please set the PPU memory access in main memory");
			return _ppuMemory->read(address);
		}
		else if (address >= 0x4000 && address < 0x4016)
		{
			sukiAssertWithMessage(_apuMemory, "Please set the APU memory access in main memory");
			return _apuMemory->read(address);
		}
		else if (address >= 0x4016 && address < 0x4016)
		{
			sukiAssertWithMessage(_inputMemory, "Please set the Input memory access in main memory");
			return _inputMemory->read(address);
		}
		else if (address >= 0x4020 && address < 0x6000)
		{
			// Expansion ROM (do nothing)
		}
		else if (address >= 0x6000 && address < 0x8000)
		{
			return _sram[static_cast<uint32>(address) % SramBaseAddress];
		}
		else
		{
			sukiAssertWithMessage(_gamepakMemory, "Please set the GamePak memory access in main memory");
			return _gamepakMemory->read(address);
		}

		return 0;
	}

	void MainMemory::write(word address, byte value)
	{
		if (address < 0x2000)
		{
			_ram[static_cast<uint32>(address) % RamSize] = value;
		}
		else if (address >= 0x2000 && address < 0x4000)
		{
			sukiAssertWithMessage(_ppuMemory, "Please set the PPU memory access in main memory");
			_ppuMemory->write(address, value);
		}
		else if (address >= 0x4000 && address < 0x4016)
		{
			sukiAssertWithMessage(_apuMemory, "Please set the APU memory access in main memory");
			_apuMemory->write(address, value);
		}
		else if (address >= 0x4016 && address < 0x4016)
		{
			sukiAssertWithMessage(_inputMemory, "Please set the Input memory access in main memory");
			_inputMemory->write(address, value);
		}
		else if (address >= 0x4020 && address < 0x6000)
		{
			// Expansion ROM (do nothing)
		}
		else if (address >= 0x6000 && address < 0x8000)
		{
			_sram[static_cast<uint32>(address) % SramBaseAddress]= value;
		}
		else
		{
			sukiAssertWithMessage(_gamepakMemory, "Please set the GamePak memory access in main memory");
			_gamepakMemory->write(address, value);
		}
	}
}