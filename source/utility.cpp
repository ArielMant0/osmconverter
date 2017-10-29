#include "..\\header\\utility.h"

using std::string;
using std::cout;
using std::endl;

///////////////////////////////////////////////////////
// Logging
///////////////////////////////////////////////////////

logging::LogLvl logging::LoglevelFromInt(int i)
{
	switch (i)
	{
		case 0: return info; break;
		case 1: return warning; break;
		case 2: return error; break;
		case 3: return debug; break;
		default: return info;
	}
}

logging::Logger::Logger()
{
	m_maxlvl = error;
}

logging::Logger::Logger(LogLvl lvl)
{
	m_maxlvl = lvl;
}

void logging::Logger::PrintLevelString(LogLvl level)
{
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	switch (level)
	{
		case info:
		{
			cout << "[";
			SetConsoleTextAttribute(console, FOREGROUND_GREEN);
			cout << "INFO";
			SetConsoleTextAttribute(console, DEFAULT_COLOR);
			cout << "]:    ";
		} break;
		case warning:
		{
			cout << "[";
			SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN);
			cout << "WARNING";
			SetConsoleTextAttribute(console, DEFAULT_COLOR);
			cout << "]: ";
		} break;
		case debug:
		{
			cout << "[";
			SetConsoleTextAttribute(console, FOREGROUND_BLUE);
			cout << "DEBUG";
			SetConsoleTextAttribute(console, DEFAULT_COLOR);
			cout << "]:   ";
		} break;
		case error:
		{
			cout << "[";
			SetConsoleTextAttribute(console, FOREGROUND_RED);
			cout << "ERROR";
			SetConsoleTextAttribute(console, DEFAULT_COLOR);
			cout << "]:   ";
		} break;
	}
}

unsigned short logging::Logger::GetLevel()
{
	switch (m_maxlvl)
	{
		case info: return 0; break;
		case warning: return 1; break;
		case debug: return 3; break;
		case error: return 2; break;
	}
	return 2;
}

void logging::Logger::SetMaxLoggingLevel(LogLvl lvl)
{
	m_maxlvl = lvl;
}

void logging::Logger::Log(LogLvl level, std::string message)
{
	if (level <= m_maxlvl)
	{
		PrintLevelString(level);
		cout << message << endl;
	}
}

void logging::Logger::Log(LogLvl level, char *message)
{
	if (level <= m_maxlvl)
	{
		PrintLevelString(level);
		cout << message << endl;
	}
}

void logging::Logger::Log(LogLvl level, const char *message)
{
	if (level <= m_maxlvl)
	{
		PrintLevelString(level);
		cout << message << endl;
	}
}

void logging::Logger::Log(LogLvl level, short indent, std::string message)
{
	if (level <= m_maxlvl)
	{
		PrintLevelString(level);
		for (short i = 0; i < indent; i++)
		{
			cout << "\t";
		}
		cout << message << endl;
	}
}

void logging::Logger::Log(LogLvl level, short indent, char *message)
{
	if (level <= m_maxlvl)
	{
		PrintLevelString(level);
		for (short i = 0; i < indent; i++)
		{
			cout << "\t";
		}
		cout << message << endl;
	}
}

void logging::Logger::Log(LogLvl level, short indent, const char *message)
{
	if (level <= m_maxlvl)
	{
		PrintLevelString(level);
		for (short i = 0; i < indent; i++)
		{
			cout << "\t";
		}
		cout << message << endl;
	}
}

///////////////////////////////////////////////////////
// User Input Handling
///////////////////////////////////////////////////////
short utility::OccurencesOf(string origin, char delim)
{
	short result = 0;

	for (size_t i = 0; i < origin.length(); i++) {
		if (origin[i] == delim)
			result++;
	}

	return result;
}

void utility::ResetConsoleColor()
{
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console, DEFAULT_COLOR);
}

void utility::PrintGreeting()
{
	cout << "**********************************OSMConverter-Application**********************************" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*  Author: Franziska Becker                                                                *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*  This application can read PBF files containing data from                                *" << endl;
	cout << "*  openstreetmap.                                                                          *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*  The output will be a fairly simple geometry - based datapool that                       *" << endl;
	cout << "*  you can use to your liking.                                                             *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*                                                                  Have Fun!               *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "**********************************OSMConverter-Application**********************************" << endl;
}

