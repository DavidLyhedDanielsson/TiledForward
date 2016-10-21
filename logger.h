/** \file log.h
* Contains a logger class as well as CONSOLE_LOG_LEVEL
*/

#ifndef Logger_h__
#define Logger_h__

#include <string>
#include <functional>
#include <ios>
#include <type_traits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <string>

#ifdef _WIN32
#include <windows.h>

#pragma warning(disable : 4091)
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")
#pragma warning(default : 4091)

#else

#include <execinfo.h>

#endif //_WIN32

/**
* Controls how much is drawn to the console or to Visual Studio's output window (PrintDebugString)
*
* \see SetConsoleLogLevel
*/
enum class CONSOLE_LOG_LEVEL
		: int
{
	NONE = 0x1 //Don't print anything in the console
	, PARTIAL = 0x2 //Print only the message type
	, FULL = 0x4 //Print exactly what is written in the file
	, EXCLUSIVE = 0x8 //Print exclusively to the console
#ifdef _WIN32
	//Combine any of the types above with the ones below to use Visual Studio's output window
	, DEBUG_STRING = 0x10 //Use binary or to output to Visual Studio's output window
	, DEBUG_STRING_EXCLUSIVE = 0x20 //Use binary or to output to Visual Studio's output window instead of console
#endif
};

