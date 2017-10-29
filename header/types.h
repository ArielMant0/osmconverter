#ifndef _TYPES_H_
#define _TYPES_H_

#include <vector>
#include <unordered_map>
#include <algorithm>

using namespace std;

namespace types
{
	static const double DOUBLE_EPSILON = std::numeric_limits<double>::epsilon();

	enum Sorting {
		most_nodes,
		first_node,
		subdivide
	};

	enum Member {
		node = 0,
		way,
		relation
	};

	enum MemberRole {
		outer,
		inner,
		main_stream,
		side_stream,
		street_role
	};

	enum Type {
		none,
		empty,
		apartments,
		detached,
		path,
		small_road,
		middle_road,
		large_road,
		plaza,
		green_land,
		farm_land,
		bare_land,
		forest,
		graveyard,
		boundary,
		city,
		state,
		nation,
		water,
		waterway,
		coast,
		industry,
		residential,
		tree,
		tree_row,
		lamp,
		street
	};

	static bool IsAreaType(types::Type t)
	{
		return t == apartments || t == detached || t == green_land || t == farm_land || t == bare_land
			|| t == water || t == industry || t == residential || t == graveyard || t == forest || t == plaza;
	}

	static bool IsHouseType(types::Type t)
	{
		return t == apartments || t == detached;
	}

	static bool IsRoadType(types::Type t)
	{
		return t == large_road || t == middle_road || t == small_road || t == path || t == street;
	}

	static bool IsBoundaryType(types::Type t)
	{
		return t == nation || t == state || t == city;
	}

	class OsmObject
	{
	public:
		// Area if object is a closed polygon
		virtual double Area() { return 0.0; };
		// Object Size
		virtual size_t Size() { return 1; };

		// Type-membership
		bool IsArea()
		{
			return types::IsAreaType(type);
		};
		bool IsRoad()
		{
			return types::IsRoadType(type);
		};
		bool IsHouse()
		{
			return types::IsHouseType(type);
		};
		bool IsBoundary()
		{
			return types::IsBoundaryType(type);
		};

		types::Type type;
	};

	class Role
	{
	public:

		Role();
		Role(types::MemberRole r, types::Member v, size_t p);

		types::MemberRole as;
		types::Member vec;
		size_t pos;
	};

	class Tile
	{
	public:

		Tile();
		Tile(double minlat, double maxlat, double minlon, double maxlon);

		double min_lat, max_lat, min_lon, max_lon;
		vector<size_t> way_refs, wayx_refs, solo_refs, relation_refs, relationx_refs;
	};

	class Node
	{
	public:

		Node();
		Node(const Node &other);
		Node(double lat, double lon, long long i);

		bool IsInsideTile(Tile &t);
		int AtLat(types::Tile &t);
		int AtLon(types::Tile &t);

		bool operator==(const Node &other)
		{
			return (lat == (other.lat + DOUBLE_EPSILON) || lat == (other.lat - DOUBLE_EPSILON)) &&
				(lon == (other.lon + DOUBLE_EPSILON) || lon == (other.lon - DOUBLE_EPSILON));
		}
		bool operator==(Node &other)
		{
			return (lat == (other.lat + DOUBLE_EPSILON) || lat == (other.lat - DOUBLE_EPSILON)) &&
				(lon == (other.lon + DOUBLE_EPSILON) || lon == (other.lon - DOUBLE_EPSILON));
		}

		double Distance(types::Node &other);
		double Distance(size_t index, vector<types::Node> &nodes);

		double lat, lon;
		long long id;
	};

	class NodeX : public OsmObject
	{
	public:

		NodeX();
		NodeX(size_t, types::Type);

		bool IsInsideTile(types::Tile &t, vector<types::Node> &nodes);
		int AtLat(types::Tile &t, vector<types::Node> &nodes);
		int AtLon(types::Tile &t, vector<types::Node> &nodes);

		double Distance(types::Node &other, vector<types::Node> &nodes);
		double Distance(size_t index, vector<types::Node> &nodes);

		size_t index;
	};

	class Way : public OsmObject
	{
	public:
		Way();
		Way(const Way &other);
		Way(vector<size_t> &refs, long long i, types::Type way_type);