void utility::PrintInputFormat()
{
	cout << "**********************************OSMConverter-Application**********************************" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*  Here is what your input should look like:                                               *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*  in=my_input.pbf [--debug] [out=out_dir] [sort=f] [line=d] [log=3]                       *" << endl;
	cout << "*                  [lod=1-1-1-1-1-1-1-1-1-1-1-1-1-1-1-1]                                   *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*  Everything in square brackets is optional, if you don't use those                       *" << endl;
	cout << "*  parameters the default input is as follows:                                             *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*  in=my_input.pbf out=./ lod=(see-below) sort=f line=d log=3                              *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*  Flag --debug: generates additional (human readable) text files for all files            *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*  Values for log:  Sets the logging level                                                 *" << endl;
	cout << "*                   0 -> Only print status information                                     *" << endl;
	cout << "*                   1 -> Print status information and warnings                             *" << endl;
	cout << "*                   2 -> Print status information, warnings and errors                     *" << endl;
	cout << "*                   3 -> Print status information, warnings, errors and debug information  *" << endl;
	cout << "*  Values for lod:  Any positive number that can fit into an 32-Bit / 64-Bit Integer       *" << endl;
	cout << "*                   (depends on system architecture)                                       *" << endl;
	cout << "*  Values for sort: f|F -> Sort by first element                                           *" << endl;
	cout << "*                   m|M -> Sort by majority                                                *" << endl;
	cout << "*                   s|S -> Divide elements that span across tiles                          *" << endl;
	cout << "*  Values for line: d|D -> Do line simplification using Douglas-Peucker algorithm          *" << endl;
	cout << "*                   v|V -> Do line simplification using Visvalingam-Whyatt algorithm       *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*  The lod parameter sets the root number of tiles per LOD (starting at LoD 0              *" << endl;
	cout << "*  up to LoD 15) you wish to have.                                                         *" << endl;
	cout << "*  If 0 is specified the tile in question will be ommited. If the parameter is             *" << endl;
	cout << "*  left out, the following default values (computed for the planet file) will              *" << endl;
	cout << "*  be used.                                                                                *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*  These default tile values are:                                                          *" << endl;
	cout << "*        LOD  0 = 1 x 1             = 1            (1:500 Million)                         *" << endl;
	cout << "*        LOD  1 = 2 x 2             = 4            (1:250 Million)                         *" << endl;
	cout << "*        LOD  2 = 4 x 4             = 16           (1:150 Million)                         *" << endl;
	cout << "*        LOD  3 = 8 x 8             = 64           (1:70 Million)                          *" << endl;
	cout << "*        LOD  4 = 16 x 16           = 256          (1:35 Million)                          *" << endl;
	cout << "*        LOD  5 = 32 x 32           = 1024         (1:15 Million)                          *" << endl;
	cout << "*        LOD  6 = 64 x 64           = 4096         (1:10 Million)                          *" << endl;
	cout << "*        LOD  7 = 128 x 128         = 16387        (1:4 Million)                           *" << endl;
	cout << "*        LOD  8 = 256 x 256         = 65536        (1:2 Million)                           *" << endl;
	cout << "*        LOD  9 = 512 x 512         = 262144       (1:1 Million)                           *" << endl;
	cout << "*        LOD 10 = 1024 x 1024       = 1048576      (1:500000)                              *" << endl;
	cout << "*        LOD 11 = 2048 x 2048       = 4194304      (1:250000)                              *" << endl;
	cout << "*        LOD 12 = 4096 x 4096       = 16777216     (1:150000)                              *" << endl;
	cout << "*        LOD 13 = 8192 x 8192       = 67108864     (1:70000)                               *" << endl;
	cout << "*        LOD 14 = 16384 x 16384     = 268435456    (1:35000)                               *" << endl;
	cout << "*        LOD 15 = 32768 x 32768     = 1073741824   (1:15000)                               *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "*                                                                                          *" << endl;
	cout << "**********************************OSMConverter-Application**********************************" << endl;
}

