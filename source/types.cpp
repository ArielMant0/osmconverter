#include "..\\header\\types.h"

namespace types
{
	// Role functions
	Role::Role()
	{
		Role(outer, node, 0);
	}

	Role::Role(types::MemberRole r, types::Member v, size_t p)
	{
		as = r;
		vec = v;
		pos = p;
	}

	// Node functions
	Node::Node()
	{
		Node(NULL, NULL, -1);
	}

	Node::Node(const Node &other)
	{
		lat = other.lat;
		lon = other.lon;
		id = other.id;
	}

	Node::Node(double latitude, double longitude, long long i)
	{
		lat = latitude;
		lon = longitude;
		id = i;
	}

	bool Node::IsInsideTile(types::Tile & t)
	{
		return AtLat(t) == 0 && AtLon(t) == 0;
	}

	int Node::AtLat(types::Tile &t)
	{
		if (lat >= t.min_lat && lat <= t.max_lat)
			return 0;
		else
			return lat > t.max_lat ? 1 : -1;
	}

	int Node::AtLon(types::Tile &t)
	{
		if (lon >= t.min_lon && lon <= t.max_lon)
			return 0;
		else
			return lon > t.max_lon ? 1 : -1;
	}

	double Node::Distance(types::Node &other)
	{
		double dist = pow(lon - other.lon, 2.0) + pow(lat - other.lat, 2.0);

		if (dist > 0.0)
			return sqrt(dist);
		else
			return 0.0;
	}

	double Node::Distance(size_t other, vector<types::Node> &nodes)
	{
		return Distance(nodes[other]);
	}

	// NodeX functions
	NodeX::NodeX()
	{
		NodeX(0, none);
	}

	NodeX::NodeX(size_t i, types::Type t)
	{
		index = i;
		type = t;
	}

	bool NodeX::IsInsideTile(types::Tile & t, vector<types::Node>& nodes)
	{
		return AtLat(t, nodes) == 0 && AtLon(t, nodes) == 0;
	}

	int NodeX::AtLat(types::Tile &t, vector<types::Node> &nodes)
	{
		return nodes[index].AtLat(t);
	}

	int NodeX::AtLon(types::Tile &t, vector<types::Node> &nodes)
	{
		return nodes[index].AtLon(t);
	}

	double NodeX::Distance(types::Node &other, vector<types::Node> &nodes)
	{
		return other.Distance(index, nodes);
	}

	double NodeX::Distance(size_t other, vector<types::Node> &nodes)
	{
		return nodes[index].Distance(other, nodes);
	}

	// Way functions
	Way::Way()
	{
		Way(vector<size_t>(), -1, none);
	}

	Way::Way(const Way &other)
	{
		id = other.id;
		refs = std::vector<size_t>(other.refs.begin(), other.refs.end());
		type = other.type;
	}

	Way::Way(vector<size_t> &references, long long i, types::Type way_type)
	{
		refs = references;
		id = i;
		type = way_type;
	}

	size_t Way::Size()
	{
		return refs.size();
	}

	double Way::Area(vector<types::Node> &nodes)
	{
		double result = 0.0;

		if (!IsArea())
			return 0.0;

		for (size_t i = 0; i < refs.size() - 1; i++)
		{
			result += fabs(nodes.at(refs[i]).lon * nodes.at(refs[i + 1]).lat - nodes.at(refs[i]).lat * nodes.at(refs[i + 1]).lon);
		}

		return result / 2;
	}

	bool Way::IsCircularWay()
	{
		return refs[0] == refs.back();
	}

	bool Way::SatisfiesAreaRules()
	{
		return IsArea() && IsCircularWay();
	}

	bool Way::IsValid(size_t expected_size)
	{
		return refs.size() == expected_size && ((refs.size() > 1 && !IsArea()) || refs.size() > 3 && IsArea());
	}

	void Way::RemoveDuplicates(vector<types::Node> *nodes)
	{
		// Remove duplicates by shifting them to the end and then erasing them
		auto last = std::unique(refs.begin(), refs.end(), [nodes](size_t one, size_t two) { return (Node)nodes->at(one) == nodes->at(two); });
		refs.erase(last, refs.end());

		// Push last point if it was erased
		if (IsArea() && refs.back() != refs[0])
			refs.push_back(refs[0]);
	}

