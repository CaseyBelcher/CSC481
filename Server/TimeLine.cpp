#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <chrono>
#include <iostream>

using namespace std::chrono;

class TimeLine {
public:
	int getTime = 0;
};

class Gametime : public TimeLine {

private:
	milliseconds start_time;
	int original_step_size;
	int step_size;
	milliseconds ea;
	milliseconds lastTimeMarker;
	int runningTotal;

public:
	Gametime(int step_size) {
		this->step_size = step_size;
		this->original_step_size = step_size;
		time_point<system_clock> epoch;
		this->start_time = duration_cast<milliseconds>(system_clock::now() - epoch);
		this->lastTimeMarker = start_time;
		this->runningTotal = 0;
	}

	int getTime() {

		time_point<system_clock> epoch;
		milliseconds now = duration_cast<milliseconds>(system_clock::now() - epoch);
		this->ea = now - (this->lastTimeMarker);

		int elapsed = (int)ea.count() / step_size;
		runningTotal += elapsed;
		if (elapsed >= 1) {
			this->lastTimeMarker = now;
		}
		return runningTotal;
	}


	void halfTime() {
		this->step_size = 2 * original_step_size;
	}
	void oneTime() {
		this->step_size = original_step_size;
	}
	void doubleTime() {
		this->step_size = original_step_size / 2;
	}


};