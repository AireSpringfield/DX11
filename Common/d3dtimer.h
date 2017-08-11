#ifndef D3DTIMER_H
#define D3DTIMER_H
#include<Windows.h>

class D3DTimer {
public:
	D3DTimer();

	float DeltaTime();
	float TotalTime();

	void Reset();
	void Pause();
	void Continue();
	void Tick();

private:
	INT64 time_reset_ = 0;
	INT64 time_pause_ = 0;
	INT64 time_continue_ = 0;
	
	INT64 time_pause_total_ = 0;

	INT64 time_curr_ = 0;
	INT64 time_prev_ = 0;

	double seconds_per_count_ = 0.0;
	double delta_time_ = 0.0;


	bool is_paused_ = true;
};

#endif