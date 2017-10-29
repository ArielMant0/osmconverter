#ifndef _MATHTOOLS_H_
#define _MATHTOOLS_H_

#include <cmath>
#include <fstream>
#include "..\\header\\types.h"

namespace mathtools
{
	class vec2 {
	public:
		double x, y;

		vec2 ();
		vec2 (double, double);
		vec2 (types::Node &n);

		bool operator==(const vec2&);
		vec2 operator-(const vec2&);
		vec2 operator+(const vec2&);
		double operator*(const vec2&);
		vec2 operator*(double);

		double Length();
		double Cross(vec2&);
		double Distance(vec2&);
		double PerpendicularDistance(vec2 &start, vec2 &end);
		static double Orientation(vec2&, vec2&, vec2&);
	};

	class vec3 {
	public:
		double x, y, z;

		vec3();
		vec3(double a, double b, double c);
		vec3(types::Node& n, double c);

		vec3 Cross(vec3&);
		double Length();
		void Normalize();
		static int Orientation(vec3&, vec3&, vec3&);
	};

	class triangle {
	public:
		size_t index;
		double area;

		triangle();
		triangle(size_t i, double a);

		friend bool operator==(const triangle& lhs, const triangle& rhs);
		friend bool operator!=(const triangle& lhs, const triangle& rhs);
		friend bool operator<(const triangle& lhs, const triangle& rhs);
		friend bool operator<=(const triangle& lhs, const triangle& rhs);
		friend bool operator>(const triangle& lhs, const triangle& rhs);
		friend bool operator>=(const triangle& lhs, const triangle& rhs);

		static double Area(vec2, vec2, vec2);
	};

	short CalculateMinLod(double left, double right, double bot, double top);
	//short CalculateMaxLod(double left, double right, double bot, double top);
	types::Node Intersection(types::Tile&, types::Node&, types::Node&);
	bool LineLineIntersection(vec2& q_start, vec2& q_end, vec2& p_start, vec2& p_end, vec2& intersect);
	mathtools::vec2 GetIntersectionPoint(vec3 & a, vec3 & b, vec3 & c, vec3 & d);
	void GetTileBorder(vec2 &, vec2 &, types::Tile&, vec2&, vec2&);
	void CalculateTilesPerLod(int(&)[6], short min_lod);
	double SquareArea(double left, double right, double bot, double top);
}

#endif /* _MATHTOOLS_H_ */
