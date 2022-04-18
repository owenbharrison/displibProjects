#include "Stopwatch.h"

Stopwatch::Stopwatch() {}

std::chrono::steady_clock::time_point Stopwatch::getTime() {
	return std::chrono::high_resolution_clock::now();
}

void Stopwatch::start() {
	this->startTime=this->getTime();
}

void Stopwatch::stop() {
	this->endTime=this->getTime();
}

int Stopwatch::getNanoseconds() {
	return std::chrono::duration_cast<std::chrono::nanoseconds>(endTime-startTime).count();
}

int Stopwatch::getMicroseconds() {
	return std::chrono::duration_cast<std::chrono::microseconds>(endTime-startTime).count();
}

int Stopwatch::getMilliseconds() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(endTime-startTime).count();
}

int Stopwatch::getSeconds() {
	return std::chrono::duration_cast<std::chrono::seconds>(endTime-startTime).count();
}