#pragma once

namespace sukiNES
{
	class GamePak;

	class iNESReader
	{
	public:
		iNESReader();
		~iNESReader();

		bool read(const char* filename);

		void setGamePak(GamePak* gamepak)
		{
			_gamePak = gamepak;
		}

	private:
		GamePak* _gamePak;
	};
}