		size_t Size();
		double Area(vector<types::Node>&);

		bool IsCircularWay();
		bool SatisfiesAreaRules();
		bool IsValid(size_t expected_size);

		void RemoveDuplicates(vector<types::Node> *nodes);

		bool IsInsideTile(types::Tile &t, vector<types::Node> &nodes, types::Sorting sort);
		int AtLat(types::Tile &t, vector<types::Node> &nodes, types::Sorting sort);
		int AtLon(types::Tile &t, vector<types::Node> &nodes, types::Sorting sort);

		void MakeClockwise(vector<types::Node> &nodes);
		bool IsCounterClockwise(vector<types::Node> &nodes);

		vector<size_t> refs;
		long long id;
	};

	class WayX : public OsmObject
	{
	public:

		WayX();
		WayX(vector<types::Node> &refs, long long i, types::Type way_type);
		// Construct WayX from normal types::Way
		WayX(types::Way &other, vector<types::Node> &nodes, unordered_map<long long, size_t> &map);

		size_t Size();
		double Area();

		bool IsCircularWay();
		bool SatisfiesAreaRules();
		bool IsValid(size_t expected_size);

		bool IsInsideTile(types::Tile &t, types::Sorting sort);
		int AtLat(types::Tile &t, types::Sorting sort);
		int AtLon(types::Tile &t, types::Sorting sort);

		void MakeClockwise();
		bool IsCounterClockwise();

		bool IsComplete();

		vector<types::Node> nodes;
		long long id;
	};

	class Relation : public OsmObject
	{
	public:

		Relation();
		Relation(vector<size_t> &wrefs, vector<types::MemberRole> &wroles, vector<types::Member> &mtypes, types::Type rtype, long long i);

		size_t Size();
		size_t NodeSize(vector<types::Way> &ways, vector<types::Relation> &relations);
		size_t MemberRelationCount();
		size_t MemberRelationSize(vector<types::Relation> &relations);

		double Area(vector<types::Node> &nodes, vector<types::Way> &ways, vector<types::Relation> &relations);
		void GetFirstLatLon(vector<types::Node> &nodes, vector<types::Way> &ways, vector<types::Relation> &relations, double &lat, double &lon);
		void GetLatLonAt(vector<types::Node> &nodes, vector<types::Way> &ways, vector<types::Relation> &relations, double &lat, double &lon, size_t index);

		bool IsInsideTile(types::Tile &t, vector<types::Node> &nodes, vector<types::Way> &ways, vector<types::Relation> &relations, types::Sorting sort);
		int AtLat(types::Tile &t, vector<types::Node> &nodes, vector<types::Way> &ways, vector<types::Relation> &relations, types::Sorting sort);
		int AtLon(types::Tile &t, vector<types::Node> &nodes, vector<types::Way> &ways, vector<types::Relation> &relations, types::Sorting sort);

		bool IsValid(size_t expected_size);
		void MakeClockwise();

		vector<size_t> refs;
		vector<types::MemberRole> roles;
		vector<types::Member> member_types;
		long long id;
	};

	class RelationX : public OsmObject
	{
	public:

		RelationX();
		RelationX(vector<types::Node> &n,
			vector<types::WayX> &w,
			vector<types::RelationX> &r,
			vector<types::Role> &role,
			long long i,
			types::Type rtype);
		// Construct RelationX from types::Relation
		RelationX(types::Relation &other,
			vector<types::Node> &n,
			unordered_map<long long, size_t> &nmap,
			vector<types::Way> &w,
			unordered_map<long long, size_t> &wmap,
			vector<types::Relation> &r,
			unordered_map<long long, size_t> &rmap);

		size_t Size();
		double Area();

		void MakeClockwise();

		bool IsInsideTile(types::Tile &t, types::Sorting sort);
		int AtLat(types::Tile &t, types::Sorting sort);
		int AtLon(types::Tile &t, types::Sorting sort);

		bool IsValid(size_t expected_size);
		bool IsComplete();

		vector<types::Node> nodes;
		vector<types::WayX> ways;
		vector<types::RelationX> relations;
		vector<types::Role> roles;
		long long id;
	};
}

#endif /* _TYPES_H_ */