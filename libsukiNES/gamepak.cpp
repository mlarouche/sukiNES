#include "gamepak.h"

// Local includes
#include "mapper.h"

namespace sukiNES
{
	static const uint32 GamePakBaseAddress = 0x8000;

	GamePak::GamePak()
	: _chrData(ChrBankSize)
	, _chrBank(nullptr)
	, _mirroring(0)
	, _hasSaveRam(false)
	, _mapperNumber(0)
	, _mapper(nullptr)
	{
		_chrBank = _chrData.get();
		memset(_chrBank, 0, ChrBankSize);

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
			_romBank[1] = _romData.get() + ((romPageCount()-1) * RomBankSize);
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
		if (_mapper)
		{
			_mapper->write(address, value);
		}
	}

	byte GamePak::readChr(word address)
	{
		return _chrBank[address];
	}

	void GamePak::writeChr(word address, byte value)
	{
		_chrBank[address] = value;
	}

	void GamePak::changeBank(Bank whichBank, byte value)
	{
		_romBank[static_cast<size_t>(whichBank)] = _romData.get() + (value*RomBankSize);
	}
}
