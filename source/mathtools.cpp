#include "..\\header\\mathtools.h"

// Triangle Implementations
mathtools::triangle::triangle()
{
	triangle(0, 0.0);
}

mathtools::triangle::triangle(size_t i, double a)
{
	index = i;
	area = a;
}

bool mathtools::operator==(const triangle & lhs, const triangle & rhs)
{
	return lhs.index == rhs.index && lhs.area == rhs.area;
}

bool mathtools::operator!=(const triangle & lhs, const triangle & rhs)
{
	return !(lhs == rhs);
}

bool mathtools::operator<(const triangle & lhs, const triangle & rhs)
{
	return lhs.area < rhs.area;
}

bool mathtools::operator<=(const triangle & lhs, const triangle & rhs)
{
	return lhs.area <= rhs.area;
}

bool mathtools::operator>(const triangle & lhs, const triangle & rhs)
{
	return lhs.area > rhs.area;
}

bool mathtools::operator>=(const triangle & lhs, const triangle & rhs)
{
	return lhs.area >= rhs.area;
}

double mathtools::triangle::Area(vec2 a, vec2 b, vec2 c)
{
	return 1 / 2 * c.PerpendicularDistance(a, b) * a.Distance(b);
}

// Vec2 Implementations
mathtools::vec2::vec2()
{
	vec2(0.0, 0.0);
}

mathtools::vec2::vec2(double a, double b)
{
	x = a;
	y = b;
}

mathtools::vec2::vec2(types::Node &n)
{
	x = n.lon;
	y = n.lat;
}

bool mathtools::vec2::operator==(const vec2 &other)
{
	return x == other.x && y == other.y;
}

mathtools::vec2 mathtools::vec2::operator-(const vec2 &v)
{
	return vec2(x - v.x, y - v.y);
}

mathtools::vec2 mathtools::vec2::operator+(const vec2 &v)
{
	return vec2(x + v.x, y + v.y);;
}

double mathtools::vec2::operator*(const vec2 &v)
{
	return x * v.x + y * v.y;
}

mathtools::vec2 mathtools::vec2::operator*(double c)
{
	return vec2(c * x, c * y);
}

double mathtools::vec2::PerpendicularDistance(vec2 &start, vec2 &end)
{
	return fabs((end.y - start.y) * x - (end.x - start.x) * y + (end.x * start.y) - (end.y * start.x)) / start.Distance(end);
}

double mathtools::vec2::Orientation(vec2 &start, vec2 &end, vec2 &at)
{
	return (end.x - start.x) * (at.y - start.y) - (end.y - start.y) * (at.x - start.x);
}

double mathtools::vec2::Distance(vec2 &other)
{
	return sqrt(pow(other.x - x, 2.0) + pow(other.y - y, 2.0));
}

double mathtools::vec2::Length()
{
	return sqrt(pow(x, 2.0) + pow(y, 2.0));
}

// Vec3 Implementations
mathtools::vec3::vec3()
{
	vec3(0.0, 0.0, 0.0);
}

mathtools::vec3::vec3(double a, double b, double c)
{
	x = a;
	y = b;
	z = c;
}

mathtools::vec3::vec3(types::Node & n, double c)
{
	x = n.lon;
	y = n.lat;
	z = c;
}

mathtools::vec3 mathtools::vec3::Cross(vec3 &other)
{
	vec3 result;

	result.x = y * other.z - z * other.y;
	result.y = z * other.x - x * other.z;
	result.z = x * other.y - y * other.x;

	return result;
}

double mathtools::vec2::Cross(vec2 &other)
{
	return vec3(x, y, 1.0).Cross(vec3(other.x, other.y, 1.0)).z;
}

double mathtools::vec3::Length()
{
	return sqrt((x * x) + (y * y) + (z * z));
}

void mathtools::vec3::Normalize()
{
	double div = Length();

	x = x / div;
	y = y / div;
	z = z / div;
}

int mathtools::vec3::Orientation(vec3 &p, vec3 &q, vec3 &r)
{
	double val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);

	// Colinear
	if (val == 0.0)
		return 0;

	// Clock-wise or Counterclock-wise
	return (val > 0.0) ? 1 : 2;
}

