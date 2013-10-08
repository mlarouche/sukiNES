#pragma once

#include "mapper.h"

namespace sukiNES
{
	class UnromMapper : public Mapper
	{
	public:
		UnromMapper(GamePak* gamepak);

		virtual void write(word address, byte value) override;
	};
}