/**
* Binary or between CONSOLE_LOG_LEVEL
*/
inline CONSOLE_LOG_LEVEL operator|(CONSOLE_LOG_LEVEL lhs, CONSOLE_LOG_LEVEL rhs)
{
	return static_cast<CONSOLE_LOG_LEVEL>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

/**
* Binary or-equal between CONSOLE_LOG_LEVEL
*/
inline CONSOLE_LOG_LEVEL operator|=(CONSOLE_LOG_LEVEL lhs, CONSOLE_LOG_LEVEL rhs)
{
	CONSOLE_LOG_LEVEL bitwiseOr = static_cast<CONSOLE_LOG_LEVEL>(static_cast<int>(lhs) | static_cast<int>(rhs));

	return bitwiseOr;
}

/**
* Binary and between CONSOLE_LOG_LEVEL
*/
inline CONSOLE_LOG_LEVEL operator&(CONSOLE_LOG_LEVEL lhs, CONSOLE_LOG_LEVEL rhs)
{
	return static_cast<CONSOLE_LOG_LEVEL>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

/**
* Checks to see if \p lhs contains \p rhs through binary &. OBS: ABNORMAL BEHAVIOUR!
*/
inline bool operator&=(CONSOLE_LOG_LEVEL lhs, CONSOLE_LOG_LEVEL rhs)
{
	CONSOLE_LOG_LEVEL binaryAnd = static_cast<CONSOLE_LOG_LEVEL>(static_cast<int>(lhs) & static_cast<int>(rhs));

	return binaryAnd == rhs;
}

/**
* Type of the logged message. Controls what to print before the message e.g. [INFO]
*
* If WARNING or FATAL are given, a stack trace will be logged after the message
* NONE means no tag will be logged, just raw text
* DEBUG will only be printed if _DEBUG is defined
*
* Any option containing _NOWRITE will not write to file
*
* \see Logger::LogLine
* \see Logger::Log
*/
enum class LOG_TYPE
		: int
{
	NONE = 0
	, INFO
	, INFO_NOWRITE
	, WARNING
	, WARNING_NOWRITE
	, FATAL
	, FATAL_NOWRITE
	, DEBUG
	, DEBUG_NOWRITE
};

/**
* Logs text to a file or to the console.
*
* Also supports logging to Visual Studio's output window
*/
class Logger
{
private:
	static CONSOLE_LOG_LEVEL consoleLogLevel;

	static bool printStackTrace;

	//Path to log
	static std::string outPath;
	static std::string outName;

	//When you call log the text will be formatted as such: <LOG_TYPE><separatorString><message>
	static std::string separatorString;

	static std::ios_base::openmode openMode;

	static void Print(std::string message);

	static std::function<void(std::string)> callOnLog;

public:
	//static Logger();
	//~Logger()

	/**
	* Sets default values for variables. Call before using.
	*/
	static void Init();

#ifdef _WIN32
	/**
	* Deinitializes this logger. Call before exiting
	*/
	static void Deinit();
#endif // _WIN32

	/**
	* \see LogLine, logs multiple data types
	* 
	* \param logType
	* \param types
	*/
	template<typename... T>
	static void LogLine(LOG_TYPE logType, T... types)
	{
		std::string message;
		LogLineInternal(logType, message, types...);
	}

	/**
	* Logs the given text and appends a newline
	*
	* Example:\n
	* `Logger::LogLine(LOG_TYPE::FATAL, "Couldn't create buffers!");`
	*
	* \param logType log type
	* \param text line to log
	*/
	static void LogLine(LOG_TYPE logType, const std::string& text);

	/**
	* Logs the given text as-is
	*
	* `Logger::LogLine(LOG_TYPE::FATAL, "Couldn't create buffers!");`
	*
	* \param logType log type
	* \param text text to log
	*/
	static void Log(LOG_TYPE logType, const std::string& text);

	/**
	* Clears the output file
	*
	* \see SetOutputDir
	* \see SetOutputName
	*/
	static void ClearLog();

	//////////////////////////////////////////////////////////////////////////
	//GETTERS
	//////////////////////////////////////////////////////////////////////////
	/**
	* When you call log the text will be formatted as such: <LOG_TYPE><separatorString><message>
	*
	* \param newSeparatorString
	*/
	static void SetSeparatorString(const std::string& newSeparatorString);

	/**
	* Specifies where to put the log file. No checking is done to make sure path is valid
	*
	* \see SetOutputName
	*
	* \param path new path
	* \param newOpenMode new open mode. Default is std::ios_base::app
	*/
	static void SetOutputDir(const std::string& path, std::ios_base::openmode newOpenMode);

	/**
	* Specifies what to call the log file. Include file extension as well
	*
	* \param name
	* \param newOpenMode
	* \returns
	*/
	static void SetOutputName(const std::string& name, std::ios_base::openmode newOpenMode);

	/**
	* Specifies what to write to the console (through cout)
	*
	* `CONSOLE_LOG_LEVEL::NONE` = > No text outputted to console \n
	* `CONSOLE_LOG_LEVEL::PARTIAL` = > Only write log type to console \n
	* `CONSOLE_LOG_LEVEL::FULL` = > Write everything to console \n
	* `CONSOLE_LOG_LEVEL::ONLY` = > Only output to console \n
	*
	* \param logLevel
	*/
	static void SetConsoleLogLevel(CONSOLE_LOG_LEVEL logLevel);

	/**
	* Whenever text is logged this method will be called
	*
	* Example:\n
	* `SetCallOnLog(std::bind(&SomeClass::SomeMethod, this, std::placeholders::_1));`
	*
	* \param function
	*/
	static void SetCallOnLog(std::function<void(std::string)> function);

	/**
	* Sets whether or not to log stack trace whenever log type is fatal or warning
	*
	* \param print
	* \returns
	*/
	static void PrintStackTrace(bool print);

private:
	template<typename T, typename... Args>
	static void LogLineInternal(LOG_TYPE logType, std::string& message, T arg, Args... args)
	{
		message += std::to_string(arg);

		LogLineInternal(logType, message, args...);
	}

	template<typename... Args>
	static void LogLineInternal(LOG_TYPE logType, std::string& message, const std::string& stdString, Args... args)
	{
		message += stdString.c_str();

		LogLineInternal(logType, message, args...);
	}

	template<typename... Args>
	static void LogLineInternal(LOG_TYPE logType, std::string& message, const char* cString, Args... args)
	{
		message += cString;

		LogLineInternal(logType, message, args...);
	}

	static void LogLineInternal(LOG_TYPE logType, std::string& message)
	{
		LogLine(logType, message);
	}
};

#endif // Logger_h__
