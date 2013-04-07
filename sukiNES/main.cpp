#include <cstdlib>
#include <iostream>

// For SDLMain on Windows
#ifdef SUKINES_PLATFORM_WINDOWS
#include <SDL.h>
#endif

int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cerr << "Video subsystem init failed: " << SDL_GetError() << std::endl;
		return 1;
	}

	Uint32 windowFlags = SDL_DOUBLEBUF | SDL_SWSURFACE;

	SDL_Surface* screen = SDL_SetVideoMode(256, 240, 32, windowFlags);

	if (screen == nullptr)
	{
		std::cerr << "SetVideoMode failed: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_WM_SetCaption("sukiNES 0.1", 0);

	while(1)
	{
		SDL_FillRect(screen, 0, SDL_MapRGBA(screen->format, 255, 255, 255, 255));

		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					exit(0);
					break;
				case SDL_KEYUP:
				{
					switch(event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							exit(0);
							break;
					}
					break;
				}
			}
		}

		SDL_Flip(screen);
	}

	return 0;
}