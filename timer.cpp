#include "timer.h"

Timer::Timer()
	: isRunning(false)
	, currentTime(std::chrono::high_resolution_clock::now())
	, previousTime(currentTime)
	, creationTime(currentTime)
	, stopTime(currentTime)
	, runTime(std::chrono::high_resolution_clock::duration::zero())
	, pauseTime(std::chrono::high_resolution_clock::duration::zero())
	, deltaTime(std::chrono::high_resolution_clock::duration::zero())
	, deltaPauseTime(std::chrono::high_resolution_clock::duration::zero())
{ }

void Timer::UpdateDelta()
{
	currentTime = std::chrono::high_resolution_clock::now();

	deltaTime = currentTime - previousTime - deltaPauseTime;
	deltaPauseTime = std::chrono::nanoseconds::zero();

	previousTime = currentTime;
}

void Timer::ResetDelta()
{
	deltaTime = std::chrono::high_resolution_clock::duration::zero();
	previousTime = std::chrono::high_resolution_clock::now();
}

void Timer::Start()
{
	if(!isRunning)
	{
		pauseTime += std::chrono::high_resolution_clock::now() - stopTime;
		deltaPauseTime += std::chrono::high_resolution_clock::now() - stopTime;

		isRunning = true;
	}
}

void Timer::Stop()
{
	if(isRunning)
	{
		stopTime = std::chrono::high_resolution_clock::now();

		isRunning = false;
	}
}

void Timer::Reset()
{
	currentTime = std::chrono::high_resolution_clock::now();

	previousTime = currentTime;
	creationTime = currentTime;
	stopTime = currentTime;

	runTime = std::chrono::high_resolution_clock::duration::zero();
	pauseTime = std::chrono::high_resolution_clock::duration::zero();
	deltaTime = std::chrono::high_resolution_clock::duration::zero();
}

std::chrono::high_resolution_clock::duration Timer::GetTime() const
{
	if(isRunning)
		return std::chrono::high_resolution_clock::now() - creationTime - pauseTime;
	else
		return stopTime - creationTime - pauseTime;
}

std::chrono::high_resolution_clock::duration Timer::GetDelta() const
{
	return deltaTime;
}

long long Timer::GetTimeNanoseconds() const
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(GetTime()).count();
}

long long Timer::GetTimeMicroseconds() const
{
	return std::chrono::duration_cast<std::chrono::microseconds>(GetTime()).count();
}

long long Timer::GetTimeMilliseconds() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(GetTime()).count();
}

float Timer::GetTimeMillisecondsFraction() const
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(GetTime()).count() * 0.000001f; //-V562
}

float Timer::GetDeltaMillisecondsFraction() const
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(GetDelta()).count() * 0.000001f; //-V562
}

bool Timer::IsRunning() const
{
	return isRunning;
}
