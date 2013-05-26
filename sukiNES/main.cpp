#include <cstdlib>
#include <iostream>

// sukiNES includes
#include <cpu.h>
#include <gamepak.h>
#include <inputio.h>
#include <inesreader.h>
#include <mainmemory.h>
#include <ppu.h>
#include <ppuio.h>

// For SDLMain on Windows
#ifdef SUKINES_PLATFORM_WINDOWS
#include <SDL.h>
#endif

static const uint32 ScalingFactor = 3;
static const uint32 ScreenWidth = 256*ScalingFactor;
static const uint32 ScreenHeight = 240*ScalingFactor;

void PutPixel(SDL_Surface *surface, int x, int y, unsigned int pixel)
{
	int bpp = surface->format->BytesPerPixel;
	// Here p is the address to the pixel we want to set
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp)
	{
		case 1:
		  *p = pixel;
		  break;

		case 2:
		  *(Uint16 *)p = pixel;
		  break;

		case 3:
		{
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
			{
				p[0] = (pixel >> 16) & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = pixel & 0xff;
			}
			else
			{
				p[0] = pixel & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = (pixel >> 16) & 0xff;
			}
			break;
		}

		case 4:
			*(Uint32 *)p = pixel;
			break;
	}
}

const long int ntscpalette_pal_size = 192;
const unsigned char ntscpalette_pal[192] = {
    0x52, 0x52, 0x52, 0x01, 0x1A, 0x51, 0x0F, 0x0F, 0x65, 0x23, 0x06, 0x63, 0x36, 0x03, 0x4B, 0x40,
    0x04, 0x26, 0x3F, 0x09, 0x04, 0x32, 0x13, 0x00, 0x1F, 0x20, 0x00, 0x0B, 0x2A, 0x00, 0x00, 0x2F,
    0x00, 0x00, 0x2E, 0x0A, 0x00, 0x26, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xA0, 0xA0, 0xA0, 0x1E, 0x4A, 0x9D, 0x38, 0x37, 0xBC, 0x58, 0x28, 0xB8, 0x75, 0x21, 0x94, 0x84,
    0x23, 0x5C, 0x82, 0x2E, 0x24, 0x6F, 0x3F, 0x00, 0x51, 0x52, 0x00, 0x31, 0x63, 0x00, 0x1A, 0x6B,
    0x05, 0x0E, 0x69, 0x2E, 0x10, 0x5C, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFE, 0xFF, 0xFF, 0x69, 0x9E, 0xFC, 0x89, 0x87, 0xFF, 0xAE, 0x76, 0xFF, 0xCE, 0x6D, 0xF1, 0xE0,
    0x70, 0xB2, 0xDE, 0x7C, 0x70, 0xC8, 0x91, 0x3E, 0xA6, 0xA7, 0x25, 0x81, 0xBA, 0x28, 0x63, 0xC4,
    0x46, 0x54, 0xC1, 0x7D, 0x56, 0xB3, 0xC0, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFE, 0xFF, 0xFF, 0xBE, 0xD6, 0xFD, 0xCC, 0xCC, 0xFF, 0xDD, 0xC4, 0xFF, 0xEA, 0xC0, 0xF9, 0xF2,
    0xC1, 0xDF, 0xF1, 0xC7, 0xC2, 0xE8, 0xD0, 0xAA, 0xD9, 0xDA, 0x9D, 0xC9, 0xE2, 0x9E, 0xBC, 0xE6,
    0xAE, 0xB4, 0xE5, 0xC7, 0xB5, 0xDF, 0xE4, 0xA9, 0xA9, 0xA9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

class NesScreen : public sukiNES::PPUIO
{
public:
	NesScreen(SDL_Surface* screen)
	: _destination(nullptr)
	{
		_destination = screen;
		/*_destination = SDL_CreateRGBSurface(screen->flags,screen->w, screen->h, 32, 
			screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);*/

		for(uint32 i = 0; i<64; ++i)
		{
			byte r = ntscpalette_pal[(i * 3)];
			byte g = ntscpalette_pal[(i * 3) + 1];
			byte b = ntscpalette_pal[(i * 3) + 2];

			_palette[i] = SDL_MapRGB(screen->format, r, g, b);
		}
	}

	~NesScreen()
	{
		//SDL_FreeSurface(_destination);
	}

	virtual void putPixel(sint32 x, sint32 y, byte paletteIndex) override
	{
		if (ScalingFactor == 1)
		{
			PutPixel(_destination, x, y, _palette[paletteIndex]);
		}
		else
		{
			for(uint32 scaleY=0; scaleY<ScalingFactor; ++scaleY)
			{
				for (uint32 scaleX=0; scaleX<ScalingFactor; ++scaleX)
				{
					PutPixel(_destination, x*ScalingFactor+scaleX, y*ScalingFactor+scaleY, _palette[paletteIndex]);
				}
			}
		}
	}

	virtual void onVBlank() override
	{
		SDL_Flip(_destination);
	}

private:
	SDL_Surface* _destination;
	Uint32 _palette[64];
};

class NesInput : public sukiNES::InputIO
{
public:
	NesInput()
	{
		for(uint32 whichController=0; whichController<2; ++whichController)
		{
			buttonStatus[whichController].raw = 0;
		}
	}

	virtual byte inputStatus(byte controller) const override
	{
		return buttonStatus[controller].raw;
	}

	union
	{
		byte raw;
		RegBit<0> A;
		RegBit<1> B;
		RegBit<2> Select;
		RegBit<3> Start;
		RegBit<4> Up;
		RegBit<5> Down;
		RegBit<6> Left;
		RegBit<7> Right;
	} buttonStatus[2];
};

class EmulatorRunner
{
public:
	EmulatorRunner()
	{
		// Setup MainMemory
		_memory.setGamepakMemory(&_gamePak);
		_memory.setPpuMemory(&_ppu);

		// Setup CPU
		_cpu.setMainMemory(&_memory);
		_cpu.setPPU(&_ppu);

		// Setup PPU
		_ppu.setGamePak(&_gamePak);
	}

	~EmulatorRunner()
	{
	}

	void powerOn()
	{
		_cpu.powerOn();
	}

	void reset()
	{
		_cpu.reset();
	}

	bool readROM(const char* filename)
	{
		sukiNES::iNESReader nesReader;
		nesReader.setGamePak(&_gamePak);
		nesReader.setPpu(&_ppu);

		if (!nesReader.read(filename))
		{
			return false;
		}

		return true;
	}

	void setPPUIO(sukiNES::PPUIO* io)
	{
		_ppu.setIO(io);
	}

	void setInputIO(sukiNES::InputIO* io)
	{
		_cpu.setInputIO(io);
	}

	void runSingleInstruction()
	{
		_cpu.executeOpcode();
	}

private:
	sukiNES::Cpu _cpu;
	sukiNES::MainMemory _memory;
	sukiNES::GamePak _gamePak;
	sukiNES::PPU _ppu;
};

//static const char* RomFilename = "Donkey Kong (World) (Rev A).nes";
//static const char* RomFilename = "Excitebike (Japan, USA).nes";
static const char* RomFilename = "Super Mario Bros. (World).nes";
//static const char* RomFilename = "Ice Climber (USA, Europe).nes";
//static const char* RomFilename = "Balloon Fight (USA).nes";
//static const char* RomFilename = "NEStress.NES";

int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cerr << "Video subsystem init failed: " << SDL_GetError() << std::endl;
		return 1;
	}

	Uint32 windowFlags = SDL_DOUBLEBUF | SDL_SWSURFACE;

	SDL_Surface* screen = SDL_SetVideoMode(ScreenWidth, ScreenHeight, 32, windowFlags);

	if (screen == nullptr)
	{
		std::cerr << "SetVideoMode failed: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_WM_SetCaption("sukiNES 0.1", 0);

	NesScreen emuScreen(screen);
	NesInput input;

	EmulatorRunner emuRunner;
	emuRunner.setPPUIO(&emuScreen);
	emuRunner.setInputIO(&input);

	if (!emuRunner.readROM(RomFilename))
	{
		std::cerr << "Cannot read ROM !" << std::endl;
		return 1;
	}

	emuRunner.powerOn();

	while(1)
	{
		emuRunner.runSingleInstruction();

		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					exit(0);
					break;
				case SDL_KEYDOWN:
				{
					switch(event.key.keysym.sym)
					{
						case SDLK_z:
							input.buttonStatus[0].A = true;
							break;
						case SDLK_x:
							input.buttonStatus[0].B = true;
							break;
						case SDLK_LCTRL:
							input.buttonStatus[0].Start = true;
							break;
						case SDLK_LALT:
							input.buttonStatus[0].Select = true;
							break;
						case SDLK_UP:
							input.buttonStatus[0].Up = true;
							break;
						case SDLK_DOWN:
							input.buttonStatus[0].Down = true;
							break;
						case SDLK_LEFT:
							input.buttonStatus[0].Left = true;
							break;
						case SDLK_RIGHT:
							input.buttonStatus[0].Right = true;
							break;
					}
					break;
				}
				case SDL_KEYUP:
				{
					switch(event.key.keysym.sym)
					{
						case SDLK_z:
							input.buttonStatus[0].A = false;
							break;
						case SDLK_x:
							input.buttonStatus[0].B = false;
							break;
						case SDLK_LCTRL:
							input.buttonStatus[0].Start = false;
							break;
						case SDLK_LALT:
							input.buttonStatus[0].Select = false;
							break;
						case SDLK_UP:
							input.buttonStatus[0].Up = false;
							break;
						case SDLK_DOWN:
							input.buttonStatus[0].Down = false;
							break;
						case SDLK_LEFT:
							input.buttonStatus[0].Left = false;
							break;
						case SDLK_RIGHT:
							input.buttonStatus[0].Right = false;
							break;
						case SDLK_ESCAPE:
							exit(0);
							break;
					}
					break;
				}
			}
		}
	}

	return 0;
}