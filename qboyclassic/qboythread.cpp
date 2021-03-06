#include "qboythread.h"

#include <fstream>
#include <cassert>
#include <QElapsedTimer>
#include <QFile>

qboythread::qboythread(QString filename, QObject *parent) : QThread(parent) {
	dorun = true;
	sloweddown = true;
	qboy = new libqboy();

	QFile file(filename);
	if (!file.exists()) {
		assert(false && "Could not find file");
	}
	qboy->loadgame(filename.toStdString());
}

qboythread::~qboythread() {
	stop();
	while (isRunning());
	delete qboy;
}

quint8* qboythread::getLCD() {
	return qboy->getLCD();
}

void qboythread::togglespeed() {
	sloweddown = !sloweddown;
}

void qboythread::run() {
	dorun = true;
	QElapsedTimer timer;
	timer.start();

	int thirds = 0;
	while (dorun) {
		qboy->cycle();
		if (qboy->refresh_screen()) {
			int s = timer.elapsed();
			timer.restart();
			emit screen_refresh();
			if (thirds++ == 3) {
				thirds = 0;
				s--;
			}
			if (sloweddown && 16 - s > 0) msleep(16 - s);
		}
	}
}

void qboythread::stop() {
	dorun = false;
}

void qboythread::keydown(GBKeypadKey key) {
	qboy->keydown(key);
}

void qboythread::keyup(GBKeypadKey key) {
	qboy->keyup(key);
}
