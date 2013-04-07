#include "gamepak.h"

namespace sukiNES
{
	static const uint32 GamePakBaseAddress = 0x8000;
	GamePak::GamePak()
	: _chrBank(nullptr)
	, _mirroring(MirroringType::Horizontal)
	, _hasSaveRam(false)
	{
		_romBank[0] = nullptr;
		_romBank[1] = nullptr;
	}

	GamePak::~GamePak()
	{
	}

	void GamePak::setRomData(DynamicArray<byte>&& romData)
	{
		_romData = std::forward<DynamicArray<byte>>(romData);

		if ( (romData.size() / RomBankSize) == 1)
		{
			_romBank[0] = romData.get();
			_romBank[1] = romData.get();
		}
		else
		{
			_romBank[0] = romData.get();
			_romBank[1] = romData.get() + RomBankSize;
		}
	}

	byte GamePak::read(word address)
	{
		return 0;
	}

	void GamePak::write(word address, byte value)
	{
	}
}
