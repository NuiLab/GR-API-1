#ifndef __GEOMETRY__
#define __GEOMETRY__
#include <cmath>
#include <vector>
#include <exception>
#include "Point.h"


namespace MTCircGR
{
	class Geometry
	{
	public:
		// Inline functions that compute the Squared Euclidean Distance between two points in 2D
		static float SqrEuclideanDistance(Point a, Point b) { return (a.X - b.X) * (a.X - b.X) + (a.Y - b.Y) * (a.Y - b.Y); };
		static float SqrEuclideanDistance(float x1, float y1, float x2, float y2) { return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2); };

		// Inline functions that compute the Euclidean Distance between two points in 2D
		static float EuclideanDistance(Point a, Point b) { return std::sqrt(SqrEuclideanDistance(a, b)); };
		static float EuclideanDistance(float x1, float y1, float x2, float y2) { return std::sqrt(SqrEuclideanDistance(x1, y1, x2, y2)); };
		static float AreaPolygon(std::vector<Point> points);
		static float AngleBetween(Point a, Point b);
		static float AngleBetween(float x1, float y1, float x2, float y2);
		static float AngleBetween(Point a, Point b, Point c);

		// Returns true if a point p is in circle C with center at 'center' and radius r
		static bool InCircle(Point p, Point center, float r) { return EuclideanDistance(p, center) <= r; };
		static float PointToLineDistance(Point l1, Point l2, Point x);


	};
}

#endif