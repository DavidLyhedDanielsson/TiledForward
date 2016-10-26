#include "logger.h"
#include <algorithm>

#ifdef _WIN32
CONSOLE_LOG_LEVEL Logger::consoleLogLevel = CONSOLE_LOG_LEVEL::FULL | CONSOLE_LOG_LEVEL::DEBUG_STRING;
#else
CONSOLE_LOG_LEVEL Logger::consoleLogLevel = CONSOLE_LOG_LEVEL::FULL;
#endif

bool Logger::printStackTrace = true;

std::string Logger::outPath = "";
std::string Logger::outName = "log.txt";
std::string Logger::separatorString = " ";
std::ios_base::openmode Logger::openMode = std::ios_base::app;

std::function<void(std::string)> Logger::callOnLog = nullptr;

void Logger::Print(std::string message)
{
#ifdef _WIN32
	if(consoleLogLevel &= CONSOLE_LOG_LEVEL::DEBUG_STRING)
		OutputDebugStringA(message.c_str());
#endif //_WIN32

	std::cout << message;
}

void Logger::Init()
{
#ifdef _WIN32 
	#ifndef STANDALONE
	SymSetOptions(SYMOPT_LOAD_LINES);
	HANDLE handle = GetCurrentProcess();
	SymInitialize(handle, NULL, TRUE);
	#endif
#else
	consoleLogLevel = CONSOLE_LOG_LEVEL::FULL;
#endif
}

#ifdef _WIN32
void Logger::Deinit()
{
#ifndef STANDALONE
	HANDLE handle = GetCurrentProcess();
	SymCleanup(handle);	
#endif
}
#endif // _WIN32

void Logger::LogLine(LOG_TYPE logType, const std::string& text)
{
	Log(logType, std::string(text) + "\n");
}