// Static class Implementations
short mathtools::CalculateMinLod(double left, double right, double bot, double top)
{
	double area = SquareArea(left, right, bot, top);

	if (area < 0.5) {
		return 5;
	}
	else if (area < 1.0) {
		return 4;
	}
	else if (area < 10.0) {
		return 3;
	}
	else if (area < 50.0) {
		return 2;
	}
	else if (area < 3000.0) {
		return 1;
	}

	return 0;
}

void mathtools::CalculateTilesPerLod(int(&lods)[6], short min_lod)
{
	switch (min_lod) {
	case 5:
		lods[0] = 0;
		lods[1] = 0;
		lods[2] = 0;
		lods[3] = 0;
		lods[4] = 0;
		lods[5] = 8;
		break;
	case 4:
		lods[0] = 0;
		lods[1] = 0;
		lods[2] = 0;
		lods[3] = 0;
		lods[4] = 4;
		lods[5] = 12;
		break;
	case 3:
		lods[0] = 0;
		lods[1] = 0;
		lods[2] = 0;
		lods[3] = 4;
		lods[4] = 16;
		lods[5] = 48;
		break;
	case 2:
		lods[0] = 0;
		lods[1] = 0;
		lods[2] = 4;
		lods[3] = 16;
		lods[4] = 64;
		lods[5] = 128;
		break;
	case 1:
		lods[0] = 0;
		lods[1] = 4;
		lods[2] = 16;
		lods[3] = 64;
		lods[4] = 512;
		lods[5] = 8192;
		break;
	default:
		lods[0] = 1;
		lods[1] = 2;
		lods[2] = 8;
		lods[3] = 512;
		lods[4] = 8192;
		lods[5] = 32768;
	}
}

void mathtools::GetTileBorder(vec2 &outside, vec2 &inside, types::Tile &t, vec2 &tile_start, vec2 &tile_end)
{
	// Lower left corner
	if (outside.y < t.min_lat && outside.x < t.min_lon)
	{
		if (inside.x >= (t.max_lon + t.min_lon) / 2.0)
		{
			tile_start.x = t.min_lon;
			tile_start.y = t.min_lat;
			tile_end.x = t.min_lon;
			tile_end.y = t.max_lat;
		}
		else
		{
			tile_start.x = t.min_lon;
			tile_start.y = t.min_lat;
			tile_end.x = t.max_lon;
			tile_end.y = t.min_lat;
		}
	}
	// Lower right corner
	else if (outside.y < t.min_lat && outside.x > t.max_lon)
	{
		if (inside.x >= (t.max_lon + t.min_lon) / 2.0)
		{
			tile_start.x = t.max_lon;
			tile_start.y = t.min_lat;
			tile_end.x = t.max_lon;
			tile_end.y = t.max_lat;
		}
		else
		{
			tile_start.x = t.min_lon;
			tile_start.y = t.min_lat;
			tile_end.x = t.max_lon;
			tile_end.y = t.min_lat;
		}
	}
	// Upper left corner
	else if (outside.y > t.max_lat && outside.x < t.min_lon)
	{
		if (inside.y >= (t.max_lat + t.min_lat) / 2.0)
		{
			tile_start.x = t.min_lon;
			tile_start.y = t.max_lat;
			tile_end.x = t.max_lon;
			tile_end.y = t.max_lat;
		}
		else
		{
			tile_start.x = t.min_lon;
			tile_start.y = t.min_lat;
			tile_end.x = t.min_lon;
			tile_end.y = t.max_lat;
		}
	}
	// Upper right corner
	else if (outside.y > t.max_lat && outside.x > t.max_lon)
	{
		if (inside.y >= (t.max_lat + t.min_lat) / 2.0)
		{
			tile_start.x = t.min_lon;
			tile_start.y = t.max_lat;
			tile_end.x = t.max_lon;
			tile_end.y = t.max_lat;
		}
		else
		{
			tile_start.x = t.max_lon;
			tile_start.y = t.min_lat;
			tile_end.x = t.max_lon;
			tile_end.y = t.max_lat;
		}
	}
	// Right or left line
	else if (outside.y >= t.min_lat && outside.y <= t.max_lat)
	{
		if (outside.x < t.min_lon)
		{
			tile_start.x = t.min_lon;
			tile_start.y = t.min_lat;
			tile_end.x = t.min_lon;
			tile_end.y = t.max_lat;
		}
		else if (outside.x > t.max_lon)
		{
			tile_start.x = t.max_lon;
			tile_start.y = t.min_lat;
			tile_end.x = t.max_lon;
			tile_end.y = t.max_lat;
		}
	}
	// Upper or lower line
	else if (outside.x >= t.min_lon && outside.x <= t.max_lon)
	{
		if (outside.y < t.min_lat)
		{
			tile_start.x = t.min_lon;
			tile_start.y = t.min_lat;
			tile_end.x = t.max_lon;
			tile_end.y = t.min_lat;
		}
		else if (outside.y > t.max_lat)
		{
			tile_start.x = t.min_lon;
			tile_start.y = t.max_lat;
			tile_end.x = t.max_lon;
			tile_end.y = t.max_lat;
		}
	}
}

