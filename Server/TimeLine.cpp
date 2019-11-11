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
	bool paused = false;
	milliseconds ea;

public:
	Gametime(int step_size) {
		this->step_size = step_size;
		this->original_step_size = step_size;
		time_point<system_clock> epoch;
		this->start_time = duration_cast<milliseconds>(system_clock::now() - epoch);
	}

	int getTime() {
		if (!paused) {
			time_point<system_clock> epoch;
			milliseconds now = duration_cast<milliseconds>(system_clock::now() - epoch);
			this->ea = now - (this->start_time);
		}
		int elapsed = (int)ea.count() / step_size;
		return elapsed;

	}

	void pause() {
		this->paused = true;
	}
	void unPause() {
		this->paused = false;
	}
	bool isPaused() {
		return paused;
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