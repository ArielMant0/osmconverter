#ifndef _CONVERTER_H_
#define _CONVERTER_H_

#ifdef USE_X86
#define OUTDIR L"\\PBFConversion_x86"
#else
#define OUTDIR L"\\PBFConversion_x64"
#endif

#define OUTDIR_LEN 18
#define START 0.000000001

#define C_MAX_LOD (short)15
#define C_MIN_LOD (short)0

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

///////////////////////////////////////////////////////
// External Includes
///////////////////////////////////////////////////////
#include <zlib.h>
#include <exception>
#include <aclapi.h>
#include <fstream>
#include <queue>
#include <array>
#include <functional>
//#include <AccCtrl.h>

///////////////////////////////////////////////////////
// PBF Includes
///////////////////////////////////////////////////////
#include "..\\header\\fileformat.pb.h"
#include "..\\header\\osmformat.pb.h"

///////////////////////////////////////////////////////
// My Includes
///////////////////////////////////////////////////////
#include "..\\header\\utility.h"
#include "..\\header\\mathtools.h"

using namespace google::protobuf;

namespace osmconverter {

	class data_error : public logic_error
	{
	public:
		explicit data_error(const string& what) : logic_error(what) {};
		explicit data_error(const char *what) : logic_error(what) {};
	};

	class io_error : public runtime_error
	{
	public:
		explicit io_error(const string& what) : runtime_error(what) {};
		explicit io_error(const char *what) : runtime_error(what) {};
	};

	class Converter
	{

	public:

		///////////////////////////////////////////////////////
		// Constants
		///////////////////////////////////////////////////////

		static const int MAX_BLOB_HEADER_SIZE = 64 * 1024;
		static const int MAX_UNCOMPRESSED_BLOB_SIZE = 32 * 1024 * 1024;
		static const int LONLAT_RESOLUTION = 1000 * 1000 * 1000;

		// Max sizes for different vectors
		static const size_t MAX_NODES = 11958657;
		static const size_t MAX_WAYS = 1574803;
		static const size_t MAX_RELATIONS = 1000000;

		static const size_t NODE_SIZE = sizeof(long long) + 2 * sizeof(double);
		static const size_t MEAN_WAYSIZE = sizeof(long long) + 50 * sizeof(double) + sizeof(int);
		static const size_t MEAN_RELSIZE = sizeof(long long) + 50 * MEAN_WAYSIZE + sizeof(int);

		///////////////////////////////////////////////////////
		// Public Functions
		///////////////////////////////////////////////////////

		Converter();

		void ConvertPBF();

		void SetParameters(string in, string out, bool, bool, logging::LogLvl, size_t[16], types::Sorting);
		void SetSorting(types::Sorting);
		void SetLoDs(size_t[16]);
		void SetLoggingLevel(logging::LogLvl);

	private:

		enum WayOrientation {
			UpperLeft,
			Up,
			UpperRight,
			Left,
			Right,
			LowerLeft,
			Down,
			LowerRight
		};

		class Extrema {
		public:

			Extrema()
			{
				Extrema(0.0, 0.0, 0);
			}

			Extrema(types::Node &n, size_t i)
			{
				Extrema(n.lat, n.lon, i);
			}

			Extrema(double latitude, double longitude, size_t i)
			{
				lat = latitude;
				lon = longitude;
				index = i;
			}

			void Update(types::Node &n, size_t i)
			{
				lat = n.lat;
				lon = n.lon;
				index = i;
			}

			void Update(double latitude, double longitude, size_t i)
			{
				lat = latitude;
				lon = longitude;
				index = i;
			}

			double lat, lon;
			size_t index;
		};

		///////////////////////////////////////////////////////
		// Private Functions
		///////////////////////////////////////////////////////

		void SetOutputDirectory(string);
		void ClearDirectory();

		void CleanUp();

		// Reading different data types
		bool ReadNodes(const RepeatedPtrField<std::string>&, OSMPBF::PrimitiveBlock&, OSMPBF::PrimitiveGroup&);
		bool ReadDenseNodes(const RepeatedPtrField<std::string>&, OSMPBF::PrimitiveBlock&, OSMPBF::PrimitiveGroup&);
		bool ReadWays(const RepeatedPtrField<std::string>&, OSMPBF::PrimitiveBlock&, OSMPBF::PrimitiveGroup&);
		bool ReadRelations(const RepeatedPtrField<std::string>&, OSMPBF::PrimitiveBlock&, OSMPBF::PrimitiveGroup&);
		types::Type AssignType(const RepeatedPtrField<std::string>&, RepeatedField<uint32>&, RepeatedField<uint32>&);
		types::Type AssignRelationType(const RepeatedPtrField<std::string>&, RepeatedField<uint32>&, RepeatedField<uint32>&);

		// Control flow
		bool CheckRestart(types::Member, size_t, size_t bytesize);
		void SetReadType(types::Member, bool);

		// Storing incomplete objects
		void AddWayForLaterUse(types::Way&, vector<long long>&);
		types::WayX WayXFromWay(types::Way&);
		void AddRelationForLaterUse(types::Relation&, vector<long long>&);
		void RelationXFromRelation(size_t);