void Logger::Log(LOG_TYPE logType, const std::string& text)
{
#ifdef NDEBUG
	if(logType == LOG_TYPE::DEBUG)
		return;
#endif // _DEBUG

	std::string message("");

	switch(logType)
	{
		case LOG_TYPE::NONE:
			break;
		case LOG_TYPE::INFO:
		case LOG_TYPE::INFO_NOWRITE:
			message += "[INFO]";
			break;
		case LOG_TYPE::WARNING:
		case LOG_TYPE::WARNING_NOWRITE:
			message += "[WARNING]";
			break;
		case LOG_TYPE::FATAL:
		case LOG_TYPE::FATAL_NOWRITE:
			message += "[FATAL]";
			break;
		case LOG_TYPE::DEBUG:
		case LOG_TYPE::DEBUG_NOWRITE:
			message += "[DEBUG]";
			break;
		default:
			break;
	}

	//Only log error type to console
	//Print message and make sure there is only one \n at the end of it
	if(consoleLogLevel &= CONSOLE_LOG_LEVEL::PARTIAL)
		text[text.size() - 1] != '\n' ? Print(message) : Print(message + "\n");

	if(logType != LOG_TYPE::NONE)
		message += std::string(separatorString.begin(), separatorString.end());

	message += text;

	if(printStackTrace 
	   && (logType == LOG_TYPE::WARNING
		   || logType == LOG_TYPE::FATAL
		   || logType == LOG_TYPE::WARNING_NOWRITE 
		   || logType == LOG_TYPE::FATAL_NOWRITE))
	{
		message += "Stack trace:\n";

#ifdef _WIN32
		std::stringstream sstream;

		//http://stackoverflow.com/questions/590160/how-to-log-stack-frames-with-windows-x64
		// Quote from Microsoft Documentation:
		// ## Windows Server 2003 and Windows XP:
		// ## The sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
		const int kMaxCallers = 62;

		void* callers_stack[kMaxCallers];
		unsigned short frames = CaptureStackBackTrace(0, kMaxCallers, callers_stack, NULL);

		IMAGEHLP_LINE64 line;
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

		HANDLE handle = GetCurrentProcess();

		const static unsigned short MAX_CALLERS_SHOWN = 12;
		frames = std::min(frames, MAX_CALLERS_SHOWN);
		for(unsigned int i = 0; i < frames; i++)
		{
			DWORD symFromAddrCanTake0AsAConstantButNotSymGetLineFromAddr64WtfMicrosoft;
			if(SymGetLineFromAddr64(handle, (DWORD64)(callers_stack[i]), &symFromAddrCanTake0AsAConstantButNotSymGetLineFromAddr64WtfMicrosoft, &line))
			{
				std::string fileName(line.FileName);
				fileName = fileName.substr(fileName.find_last_of("/\\") + 1); //File name with extension
				fileName = fileName.substr(0, fileName.find_last_of('.')); //File name without extension

				if(fileName == "logger"
				   || fileName == "Logger")
				{
					frames++;
					frames = std::min(static_cast<int>(frames), kMaxCallers);
				}
				else
					sstream << "    " << line.FileName << "(" << line.LineNumber << ")" << std::endl;
			}
			else
			{
				SYMBOL_INFO* symbol;
				symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
				symbol->MaxNameLen = 255;
				symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

				SymFromAddr(handle, (DWORD64)(callers_stack[i]), 0, symbol);
				sstream << "Couldn't get line numbers, make sure to call Logger::Init (WIN32 only). " << symbol->Name << std::endl;

				free(symbol);
			}
		}

		message += sstream.str();
#else
		void* stackTrace[16];
		int size = backtrace(stackTrace, 16);

		char** strings = backtrace_symbols(stackTrace, size);
		for(int i = 0; i < size; i++)
		{
            //Find address
			int begin = 0;
			while(strings[i][begin] != '('
				  && strings[i][begin] != ' '
				  && strings[i][begin] != 0)
				++begin;

			char syscom[256];
			char fileBuffer[512]; //Should be big enough for file paths

            //Concat into command to pipe to file
			sprintf(syscom, "addr2line %p -e %.*s", stackTrace[i], begin, strings[i]);

            std::string line;

			FILE* filePtr;
			filePtr = popen(syscom, "r");
			while(fgets(fileBuffer, sizeof(fileBuffer), filePtr) != NULL)
                line = fileBuffer;
			pclose(filePtr);

            if(line.back() == '\n')
                line.pop_back();

            if(line.compare(0, 2, "??") != 0)
            {
                std::string fileName;
                fileName = line.substr(line.find_last_of("/\\") + 1); //File name with extension and line number

                if((fileName[0] != 'l' && fileName[0] != 'L')
                   && !fileName.compare(1, 5, "ogger") == 0)
                {
                    message += line + '\n';
                }
            }
		}

		free(strings);
#endif //_WIN32
	}

	if(callOnLog != nullptr)
		callOnLog(message);

	if((consoleLogLevel &= CONSOLE_LOG_LEVEL::FULL)
	   || (consoleLogLevel &= CONSOLE_LOG_LEVEL::EXCLUSIVE))
	{
		Print(message);

		if(consoleLogLevel &= CONSOLE_LOG_LEVEL::EXCLUSIVE)
			return; //Don't write anything to file
	}
#ifdef _WIN32
	else if(consoleLogLevel &= CONSOLE_LOG_LEVEL::DEBUG_STRING)
		OutputDebugStringA(message.c_str());
	else if(consoleLogLevel &= CONSOLE_LOG_LEVEL::DEBUG_STRING_EXCLUSIVE)
	{
		OutputDebugStringA(message.c_str());

		return;
	}
#endif //_WIN32

	if(logType == LOG_TYPE::INFO_NOWRITE
	   || logType == LOG_TYPE::WARNING_NOWRITE
	   || logType == LOG_TYPE::FATAL_NOWRITE
	   || logType == LOG_TYPE::DEBUG_NOWRITE)
		return;

	std::ofstream out;
	out.open(outPath + outName, openMode);

	if(out.is_open())
	{
		out.write(&message[0], message.size());
		out.close();
	}

	if(message.back() == '\n')
		message.pop_back();
}

void Logger::ClearLog()
{
	std::ifstream in(outPath+ outName);
	if(!in.is_open())
		return; //File didn't exist

	if(in.peek() == std::ifstream::traits_type::eof())
	{
		in.close();
		return; //File is empty
	}

	in.close();

	std::ofstream out;
	out.open(outPath + outName, std::ios_base::trunc | std::ios_base::out);
	out.close();
}

void Logger::SetSeparatorString(const std::string& newSeparatorString)
{
	separatorString = newSeparatorString;
}

void Logger::SetOutputDir(const std::string& path, std::ios_base::openmode newOpenMode)
{
	outPath = path;
	openMode = newOpenMode;
}

void Logger::SetOutputName(const std::string& name, std::ios_base::openmode newOpenMode)
{
	outName = name;
	openMode = newOpenMode;
}

void Logger::SetConsoleLogLevel(CONSOLE_LOG_LEVEL logLevel)
{
	consoleLogLevel = logLevel;
}

void Logger::SetCallOnLog(std::function<void(std::string)> function)
{
	callOnLog = std::move(function);
}

void Logger::PrintStackTrace(bool print)
{
	printStackTrace = print;
}