	bool Way::IsInsideTile(types::Tile & t, vector<types::Node>& nodes, types::Sorting sort)
	{
		return AtLat(t, nodes, sort) == 0 && AtLon(t, nodes, sort) == 0;
	}

	int Way::AtLat(types::Tile &t, vector<types::Node> &nodes, types::Sorting sort)
	{
		if (sort == first_node)
		{
			return nodes[refs[0]].AtLat(t);
		}
		else
		{
			size_t inside = 0, below = 0, above = 0;
			for (size_t i = 0; i < refs.size(); i++)
			{
				int result = nodes[refs[i]].AtLat(t);
				if (result == 0)
					inside++;
				else
					result > 0 ? above++ : below++;
			}

			if (sort == most_nodes)
			{
				if (inside >= refs.size() / 9)
					return 0;
				else
					return above > below ? 1 : -1;
			}
			else
			{
				if (inside == refs.size())
					return 0;
				else
					return above > below ? 1 : -1;
			}
		}
	}

	int Way::AtLon(types::Tile & t, vector<types::Node>& nodes, types::Sorting sort)
	{
		if (sort == first_node)
		{
			return nodes[refs[0]].AtLon(t);
		}
		else
		{
			size_t inside = 0, below = 0, above = 0;
			for (size_t i = 0; i < refs.size(); i++)
			{
				int result = nodes[refs[i]].AtLon(t);
				if (result == 0)
					inside++;
				else
					result > 0 ? above++ : below++;
			}

			if (sort == most_nodes)
			{
				if (inside >= refs.size() / 9)
					return 0;
				else
					return above > below ? 1 : -1;
			}
			else
			{
				if (inside == refs.size())
					return 0;
				else
					return above > below ? 1 : -1;
			}
		}
	}

	void Way::MakeClockwise(vector<types::Node> &nodes)
	{
		if (IsCounterClockwise(nodes))
		{
			std::reverse(refs.begin(), refs.end());
		}
	}

	bool Way::IsCounterClockwise(vector<types::Node> &nodes)
	{
		double sum = 0.0;
		for (size_t i = 0; i < refs.size() - 1; i++)
		{
			sum += (nodes[refs[i + 1]].lon - nodes[refs[i]].lon) * (nodes[refs[i + 1]].lat + nodes[refs[i]].lat);
		}
		return sum < 0.0;
	}

	// WayX functions
	WayX::WayX()
	{
		WayX(vector<Node>(), -1, none);
	}

	WayX::WayX(vector<types::Node> &references, long long i, types::Type way_type)
	{
		nodes = references;
		id = i;
		type = way_type;
	}

	WayX::WayX(types::Way &other, vector<types::Node> &nodes, unordered_map<long long, size_t> &map)
	{
		id = other.id;
		type = other.type;
		nodes = vector<Node>(other.refs.size());

		for (size_t i = 0; i < other.refs.size(); i++)
		{
			// store the Node's position in our nodes vector
			std::unordered_map<long long, size_t>::iterator node_it = map.find(other.refs[i]);
			if (node_it != map.end())
				nodes.push_back(nodes.at(node_it->second));
			else
				nodes.push_back(Node(NULL, NULL, other.refs[i]));
		}
	}

	size_t WayX::Size()
	{
		return nodes.size();
	}

	double WayX::Area()
	{
		double result = 0.0;

		if (!IsArea())
			return result;

		size_t j = nodes.size() - 1;
		for (size_t i = 0; i < nodes.size() - 1; i++)
		{
			result += fabs((nodes[j].lon * nodes[i].lat) - (nodes[j].lat * nodes[i].lon));
			j = i;
		}

		return result / 2.0;
	}

	bool WayX::IsCircularWay()
	{
		return nodes[0] == nodes.back();
	}

	bool WayX::SatisfiesAreaRules()
	{
		return IsArea() && IsCircularWay();
	}

	bool WayX::IsInsideTile(types::Tile & t, types::Sorting sort)
	{
		return AtLat(t, sort) == 0 && AtLon(t, sort) == 0;
	}