void utility::PrintUserInput(string in, string out, bool debug, bool line, logging::LogLvl log, size_t lods[16], types::Sorting s)
{
	string sort, loglvl;

	switch (s)
	{
		case types::Sorting::first_node: sort.assign("First Node"); break;
		case types::Sorting::most_nodes: sort.assign("Most Node"); break;
		case types::Sorting::subdivide: sort.assign("Subdivide"); break;
	}

	switch (log)
	{
		case logging::LogLvl::info: loglvl.assign("Info (" + std::to_string(log) + ")"); break;
		case logging::LogLvl::warning: loglvl.assign("Warning (" + std::to_string(log) + ")"); break;
		case logging::LogLvl::error: loglvl.assign("Error (" + std::to_string(log) + ")"); break;
		case logging::LogLvl::debug: loglvl.assign("Debug (" + std::to_string(log) + ")"); break;
	}

	cout << "\t\tinput file: " + in << endl;
	cout << "\t\toutput directory: " + out << endl;
	cout << "\t\tdebug: " + (debug ? string("true") : string("false")) << endl;
	cout << "\t\tline simplification: " + (line ? string("Douglas-Peucker") : string("Visvalingam-Whyatt")) << endl;
	cout << "\t\tlog level: " + loglvl << endl;
	cout << "\t\tsorting: " + sort << endl;
	cout << "\t\tLoDs: " << endl;
	for (int i = 0; i < 16; i++)
	{
		cout << "\t\t\tLoD " + to_string(i) + " : " + to_string(lods[i]) + " x " + to_string(lods[i]) << endl;
	}
}

