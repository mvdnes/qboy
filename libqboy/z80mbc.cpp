#include "z80mbc.h"

#include <fstream>

z80mbc0::z80mbc0(const std::vector<quint8> &rom) {
	this->rom = rom;
	ram.resize(0x2000);
}

quint8 z80mbc0::readROM(quint16 address) {
	return rom[address];
}

quint8 z80mbc0::readRAM(quint16 address) {
	return ram[address];
}

void z80mbc0::writeROM(quint16, quint8) {}

void z80mbc0::writeRAM(quint16 address, quint8 value) {
	ram[address] = value;
}

/********************************/

z80mbc1::z80mbc1(const std::vector<quint8> &rom) {
	this->rom = rom;
	ram.resize(0x10000);
	rombank = 1;
	rambank = 0;
	extram_on = false;
	ram_mode = false;
}

quint8 z80mbc1::readROM(quint16 address) {
	if (address < 0x4000) return rom[address];
	address &= 0x3FFF;
	int the_rombank = (ram_mode) ? rombank & 0x1F : rombank;
	return rom[the_rombank * 0x4000 | address];
}

quint8 z80mbc1::readRAM(quint16 address) {
	if (!extram_on || !ram_mode) return 0;
	address &= 0x1FFF;
	return ram[rambank * 0x2000 | address];
}

void z80mbc1::writeROM(quint16 address, quint8 value) {
	switch (address & 0xF000) {
	case 0x0000:
	case 0x1000:
		extram_on = (value == 0x0A);
		break;
	case 0x2000:
	case 0x3000:
		value &= 0x1F;
		if (value == 0) value = 1;
		rombank = (rombank & 0x60) | value;
		break;
	case 0x4000:
	case 0x5000:
		value &= 0x03;
		if (ram_mode) {
			rambank = value;
		} else {
			rombank = (value << 5) | (rombank & 0x1F);
		}
		break;
	case 0x6000:
	case 0x7000:
		ram_mode = value & 1;
		break;
	}
}

void z80mbc1::writeRAM(quint16 address, quint8 value) {
	if (!extram_on || !ram_mode) return;
	if (!ram_mode) rambank = 0;
	address &= 0x1FFF;
	ram[rambank * 0x2000 | address] = value;
}

/********************************/

z80mbc3::z80mbc3(const std::vector<quint8> &rom) {
	rtczero = time(NULL);
	this->rom = rom;
	ram.resize(0x10000);
	rtc.resize(5);
	rombank = 1;
	rambank = 0;
	extram_on = false;
}

quint8 z80mbc3::readROM(quint16 address) {
	if (address < 0x4000) return rom[address];
	address &= 0x3FFF;
	return rom[rombank * 0x4000 | address];
}

quint8 z80mbc3::readRAM(quint16 address) {
	if (!extram_on) return 0;
	if (rambank <= 3) {
		address &= 0x1FFF;
		return ram[rambank * 0x2000 | address];
	} else {
		calc_rtcregs();
		return rtc[rambank - 0x08];
		return 0;
	}
}

void z80mbc3::writeROM(quint16 address, quint8 value) {
	switch (address & 0xF000) {
	case 0x0000:
	case 0x1000:
		extram_on = (value == 0x0A);
		break;
	case 0x2000:
	case 0x3000:
		value &= 0x7F;
		rombank = (value == 0) ? 1 : value;
		break;
	case 0x4000:
	case 0x5000:
		rambank = value;
		break;
	case 0x6000:
	case 0x7000:
		// TODO: Lock RTC...
		break;
	}
}

void z80mbc3::writeRAM(quint16 address, quint8 value) {
	if (!extram_on) return;

	if (rambank <= 3) {
		address &= 0x1FFF;
		ram[rambank * 0x2000 | address] = value;
	} else {
		rtc[rambank - 0x8] = value;
		calc_rtczero();
	}
}

void z80mbc3::save(std::string filename) {
	std::ofstream fout(filename.c_str(), std::ios_base::out | std::ios_base::binary);
	fout.write(reinterpret_cast<const char *>(&rtczero), sizeof(rtczero));
	for (unsigned i = 0; i < ram.size(); ++i) {
		fout.write((char*)&ram[i], 1);
	}
	fout.close();
}

void z80mbc3::load(std::string filename) {
	std::ifstream fin(filename.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!fin.is_open()) return;

	char byte;
	ram.clear();

	fin.read(reinterpret_cast<char *>(&rtczero), sizeof(rtczero));
	while (fin.read(&byte, 1)) {
		ram.push_back(byte);
	}
	fin.close();
}

void z80mbc3::calc_rtczero() {
	time_t difftime = time(NULL);
	long long days;
	difftime -= rtc[0];
	difftime -= rtc[1] * 60;
	difftime -= rtc[2] * 3600;
	days = rtc[4] & 0x1;
	days = days << 8 | rtc[3];
	difftime -= days * 3600 * 24;
	rtczero = difftime;
}

void z80mbc3::calc_rtcregs() {
	time_t difftime = time(NULL) - rtczero;
	rtc[0] = difftime % 60;
	rtc[1] = (difftime / 60) % 60;
	rtc[2] = (difftime / 3600) % 24;
	long long days = (difftime / (3600*24));
	rtc[3] = days & 0xFF;
	rtc[4] = (rtc[4] & 0xFE) | ((days >> 8) & 0x1);
}

void z80mbc3::calc_halttime() {

}

/********************************/

z80mbc5::z80mbc5(const std::vector<quint8> &rom) {
	this->rom = rom;
	ram.resize(0x20000);
	rombank = 1;
	rambank = 0;
	extram_on = false;
}

quint8 z80mbc5::readROM(quint16 address) {
	if (address < 0x4000) return rom[address];
	address &= 0x3FFF;

	return rom[rombank * 0x4000 | address];
}

quint8 z80mbc5::readRAM(quint16 address) {
	if (!extram_on) return 0;

	address &= 0x1FFF;
	return ram[rambank * 0x2000 | address];
}

void z80mbc5::writeROM(quint16 address, quint8 value) {
	switch (address & 0xF000) {
	case 0x0000:
	case 0x1000:
		extram_on = (value == 0x0A);
		break;
	case 0x2000:
		rombank = (rombank & 0x100) | (value);
		break;
	case 0x3000:
		value &= 0x1;
		rombank = (rombank & 0xFF) | (((int)value) << 8);
		break;
	case 0x4000:
	case 0x5000:
		rambank = value & 0x0F;
		break;
	}
}

void z80mbc5::writeRAM(quint16 address, quint8 value) {
	if (!extram_on) return;

	address &= 0x1FFF;
	ram[rambank * 0x2000 | address] = value;
}

void z80mbc5::save(std::string filename) {
	std::ofstream fout(filename.c_str(), std::ios_base::out | std::ios_base::binary);
	for (unsigned i = 0; i < ram.size(); ++i) {
		fout.write((char*)&ram[i], 1);
	}
	fout.close();
}

void z80mbc5::load(std::string filename) {
	std::ifstream fin(filename.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!fin.is_open()) return;

	char byte;
	ram.clear();
	while (fin.read(&byte, 1)) {
		ram.push_back(byte);
	}
	fin.close();
}
