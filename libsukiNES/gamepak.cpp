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
			_romBank[0] = _romData.get();
			_romBank[1] = _romData.get();
		}
		else
		{
			_romBank[0] = _romData.get();
			_romBank[1] = _romData.get() + RomBankSize;
		}
	}

	byte GamePak::read(word address)
	{
		uint32 relativeAddress = static_cast<uint32>(address) % GamePakBaseAddress;

		uint32 bankToUse = (relativeAddress >= RomBankSize) ? 1 : 0;

		return _romBank[bankToUse][relativeAddress % RomBankSize];
	}

	void GamePak::write(word address, byte value)
	{
	}
}