	int WayX::AtLat(types::Tile &t, types::Sorting sort)
	{
		if (sort == first_node)
		{
			return nodes[0].AtLat(t);
		}
		else
		{
			int result = 0;
			size_t inside = 0, below = 0, above = 0;
			for (size_t i = 0; i < nodes.size(); i++)
			{
				result = nodes[i].AtLat(t);
				if (result == 0)
					inside++;
				else
					result > 0 ? above++ : below++;
			}

			if (sort == most_nodes)
			{
				if (inside >= nodes.size() / 9)
					return 0;
				else
					return above > below ? 1 : -1;
			}
			else
			{
				if (inside == nodes.size())
					return 0;
				else
					return above > below ? 1 : -1;
			}
		}
	}

	int WayX::AtLon(types::Tile &t, types::Sorting sort)
	{
		if (sort == first_node)
		{
			return nodes[0].AtLon(t);
		}
		else
		{
			int result = 0;
			size_t inside = 0, below = 0, above = 0;
			for (size_t i = 0; i < nodes.size(); i++)
			{
				result = nodes[i].AtLon(t);
				if (result == 0)
					inside++;
				else
					result > 0 ? above++ : below++;
			}

			if (sort == most_nodes)
			{
				if (inside >= nodes.size() / 9)
					return 0;
				else
					return above > below ? 1 : -1;
			}
			else
			{
				if (inside == nodes.size())
					return 0;
				else
					return above > below ? 1 : -1;
			}
		}
	}

	void WayX::MakeClockwise()
	{
		if (IsCounterClockwise())
		{
			std::reverse(nodes.begin(), nodes.end());
		}
	}

	bool WayX::IsCounterClockwise()
	{
		double sum = 0.0;
		for (size_t i = 0; i < nodes.size() - 1; i++)
		{
			sum += (nodes[i + 1].lon - nodes[i + 1].lon) * (nodes[i + 1].lat + nodes[i + 1].lat);
		}
		return sum < 0.0;
	}

	bool WayX::IsComplete()
	{
		return std::all_of(nodes.begin(), nodes.end(), [](Node &n) { return n.lat != NULL && n.lon != NULL; });
	}

	// Relation functions
	Relation::Relation()
	{
		Relation(vector<size_t>(), vector<MemberRole>(), vector<Member>(), none, -1);
	}

	Relation::Relation(std::vector<size_t> &wrefs, vector<types::MemberRole> &wroles, vector<types::Member> &mtypes, types::Type rtype, long long i)
	{
		refs = wrefs;
		roles = wroles;
		member_types = mtypes;
		type = rtype;
		id = i;
	}

	size_t Relation::Size()
	{
		return refs.size();
	}

	size_t Relation::NodeSize(std::vector<types::Way> &ways, vector<types::Relation> &relations)
	{
		size_t result = 0;

		for (size_t i = 0; i < refs.size(); i++)
		{
			if (member_types[i] == way)
				result += ways[refs[i]].Size();
			else if (member_types[i] == relation)
				result += relations[refs[i]].NodeSize(ways, relations);
			else
				result++;
		}

		return result;
	}

	size_t Relation::MemberRelationCount()
	{
		return std::count(member_types.begin(), member_types.end(), Member::relation);
	}

	size_t Relation::MemberRelationSize(vector<types::Relation>& relations)
	{
		size_t result = 0;
		for (size_t i = 0; i < refs.size(); i++)
		{
			if (member_types[i] == relation)
			{
				for (size_t j = 0; j < relations[refs[i]].refs.size(); j++)
				{
					switch (relations[refs[i]].member_types[j])
					{
					case node: case way: result++;	break;
					case relation: result += relations[relations[refs[i]].refs[j]].MemberRelationSize(relations); break;
					}
				}
			}
		}
		return result;
	}