		// Preparing and Writing Output
		bool InitTile(short lod, size_t &start, size_t &end);
		void CleanOutData();

		// Tile-Membership
		size_t FindTile(size_t object_index, types::Member mem);
		void GetLatLonForSearch(size_t object_index, types::Member mem, double &lat, double &lon);

		// Overflow flag
		void SetOverflow();
		bool GetOverflow();

		// Metafile
		void WriteMetaFile(short);

		// Data-Output
		void WriteDataToFile(short);
		void UpdateDatabase(short);
		void TransferRelation(FILE *from, FILE *to);
		void TransferMemberRelation(FILE *from, FILE *to, size_t size);
		void WriteNode(FILE*, size_t, bool);
		void WriteWay(FILE*, size_t, bool);
		void WriteWayX(FILE*, size_t, bool);
		void WriteRelation(FILE*, size_t, bool);
		void WriteMemberWay(FILE*, size_t, bool);
		void WriteMemberRelation(FILE*, size_t, bool);
		void WriteRelationX(FILE*, size_t, bool);
		void WriteXMemberWay(FILE*, size_t, size_t, bool);
		void WriteXMemberRelation(FILE*, size_t, size_t, bool);

		// Filenames
		string GetDataFilename(short lod);
		string GetLookupFilename(short lod);

		// Data Generalization
		size_t SubdivideLine(types::Tile&, types::Way&);
		size_t SubdivideArea(types::Tile&, types::Way&);
		size_t Subdivide(types::Tile&, types::Way&, bool);

		bool MergeAreas(short lod, types::Way &at, types::Way &other, std::vector<size_t>&);
		WayOrientation GetWayOrientation(Extrema[4], Extrema[4]);
		bool CloseCoordinates(Converter::Extrema &one, Converter::Extrema &two);
		void GetNewExtrema(Converter::WayOrientation o, Converter::Extrema &one, types::Way &wone, Converter::Extrema &two, types::Way &wtwo);
		std::vector<size_t> Merge(std::vector<size_t> &first, std::vector<size_t> &second, size_t onemin, size_t onemax, size_t twomin, size_t twomax);

		void ConstructConvexHull(std::vector<size_t> &points, size_t index);
		double IsLeft(size_t start, size_t end, size_t at);

		std::vector<size_t> DouglasPeucker(std::vector<size_t>, double);
		std::vector<size_t> VisvalingamWhyatt(std::vector<size_t>&, size_t);
		double PerpendicularDistance(std::vector<size_t> &polygon, size_t current);

		bool IsLoDType(short, types::Type);
		double GetLoDEpsilon(short);
		double GetLoDAreaSize(short);
		size_t GetLoDPercentage(short, size_t);

		// Way Generalization
		void GeneralizeWays(std::vector<types::Way>&, size_t, short);
		void SortWays(short lod, std::vector<types::Way>&);

		// Relation Generalization
		void GeneralizeRelations(std::vector<types::Relation>&, size_t, short);
		void SortRelations(short lod, std::vector<types::Relation>&);
		void SubdivideRelation(types::Tile&, types::Relation&);

		// This is kinda redundant, change?
		// Leftover Way Generalization
		void SortLeftoverWays(short lod);
		void UpdateLeftoverWays();
		bool LeftoverWayInTile(types::Tile&, size_t, size_t);
		std::vector<types::WayX*> SubdivideLine(types::Tile*, types::WayX*);
		std::vector<types::WayX*> SubdivideArea(types::Tile*, types::WayX*);
		void MarkLeftoverWays(std::vector<types::WayX>&, types::Tile&, size_t, size_t);

		// Leftover Relation Generalization
		void SortLeftoverRelations(short lod);
		void UpdateLeftoverRelations();

		///////////////////////////////////////////////////////
		// Member Variables
		///////////////////////////////////////////////////////

		// Meta info
		size_t m_way_count, m_relation_count, m_tilecount;
		// LoD tile number roots
		size_t m_lods[16];
		// Input and output locations
		std::string m_input, m_output;
		// Flags
		bool m_debug, m_line, m_overflow, m_update, m_read_type[3];
		// Sorting
		types::Sorting m_sort;
		// Current tile's lat and lon step
		double m_lat_step, m_lon_step;
		// Bounding Box
		double m_minlat, m_maxlat, m_minlon, m_maxlon;
		// Version
		int m_major, m_minor, m_patch;

		// Used for informational output
		logging::Logger logger;

		// Nodes and single objects
		std::vector<types::Node> m_nodes;
		std::vector<types::NodeX> m_singles;
		std::unordered_map<long long, size_t> m_node_map;

		// Ways and incomplete ways
		std::vector<types::Way> m_ways;
		std::vector<types::WayX> m_ways_left;
		std::unordered_map<long long, size_t> m_way_map;
		std::unordered_map<long long, size_t> m_ways_left_map;

		// Relations and incomplete relations
		std::vector<types::Relation> m_relations;
		std::vector<types::RelationX> m_rels_left;
		std::unordered_map<long long, size_t> m_rel_map;
		std::unordered_map<long long, size_t> m_rels_left_map;

		// All tiles of the current LoD
		std::vector<types::Tile> m_tiles;
	};
}

#endif /* _CONVERTER_H_ */
