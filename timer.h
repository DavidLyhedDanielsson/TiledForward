#ifndef Timer_h__
#define Timer_h__

#include <ctime>
#include <chrono>

/**
* A CPU timer
*/
class Timer
{
public:
	Timer();	

	~Timer() = default;

	/**
	* Updates the delta time
	*
	* \see GetDelta
	*/
	void UpdateDelta();
	/**
	* Resets the delta time to 0
	*/
	void ResetDelta();

	/**
	* Starts the timer
	*/
	void Start();
	/**
	* Stops (pauses) the timer
	* 
	*/
	void Stop();
	/**
	* Resets the timer. Doesn't stop the timer
	*/
	void Reset();

	/**
	* Gets the time since Start was called, excluding any paused time
	*
	* \see GetTimeNanoseconds
	* \see GetTimeMicroseconds
	* \see GetTimeMillisecondsFraction
	* \see GetTimeMilliseconds
	*
	* \returns the current delta
	*/
	std::chrono::high_resolution_clock::duration GetTime() const;
	/**
	* Gets the time since UpdateDelta was called
	*
	* \see GetDeltaMillisecondsFraction
	* 
	* \returns the current delta
	*/
	std::chrono::high_resolution_clock::duration GetDelta() const;

	/**
	* Gets the time since Start was called, excluding any paused time
	* 
	* \returns number of (whole) nanoseconds since calling Start()
	*/
	long long GetTimeNanoseconds() const;
	/**
	* Gets the time since Start was called, excluding any paused time
	* 
	* \returns number of (whole) microsecondes since calling Start()
	*/
	long long GetTimeMicroseconds() const;
	/**
	* Gets the time since Start was called, excluding any paused time
	*
	* \see GetTimeMillisecondsFraction
	*
	* \returns number of (whole) milliseconds since calling Start()
	*/
	long long GetTimeMilliseconds() const;
	/**
	* Gets the time since Start was called, excluding any paused time
	*
	* Example:\n
	* std::cout << GetTimeMillisecondsFraction()\n
	* OUTPUT: 41.412312
	*
	* \see GetTimeMilliseconds
	*
	* \returns number of milliseconds (with decimals) since calling Start()
	*/
	float GetTimeMillisecondsFraction() const;

	/**
	* Gets the current delta time given in milliseconds as a fraction
	*
	* Example:\n
	* std::cout << GetDeltaMillisecondsFraction()\n
	* OUTPUT: 41.412312
	*
	* \see GetDelta
	*
	* \returns number of milliseconds (with decimals) since calling UpdateDelta()
	*/
	float GetDeltaMillisecondsFraction() const;

	/**
	* Whether or not the timer is currently running
	* 
	* \returns whether or not the timer is running
	*/
	bool IsRunning() const;

private:
	bool isRunning;

	std::chrono::high_resolution_clock::time_point currentTime;
	std::chrono::high_resolution_clock::time_point previousTime;
	std::chrono::high_resolution_clock::time_point creationTime;
	std::chrono::high_resolution_clock::time_point stopTime;

	std::chrono::high_resolution_clock::duration runTime;
	std::chrono::high_resolution_clock::duration pauseTime;
	std::chrono::high_resolution_clock::duration deltaTime;
	std::chrono::high_resolution_clock::duration deltaPauseTime;
};

#endif // Timer_h__
