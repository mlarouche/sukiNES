// STL includes
#include <iostream>
#include <cstdlib>

// sukiNES includes
#include <cpu.h>

void showpause()
{
	system("pause");
}

int main(int argc, char** argv)
{
	atexit(showpause);

	sukiNES::Cpu cpu;

	if (cpu.executeOpcode() == 6)
	{
		std::cout << "Success !" << std::endl;
		return 0;
	}

	std::cerr << "FAILURE !" << std::endl;

	return 1;
}