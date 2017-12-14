#include "Geometry.h"


using namespace std;
using namespace MTCircGR;

// Computes the Area of any Irregular (or regular) Polygon
float Geometry::AreaPolygon(vector<Point> points)
{
	//last point must be the same as the first point
	float areaP = 0;
	if (points.empty())
		throw exception("points cannot be null");			

	int k = points.size() - 1;
	for (int i = 0; i < k; ++i)
		areaP += points[i].X * points[i + 1].Y - points[i + 1].X * points[i].Y;

	areaP += points[k].X * points[0].Y - points[0].X * points[k].Y;

	return areaP / 2.0f; ;
}


// Computes the angle between Point a and b (as position vectors), assuming the tail of both is at the origin
float Geometry::AngleBetween(Point a, Point b)
{

	Point origin = Point(0.0f, 0.0f);

	float aMagnitude = EuclideanDistance(a, origin);
	float bMagnitude = EuclideanDistance(b, origin);

	float dot = (a.X * b.X) + (a.Y * b.Y);

	return static_cast<float>(acos(dot / (aMagnitude * bMagnitude)));
}


float Geometry::AngleBetween(float x1, float y1, float x2, float y2)
{
	Point origin = Point(0.0f, 0.0f);

	float aMagnitude = EuclideanDistance(x1, y1, 0, 0);

	float bMagnitude = EuclideanDistance(x2, y2, 0, 0);

	float dot = (x1 * x2) + (y1 * y2);

	return static_cast<float>(acos(dot / (aMagnitude * bMagnitude)));
}


// Compute the angle between vector a and b, assuming the tail of both is at point c
float Geometry::AngleBetween(Point a, Point b, Point c)
{
	float aMagnitude = EuclideanDistance(a, c);
	float bMagnitude = EuclideanDistance(b, c);

	float dot = (a.X * b.X) + (a.Y * b.Y);
	float angle = static_cast<float>(acos(dot / (aMagnitude * bMagnitude)));
	return angle;
}


// If a line segment is defined by two points, l1 and l2, this method returns the distance
// from a point x, to that line segment.
float Geometry::PointToLineDistance(Point l1, Point l2, Point x)
{
	float denominator = EuclideanDistance(l1, l2);

	float numerator = ((l2.Y - l1.Y) * x.X) - ((l2.X - l1.X) * x.Y) + (l2.X * l1.Y) - (l2.Y * l1.X);

	numerator = abs(numerator);

	return numerator / denominator;

}