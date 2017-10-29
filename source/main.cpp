#include <chrono>
#include "..\\header\\utility.h"
#include "..\\header\\converter.h"

using utility::GetUserInput;
using utility::ResetConsoleColor;

// in=./testing/pbf/Osnabrueck.osm.pbf out=./testing lod=0-0-0-0-0-0-0-0-0-0-0-0-1-2-4-8
// in=./testing/pbf/Berlin.osm.pbf out=./testing lod=0-0-0-0-0-0-0-0-0-0-0-0-4-8-16-32
// in=./testing/pbf/schloga.pbf out=./testing lod=0-0-0-0-0-0-0-0-0-0-0-0-0-0-2-4

// in=./testing/pbf/Osnabrueck.osm.pbf out=C:/Users/Franz_000/Studium/Bachelorarbeit/Bachelorarbeit/Bachelorarbeit/Data lod=0-0-0-0-0-0-0-0-0-0-0-0-1-2-4-8
// in=./testing/pbf/Berlin.osm.pbf out=C:/Users/Franz_000/Studium/Bachelorarbeit/Bachelorarbeit/Bachelorarbeit/Data lod=0-0-0-0-0-0-0-0-0-0-0-0-4-8-16-32
// in=./testing/pbf/schloga.pbf out=C:/Users/Franz_000/Studium/Bachelorarbeit/Bachelorarbeit/Bachelorarbeit/Data lod=0-0-0-0-0-0-0-0-0-0-0-0-0-0-2-4


int main() {
	// Input file and output directory
	string in, out;
	// Logging level [0-3]
	logging::LogLvl loglevel;
	// Sorting to use
	types::Sorting sort;
	// Create debug text file
	bool debug;
	// Which line simplification algorithm to use, true -> Douglas-Peucker, false -> Visvalingam-Whyatt
	bool line;
	// Root number of Tiles per LoD
	size_t lods[16] = { 0 };

	// Set background coloer to black and text color to white (usually the default anyway)
	ResetConsoleColor();
	// Create new parser/converter
	osmconverter::Converter parser = osmconverter::Converter();
	// Get user input from command line
	GetUserInput(in, out, debug, line, loglevel, lods, sort);
	// Set converter parameters according to user input
	parser.SetParameters(in, out, debug, line, loglevel, lods, sort);

	// Time before conversion
	std::chrono::time_point<std::chrono::system_clock> before = std::chrono::system_clock::now();
	// Parse file
	try
	{
		parser.ConvertPBF();
	}
	catch (exception e)
	{
		std::cout << "Exception occured while creating the database:" << std::endl;
		std::cout << "\t--> " << e.what() << std::endl;
	}
	// Time after conversion took place
	std::chrono::time_point<std::chrono::system_clock> after = std::chrono::system_clock::now();

	std::chrono::duration<double> sec = after - before;
	int hours = std::floor(sec.count() / 3600.0);
	int minutes = std::floor(sec.count() / 60.0) - (hours * 60);
	double seconds = sec.count() - (minutes * 60.0);

	std::cout.precision(7);
	std::cout << "Time needed for Database Construction:" << std::endl;
	std::cout << "\tHours:        " << hours << std::endl;
	std::cout << "\tMinutes:      " << minutes << std::endl;
	std::cout << "\tSeconds:      " << seconds << std::endl;

	// Clean up the protobuf library
	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}
