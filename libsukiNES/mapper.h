#pragma once

namespace sukiNES
{
	class GamePak;

	class Mapper
	{
	public:
		Mapper(GamePak* gamepak);

		virtual void write(word address, byte value) = 0;

	protected:
		GamePak* gamepak() const
		{
			return _gamePak;
		}

	private:
		GamePak* _gamePak;
	};
}