	double Relation::Area(vector<types::Node> &nodes, vector<types::Way> &ways, vector<types::Relation> &relations)
	{
		bool outer_seperate = true, inner_seperate = true;
		double outer = 0, inner = 0;
		vector<size_t> tmp_outer, tmp_inner;
		tmp_inner = tmp_outer = vector <size_t>();

		if (!IsArea())
			return 0.0;

		for (size_t i = 0; i < refs.size(); i++)
		{
			// If this is an outer polygon add it to outer
			if (roles[i] == outer)
			{
				switch (member_types[i])
				{
				case way:
				{
					// If the member is a closed polygon just add its area
					if (ways[refs[i]].refs[0] == ways[refs[i]].refs.back())
					{
						if (outer_seperate)
						{
							outer += ways[refs[i]].Area(nodes);
						}
						else
						{
							// If we get here we previously found an unclosed member with the
							// role 'inner', so this means that polygon should now be complete
							// and we can add its area to the inner area
							outer_seperate = true;
							if (!tmp_inner.empty())
								inner += Way(tmp_inner, -1, none).Area(nodes);
							tmp_inner.clear();
						}
					}
					// If the member is just part of a polygon push all member Node indices until
					// we encounter members with another role or all members are read
					else
					{
						outer_seperate = false;
						for (size_t j = 0; j < ways[refs[i]].refs.size(); j++)
						{
							tmp_outer.push_back(ways[refs[i]].refs[j]);
						}
					}
				} break;
				case relation:
				{
					// Recursive call
					outer += relations[refs[i]].Area(nodes, ways, relations);
				}
				}
			}
			// If it is an inner polygon add it to inner
			else
			{
				switch (member_types[i])
				{
				case way:
				{
					// If the member is a closed polygon just add its area
					if (ways[refs[i]].refs[0] == ways[refs[i]].refs.back())
					{
						if (inner_seperate)
						{
							inner += ways[refs[i]].Area(nodes);
						}
						else
						{
							// If we get here we previously found an unclosed member with the
							// role 'outer', so this means that polygon should now be complete
							// and we can add its area to the outer area
							inner_seperate = true;
							if (!tmp_outer.empty())
								outer += Way(tmp_outer, -1, none).Area(nodes);
							tmp_outer.clear();
						}
					}
					// If the member is just part of a polygon push all member Node indices until
					// we encounter members with another role or all members are read
					else
					{
						inner_seperate = false;
						for (size_t j = 0; j < ways[refs[i]].refs.size(); j++)
						{
							tmp_inner.push_back(ways[refs[i]].refs[j]);
						}
					}
				} break;
				case relation:
				{
					// Recursive call
					inner += relations[refs[i]].Area(nodes, ways, relations);
				}
				}
			}
		}
		// If the last member was made up of unclosed members add
		// the resulting areas to outer or inner respectively
		if (!tmp_outer.empty())
			outer += Way(tmp_outer, -1, none).Area(nodes);
		if (!tmp_inner.empty())
			inner += Way(tmp_inner, -1, none).Area(nodes);
		// Return outer area subtracted by the inner area
		return outer - inner;
	}

	void Relation::GetFirstLatLon(vector<types::Node> &nodes, vector<types::Way> &ways, vector<types::Relation> &relations, double &lat, double &lon)
	{
		GetLatLonAt(nodes, ways, relations, lat, lon, 0);
	}

	void Relation::GetLatLonAt(vector<types::Node> &nodes, vector<types::Way> &ways, vector<types::Relation> &relations, double &lat, double &lon, size_t index)
	{
		if (index >= refs.size())
			throw logic_error("Invalid index in GetLatLonAt()");

		switch (member_types[index])
		{
			case node:
			{
				lat = nodes[refs[index]].lat;
				lon = nodes[refs[index]].lon;
			} break;
			case way:
			{
				lat = nodes[ways[refs[index]].refs[0]].lat;
				lon = nodes[ways[refs[index]].refs[0]].lon;
			} break;
			case relation:
			{
				relations[refs[index]].GetFirstLatLon(nodes, ways, relations, lat, lon);
			} break;
		}
	}

	bool Relation::IsInsideTile(types::Tile &t, vector<types::Node>& nodes, vector<types::Way>& ways, vector<types::Relation>& relations, types::Sorting sort)
	{
		return AtLat(t, nodes, ways, relations, sort) == 0 && AtLon(t, nodes, ways, relations, sort) == 0;
	}

