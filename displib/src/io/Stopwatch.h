#include <chrono>

class Stopwatch {
	public:
	std::chrono::steady_clock::time_point startTime, endTime;

	std::chrono::steady_clock::time_point getTime();

	Stopwatch();

	void start();

	void stop();

	int getNanoseconds();

	int getMicroseconds();

	int getMilliseconds();

	int getSeconds();
};