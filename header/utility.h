#ifndef _UTILITY_H_
#define _UTILITY_H_

#ifdef BLACK_BACKGROUND
#define DEFAULT_COLOR 0x0001 | 0x0002 | 0x0004
#else
#define DEFAULT_COLOR 0x0010 | 0x0020 | 0x0040 | 0x0080
#endif

///////////////////////////////////////////////////////
// External Includes
///////////////////////////////////////////////////////
#include <iostream>
#include <string>
#include <io.h>
#include <Shlwapi.h>
#include <Windows.h>

///////////////////////////////////////////////////////
// My Includes
///////////////////////////////////////////////////////
#include "..\\header\\types.h"


namespace logging {

	enum LogLvl {
		info,
		warning,
		error,
		debug
	};

	LogLvl LoglevelFromInt(int);

	class Logger
	{
	public:

		Logger();
		Logger(LogLvl);

		unsigned short GetLevel();
		void SetMaxLoggingLevel(LogLvl);

		void Log(LogLvl level, std::string message);
		void Log(LogLvl level, char*);
		void Log(LogLvl level, const char*);

		void Log(LogLvl level, short indent, std::string message);
		void Log(LogLvl level, short indent, char*);
		void Log(LogLvl level, short indent, const char*);

	private:

		void PrintLevelString(LogLvl level);

		LogLvl m_maxlvl;
	};
}

namespace utility {

	short OccurencesOf(string origin, char delim);

	void ResetConsoleColor();

	void PrintInputFormat();
	void PrintGreeting();
	void PrintUserInput(string, string, bool, bool, logging::LogLvl, size_t[16], types::Sorting);

	bool CheckInput(string&, string&, string&, bool&, bool&, logging::LogLvl&, size_t(&)[16], types::Sorting&);
	void GetUserInput(string&, string&, bool&, bool&, logging::LogLvl&, size_t(&)[16], types::Sorting&);
}

#endif /* _UTILITY_H_ */