	int Relation::AtLat(types::Tile &t, vector<types::Node> &nodes, vector<types::Way> &ways, vector<types::Relation> &relations, types::Sorting sort)
	{
		if (sort == first_node)
		{
			switch (member_types[0])
			{
				case node: return nodes[refs[0]].AtLat(t); break;
				case way: return ways[refs[0]].AtLat(t, nodes, sort); break;
				case relation: return relations[refs[0]].AtLat(t, nodes, ways, relations, sort); break;
			}
		}
		else
		{
			int result = 0;
			size_t inside = 0, below = 0, above = 0;
			for (size_t i = 0; i < refs.size(); i++)
			{
				switch (member_types[i])
				{
					case node: result = nodes[refs[i]].AtLat(t); break;
					case way: result = ways[refs[i]].AtLat(t, nodes, sort); break;
					case relation: result = relations[refs[i]].AtLat(t, nodes, ways, relations, sort); break;
				}

				if (result == 0)
					inside++;
				else
					result > 0 ? above++ : below++;
			}
			if (sort == most_nodes)
			{
				if (inside >= refs.size() / 9)
					return 0;
				else
					return above > below ? 1 : -1;
			}
			else
			{
				if (inside == refs.size())
					return 0;
				else
					return above > below ? 1 : -1;
			}
		}
		// Shut up the compiler
		return 0;
	}

	int Relation::AtLon(types::Tile &t, vector<types::Node>& nodes, vector<types::Way>& ways, vector<types::Relation>& relations, types::Sorting sort)
	{
		if (sort == first_node)
		{
			switch (member_types[0])
			{
				case node: return nodes[refs[0]].AtLon(t); break;
				case way: return ways[refs[0]].AtLon(t, nodes, sort); break;
				case relation: return relations[refs[0]].AtLon(t, nodes, ways, relations, sort); break;
			}
		}
		else
		{
			int result = 0;
			size_t inside = 0, below = 0, above = 0;
			for (size_t i = 0; i < refs.size(); i++)
			{
				switch (member_types[i])
				{
					case node: result = nodes[refs[i]].AtLon(t); break;
					case way: result = ways[refs[i]].AtLon(t, nodes, sort); break;
					case relation: result = relations[refs[i]].AtLon(t, nodes, ways, relations, sort); break;
				}

				if (result == 0)
					inside++;
				else
					result > 0 ? above++ : below++;
			}
			if (sort == most_nodes)
			{
				if (inside >= refs.size() / 9)
					return 0;
				else
					return above > below ? 1 : -1;
			}
			else
			{
				if (inside == refs.size())
					return 0;
				else
					return above > below ? 1 : -1;
			}
		}
		// Shut up the compiler
		return 0;
	}

	bool Relation::IsValid(size_t expected_size)
	{
		return !refs.empty() && refs.size() == expected_size && refs.size() == roles.size() && refs.size() == member_types.size();
	}

	void Relation::MakeClockwise()
	{
		std::reverse(refs.begin(), refs.end());
		std::reverse(roles.begin(), roles.end());
		std::reverse(member_types.begin(), member_types.end());
	}

	// RelationX functions
	RelationX::RelationX()
	{
		RelationX(vector<Node>(), vector<WayX>(), vector<RelationX>(), vector<Role>(), -1, none);
	}

	RelationX::RelationX(vector<types::Node> &n,
		vector<types::WayX> &w,
		vector<types::RelationX> &r,
		vector<types::Role> &role,
		long long i,
		types::Type rtype)
	{
		nodes = n;
		ways = w;
		relations = r;
		roles = role;
		type = rtype;
		id = i;
	}

	RelationX::RelationX(types::Relation &other,
		vector<types::Node> &n,
		unordered_map<long long, size_t> &nmap,
		vector<types::Way> &w,
		unordered_map<long long, size_t> &wmap,
		vector<types::Relation> &r,
		unordered_map<long long, size_t> &rmap)
	{
		nodes = vector<Node>(other.refs.size() / 3);
		ways = vector<WayX>(other.refs.size() / 3);
		relations = vector<RelationX>(other.refs.size() / 3);

		for (size_t i = 0; i < other.refs.size(); i++)
		{
			switch (other.member_types[i])
			{
				case node: nodes.push_back(n[other.refs[i]]); break;
				case way: ways.push_back(WayX(w[other.refs[i]], n, nmap)); break;
				case relation: relations.push_back(RelationX(r[other.refs[i]], n, nmap, w, wmap, r, rmap)); break;
			}
		}
	}

	size_t RelationX::Size()
	{
		return nodes.size() + ways.size() + relations.size();
	}

	double RelationX::Area()
	{
		double result = 0.0;

		if (!IsArea())
			return 0.0;

		for (size_t i = 0; i < ways.size(); i++)
		{
			if (ways[i].IsArea())
			{
				result += ways[i].Area();
			}
		}

		for (size_t i = 0; i < relations.size(); i++)
		{
			if (relations[i].IsArea())
			{
				result += relations[i].Area();
			}
		}

		return result;
	}

