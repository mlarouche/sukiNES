#include "unrom_mapper.h"

#include "gamepak.h"

namespace sukiNES
{
	UnromMapper::UnromMapper(GamePak* gamepak)
		: Mapper(gamepak)
	{
	}

	void UnromMapper::write(word address, byte value)
	{
		gamepak()->changeBank(GamePak::Bank::LowerBank, value & 0xF);
	}
}