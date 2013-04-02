#ifndef LIBQBOY_H
#define LIBQBOY_H

#include "libqboy_global.h"

#include "z80.h"
#include "z80mmu.h"
#include "gbgpu.h"
#include "gbkeypad.h"
#include "z80timer.h"

#include <string>

class LIBQBOYSHARED_EXPORT libqboy {
public:
	libqboy();
	void reset();
	quint8 *getLCD();
	void loadgame(std::string filename);
	void cycle();
	int get_elapsed_time();
	void keyup(GBKeypadKey key);
	void keydown(GBKeypadKey key);
	void save();
private:
	z80 cpu;
	z80mmu mmu;
	z80timer timer;
	gbgpu gpu;
	gbkeypad keypad;

	int passed_mtime;
};

#endif // LIBQBOY_H