bool utility::CheckInput(string &test, string &in, string &out, bool &de, bool &l, logging::LogLvl &log, size_t (&lods)[16], types::Sorting &s)
{
	bool found_param[7] = { false };
	short limit = OccurencesOf(test, ' ');
	string::size_type found;

	// Get length of currend directory path
	DWORD needed_size = GetCurrentDirectory(0, NULL);

	TCHAR *buffer = new TCHAR[needed_size];
	DWORD dw_return = GetCurrentDirectory(needed_size, buffer);

	if (dw_return == 0)
	{
		cout << "GetCurrentDirectory() failed" << endl;
		return false;
	}

	for (short j = 0; j <= limit; j++)
	{
		if (!found_param[0] && (found = test.find("in=")) != string::npos)
		{
			found_param[0] = true;
			size_t at = found + 3;
			if ((found = test.substr(at, string::npos).find(" ")) != string::npos)
			{
				in.assign(test.substr(at, found));

				// Make path windows specific
				for (size_t replace = 0; replace < in.length(); replace++)
				{
					if (in[replace] == '/')
						in[replace] = '\\';
				}

				wchar_t *abs;
				size_t fullsize = 0, converted = 0, stringsize = strlen(in.data()) + 1;

				if (in[0] == '.')
				{
					in = in.substr(1, string::npos);

					// Check if path actually exists
					abs = new wchar_t[needed_size + stringsize];
					wcscpy_s(abs, needed_size, buffer);
					wchar_t *path = new wchar_t[stringsize];

					mbstowcs_s(&converted, path, stringsize, in.data(), _TRUNCATE);
					wcscat_s(abs, needed_size + stringsize, path);
					fullsize = needed_size + stringsize;
				}
				else
				{
					abs = new wchar_t[stringsize];
					mbstowcs_s(&converted, abs, stringsize, in.data(), _TRUNCATE);
					fullsize = stringsize;
				}

				if (PathFileExists(abs) == FALSE)
				{
					cout << "File \"" << in << "\" does not exist!" << endl;
					wcout << "Tested directory  " << abs << "\n" ;
					return false;
				}

				size_t bla = 0;
				char *full = new char[fullsize];
				size_t err = wcsrtombs_s(&bla, full, fullsize, (const wchar_t **)&abs, _TRUNCATE, NULL);
				if (err == (size_t)-1)
					throw invalid_argument("Unable to construct multibyte string from wide-character string");

				in.assign(full);
			}
		}
		else if (!found_param[1] && (found = test.find("out=")) != string::npos)
		{
			found_param[1] = true;
			size_t at = found + 4;
			if ((found = test.substr(at, string::npos).find(" ")) != string::npos)
			{
				out.assign(test.substr(at, found));

				if (out.back() == '/' || out.back() == '\\')
					out.pop_back();

				// Make path windows specific
				for (size_t replace = 0; replace < out.length(); replace++)
				{
					if (out[replace] == '/')
						out[replace] = '\\';
				}

				size_t fullsize = 0, converted = 0, stringsize = strlen(out.data()) + 1;
				wchar_t *abs;

				if (out[0] == '.')
				{
					out = out.substr(1, string::npos);
					// Check if path actually exists
					abs = new wchar_t[needed_size + stringsize];
					wcscpy_s(abs, needed_size, buffer);
					wchar_t *path = new wchar_t[stringsize];

					mbstowcs_s(&converted, path, stringsize, out.data(), _TRUNCATE);
					wcscat_s(abs, needed_size + stringsize, path);
					fullsize = needed_size + stringsize;
				}
				else
				{
					abs = new wchar_t[stringsize];
					mbstowcs_s(&converted, abs, stringsize, out.data(), _TRUNCATE);
					fullsize = stringsize;
				}

				if (PathFileExists(abs) == FALSE)
				{
					cout << "Directory \"" << out << "\" does not exist!" << endl;
					wcout << "Tested directory  " << abs << "\n";
					return false;
				}

				size_t bla = 0;
				char *full = new char[fullsize];
				size_t err = wcsrtombs_s(&bla, full, fullsize, (const wchar_t **)&abs, _TRUNCATE, NULL);
				if (err == (size_t)-1)
					throw invalid_argument("Unable to construct multibyte string from wide-character string");

				out.assign(full);
			}
		}
		else if (!found_param[2] && (found = test.find("--debug")) != string::npos)
		{
			de = true;
			found_param[2] = true;
		}
		else if (!found_param[3] && (found = test.find("line=")) != string::npos)
		{
			found_param[3] = true;

			if (test[found + 5] == 'v' || test[found + 5] == 'V')
				l = false;
			else
				l = true;
		}
		else if (!found_param[4] && (found = test.find("lod=")) != string::npos)
		{
			found_param[4] = true;
			size_t at = found + 4;

			for (short i = 0; i < 15; i++)
			{
				if ((found = test.substr(at, string::npos).find("-")) != string::npos)
				{
					try
					{
						lods[i] = stoul(test.substr(at, at + found), nullptr, 10);
					}
					catch (invalid_argument)
					{
						cout << "Argument of lod parameter could not be convertred to an integer!" << endl;
						return false;
					}
					catch (out_of_range)
					{
						cout << "Argument of lod parameter was out of integer range!" << endl;
						return false;
					}
					at += found + 1;
				}
				if ((found = test.substr(at, string::npos).find(" ")) != string::npos)
					lods[15] = stoi(test.substr(at, at + found), nullptr, 10);
				else
					lods[15] = stoi(test.substr(at, string::npos), nullptr, 10);
			}
		}
		else if (!found_param[5] && (found = test.find("sort=")) != string::npos)
		{
			found_param[5] = true;

			if (test[found + 5] == 's' || test[found + 5] == 'S')
			{
				s = types::Sorting::subdivide;
			}
			else if (test[found + 5] == 'm' || test[found + 5] == 'M')
			{
				s = types::Sorting::most_nodes;
			}
			else if (test[found + 5] == 'f' || test[found + 5] == 'F')
			{
				s = types::Sorting::first_node;
			}
			else
			{
				cout << "Invalid sorting paramter value" << endl;
				return false;
			}
		}
		else if (!found_param[6] && (found = test.find("log=")) != string::npos)
		{
			found_param[6] = true;
			try
			{
				log = logging::LoglevelFromInt(stoi(test.substr(found + 4, 1), nullptr, 10));
			}
			catch (invalid_argument)
			{
				cout << "Argument of log parameter could not be convertred to an integer!" << endl;
				return false;
			}
			catch (out_of_range)
			{
				cout << "Argument of log parameter was out of integer range!" << endl;
				return false;
			}
		}
	}

	if (!found_param[0])
		return false;

	if (!found_param[1])
	{
		size_t converted = 0;
		char *full = new char[needed_size];
		size_t err = wcsrtombs_s(&converted, full, needed_size, (const wchar_t **)&buffer, _TRUNCATE, NULL);

		if (err == (size_t)-1)
			throw invalid_argument("Unable to construct multibyte string from wide-character string");
		else
			out.assign(".\\");
	}

	if (!found_param[2])
		de = false;

	if (!found_param[3])
		l = true;

	if (!found_param[4])
	{
		lods[0] = 1;
		lods[1] = 2;
		lods[2] = 4;
		lods[3] = 8;
		lods[4] = 16;
		lods[5] = 32;
		lods[6] = 64;
		lods[7] = 128;
		lods[8] = 256;
		lods[9] = 512;
		lods[10] = 1024;
		lods[11] = 2048;
		lods[12] = 4096;
		lods[13] = 8192;
		lods[14] = 16384;
		lods[15] = 32768;
	}

	if (!found_param[5])
		s = types::Sorting::first_node;

	if (!found_param[6])
		log = logging::LogLvl::error;

	return true;
}

void utility::GetUserInput(string &in, string &out, bool &de, bool &l, logging::LogLvl &log, size_t (&lods)[16], types::Sorting &s)
{
	string input;
	bool valid = false;

	PrintGreeting();
	do {
		PrintInputFormat();
		getline(cin, input);

		// Only check user input if it is not empty
		if (!input.empty())
			valid = CheckInput(input, in, out, de, l, log, lods, s);

	} while (!valid);
}