	void RelationX::MakeClockwise()
	{
		std::reverse(roles.begin(), roles.end());
	}

	bool RelationX::IsInsideTile(types::Tile & t, types::Sorting sort)
	{
		return AtLat(t, sort) == 0 && AtLon(t, sort) == 0;
	}

	int RelationX::AtLat(types::Tile & t, types::Sorting sort)
	{
		if (sort == first_node)
		{
			switch (roles[0].as)
			{
				case node: return nodes[0].AtLat(t); break;
				case way: return ways[0].AtLat(t, sort); break;
				case relation: return relations[0].AtLat(t, sort); break;
			}
		}
		else
		{
			size_t inside = 0, below = 0, above = 0;
			for (size_t i = 0; i < nodes.size(); i++)
			{
				int result = nodes[i].AtLat(t);
				if (result == 0)
					inside++;
				else
					result > 0 ? above++ : below++;
			}

			for (size_t i = 0; i < ways.size(); i++)
			{
				int result = ways[i].AtLat(t, sort);
				if (result == 0)
					inside++;
				else
					result > 0 ? above++ : below++;
			}

			for (size_t i = 0; i < relations.size(); i++)
			{
				int result = relations[i].AtLat(t, sort);
				if (result == 0)
					inside++;
				else
					result > 0 ? above++ : below++;
			}

			if ((sort == most_nodes && inside >= above && inside >= below) ||
				(sort == subdivide && inside == (nodes.size() + ways.size() + relations.size())))
				return 0;
			else
				return above > below ? 1 : -1;
		}
		// Shut up the compiler
		return 0;
	}

	int RelationX::AtLon(types::Tile & t, types::Sorting sort)
	{
		if (sort == first_node)
		{
			switch (roles[0].as)
			{
			case node: return nodes[0].AtLon(t); break;
			case way: return ways[0].AtLon(t, sort); break;
			case relation: return relations[0].AtLon(t, sort); break;
			}
		}
		else
		{
			size_t inside = 0, below = 0, above = 0;
			for (size_t i = 0; i < nodes.size(); i++)
			{
				int result = nodes[i].AtLon(t);
				if (result == 0)
					inside++;
				else if (result < 0)
					below++;
				else
					above++;

				if (sort == subdivide && inside > 0)
					return 0;
			}

			for (size_t i = 0; i < ways.size(); i++)
			{
				int result = ways[i].AtLon(t, sort);
				if (result == 0)
					inside++;
				else if (result < 0)
					below++;
				else
					above++;

				if (sort == subdivide && inside > 0)
					return 0;
			}

			for (size_t i = 0; i < relations.size(); i++)
			{
				int result = relations[i].AtLon(t, sort);
				if (result == 0)
					inside++;
				else if (result < 0)
					below++;
				else
					above++;

				if (sort == subdivide && inside > 0)
					return 0;
			}

			if ((sort == most_nodes && inside >= above && inside >= below) ||
				(sort == subdivide && inside == (nodes.size() + ways.size() + relations.size())))
				return 0;
			else
				return above > below ? 1 : -1;
		}
		// Shut up the compiler
		return 0;
	}

	bool RelationX::IsComplete()
	{
		return std::all_of(nodes.begin(), nodes.end(), [](Node &n) { return n.lat != NULL && n.lon != NULL; }) &&
			   std::all_of(ways.begin(), ways.end(), [](WayX &w) { return w.IsComplete(); }) &&
			   std::all_of(relations.begin(), relations.end(), [](RelationX &r) { return r.IsComplete(); });
	}

	// Tile functions
	Tile::Tile()
	{
		Tile(0.0, 0.0, 0.0, 0.0);
	}

	Tile::Tile(double minlat, double maxlat, double minlon, double maxlon)
	{
		min_lat = minlat;
		max_lat = maxlat;
		min_lon = minlon;
		max_lon = maxlon;
		way_refs = vector<size_t>();
		wayx_refs = vector<size_t>();
		solo_refs = vector<size_t>();
		relation_refs = vector<size_t>();
		relationx_refs = vector<size_t>();
	}
}
