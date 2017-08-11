#include"d3dtimer.h"

D3DTimer::D3DTimer(){
	INT64 counts_per_second;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&counts_per_second));
	seconds_per_count_ = 1.0 / static_cast<double>(counts_per_second);
}

float D3DTimer::DeltaTime() {
	return static_cast<float>(delta_time_);
}

float D3DTimer::TotalTime() {
	if (!is_paused_)
		return static_cast<float>((time_curr_ - time_reset_ - time_pause_total_)*seconds_per_count_);
	else
		return static_cast<float>((time_pause_ - time_reset_ - time_pause_total_)*seconds_per_count_);
}

void D3DTimer::Tick() {
	if (is_paused_) {
		delta_time_ = 0.0;
		return;
	}

	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time_curr_));

	delta_time_ = (time_curr_ - time_prev_) * seconds_per_count_;
	time_prev_ = time_curr_;

	// Force nonnegative. This is caused by switching processor or power save mode.
	if (delta_time_ < 0.0) {
		delta_time_ = 0;
	}

}



void D3DTimer::Reset() {
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time_reset_));
	time_prev_ = time_reset_;
	time_pause_total_ = 0;
	is_paused_ = false; // After reseting, the timing is started
}


void D3DTimer::Pause() {
	if (!is_paused_) {
		is_paused_ = true;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time_pause_));
	}

}

void D3DTimer::Continue() {
	if (is_paused_) {
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time_continue_));
		time_pause_total_ += time_continue_ - time_pause_;
		time_prev_ = time_continue_;
		is_paused_ = false;
	}
}