types::Node mathtools::Intersection(types::Tile &lod_tile, types::Node &one, types::Node &two)
{
	vec2 outside = vec2(one), inside = vec2(two);
	vec2 result = vec2(), tile_start = vec2(), tile_end = vec2();

	GetTileBorder(outside, inside, lod_tile, tile_start, tile_end);

	vec2 intersect = vec2();
	if (LineLineIntersection(outside, inside, tile_start, tile_end, intersect))
		return types::Node(intersect.y, intersect.x, -2);

	/*
		std::ofstream of("intersection.txt", ios_base::app);
		of.precision(12);

		of << "Tile BBox: " << lod_tile.min_lat << ", " << lod_tile.max_lat << ", " << lod_tile.min_lon << ", " << lod_tile.max_lon << std::endl;
		of << "Tile Line: (" << tile_start.x << ", " << tile_start.y << ") (" << tile_end.x << ", " << tile_end.y << ')' << std::endl;
		of << "Points: out | in (" << outside.x << ", " << outside.y << ") (" << inside.x << ", " << inside.y << ')' << std::endl << std::endl;

		of.close();
	*/

	throw logic_error("No intersection possible");

	return types::Node();
}

bool mathtools::LineLineIntersection(vec2 & q_start, vec2 & q_end, vec2 & p_start, vec2 & p_end, vec2 &intersect)
{
	// VARIANTE 1
	/*
	vec2 r = q_end - q_start;
	vec2 s = p_end - p_start;

	double rxs = r.Cross(s);
	double pqxr = (p_start - q_start).Cross(r);

	if (fabs(rxs) < 0.0000000001 && fabs(pqxr) < 0.0000000001)
	{
		if ((0 <= (p_start - q_start) * r && (p_start - q_start)* r <= r * r) ||
			(0 <= (q_start - p_start) * s && (q_start - p_start)* s <= s * s))
			return true;

		return false;
	}

	if (fabs(rxs) < 0.0000000001 && fabs(pqxr) >= 0.0000000001)
		return false;

	double t = (p_start - q_start).Cross(s) / rxs;
	double u = (p_start - q_start).Cross(r) / rxs;

	if (fabs(rxs) >= 0.0000000001 && (t >= 0.0  && t <= 1.0) && (u >= 0.0  && u <= 1.0))
	{
		intersect = q_start + (r * t);

		return true;
	}*/

	// VARIANTE 2

	vec3 cross1 = vec3(q_start.x, q_start.y, 1.0).Cross(vec3(q_end.x, q_end.y, 1.0));
	vec3 cross2 = vec3(p_start.x, p_start.y, 1.0).Cross(vec3(p_end.x, p_end.y, 1.0));
	vec3 cut = cross1.Cross(cross2);

	ofstream of("example.txt", ios::app);

	of.precision(8);
	of << q_start.x << ", " << q_start.y << " - " << q_end.x << ", " << q_end.y << std::endl;
	of << p_start.x << ", " << p_start.y << " - " << p_end.x << ", " << p_end.y << std::endl;

	if (cut.z != 0.0)
	{
		cut.x = cut.x / cut.z;
		cut.y = cut.y / cut.z;

		double lambda1 = (cut.x - q_start.x) / (q_end.x - q_start.x);
		double lambda2 = (cut.x - p_start.x) / (p_end.x - p_start.x);

		if (fabs(lambda1) >= 0.0000000001 && fabs(lambda1) <= 1.0000000001 &&
			fabs(lambda2) >= 0.0000000001 && fabs(lambda2) <= 1.0000000001)
		{
			intersect.x = lambda1 * (q_end.x - q_start.x) + q_start.x;
			intersect.y = lambda1 * (q_end.y - q_start.y) + q_start.y;

			of << intersect.x << ", " << intersect.y << std::endl << std::endl;
			of.close();

			return true;
		}
		else
		{
			of << std::endl;
			of.close();
		}
	}


	// VARIANTE 3
	/*intersect = GetIntersectionPoint(vec3(q_start.x, q_start.y, 1.0),
								 	 vec3(q_end.x, q_end.y, 1.0),
								 	 vec3(p_start.x, p_start.y, 1.0),
								 	 vec3(p_end.x, p_end.y, 1.0));

	double minx1 = q_start.x < q_end.x ? q_start.x : q_end.x;
	double minx2 = p_start.x < p_end.x ? p_start.x : p_end.x;
	double maxx1 = q_start.x > q_end.x ? q_start.x : q_end.x;
	double maxx2 = p_start.x > p_end.x ? p_start.x : p_end.x;
	double miny1 = q_start.y < q_end.y ? q_start.y : q_end.y;
	double miny2 = p_start.y < p_end.y ? p_start.y : p_end.y;
	double maxy1 = q_start.y > q_end.y ? q_start.y : q_end.y;
	double maxy2 = p_start.y > p_end.y ? p_start.y : p_end.y;

	if (intersect.x <= maxx1 && intersect.x <= maxx2 && intersect.x >= minx1 && intersect.x >= minx2 &&
		intersect.y <= maxy1 && intersect.y <= maxy2 && intersect.y >= miny1 && intersect.y >= miny2)
		return true;
	*/

	// VARIANTE 4
	/*
	double s1_x = q_end.x - q_start.x;
	double s1_y = q_end.y - q_start.y;
	double s2_x = p_end.x - p_start.x;
	double s2_y = p_end.y - p_start.y;

	if ((-s2_x * s1_y + s1_x * s2_y) != 0.0 && (-s2_x * s1_y + s1_x * s2_y) != 0.0)
	{
		double s = (-s1_y * (q_start.x - p_start.x) + s1_x * (q_start.y - p_start.y)) / (-s2_x * s1_y + s1_x * s2_y);
		double t = (s2_x * (q_start.y - p_start.y) - s2_y * (q_start.x - p_start.x)) / (-s2_x * s1_y + s1_x * s2_y);

		if (s >= 0.0 && s <= 1.0 && t >= 0.0 && t <= 1.0)
		{
			// Compute intersection point
			intersect.x = q_start.x + (t * s1_x);
			intersect.y = q_start.y + (t * s1_y);

			return true;
		}
	}*/

	// VARIANTE 5
	// intersect.x = ((((p_end.x * p_start.y) - (p_start.x * p_end.y)) * (q_end.x - q_start.x)) - (((q_end.x * q_start.y) - (q_start.x * q_end.y)) * (p_end.x - p_start.x))) / (((p_end.x * p_start.x) - (q_end.y * q_start.y)) - ((q_end.x * q_start.x) - (p_end.y * p_start.y)));
	// intersect.y = ((((p_end.x * p_start.y) - (p_start.x * p_end.y)) * (q_end.y - q_start.y)) - (((q_end.x * q_start.y) - (q_start.x * q_end.y)) * (p_end.y - p_start.y))) / (((p_end.x * p_start.x) - (q_end.y * q_start.y)) - ((q_end.x * q_start.x) - (p_end.y * p_start.y)));

	// No intersection
	return true;
}

mathtools::vec2 mathtools::GetIntersectionPoint(vec3 &a, vec3 &b, vec3 &c, vec3 &d)
{
	vec2 returnValue = vec2();

	returnValue.x = (c.z - a.z + ((b.z - a.z) / (b.x - a.x)) * a.x - ((d.z - c.z) / (d.x - c.x)) * c.x) / (((b.z - a.z) / (b.x - a.x)) - ((d.z - c.z) / (d.x - c.x)));
	returnValue.y = ((b.z - a.z) / (b.x - a.x)) * ((c.z - a.z + ((b.z - a.z) / (b.x - a.x))*a.x - ((d.z - c.z) / (d.x - c.x)) * c.x) / (((b.z - a.z) / (b.x - a.x)) - ((d.z - c.z) / (d.x - c.x))) - a.x) + a.z;

	return returnValue;
}

double mathtools::SquareArea(double left, double right, double bot, double top)
{
	return fabs(right - left) * fabs(top - bot);
}
