#include "Gesture.h"


using namespace std;
using namespace MTCircGR;


/*
Generic Gesture Object
Consist of unprocessed traces, a name, a type, and the template name it is supposed to be
Gestures are normalized with respect to scale, translated to origin, and resampled into a 
fixed number of SAMPLING_RESOLUTION specified points.
*/


Gesture::Gesture()
{
	PointMap map = PointMap();
	rawTraces = map;
	Name = "";
	eGestureType gType = eGestureType::Template;			// NOTE default gesture type is Template
	GestureType = gType;
}


Gesture::Gesture(PointMap map, std::string gestureName, eGestureType gestureType):
	rawTraces(map),
	Name(gestureName),
	GestureType(gestureType)
{
}


Gesture::Gesture(PointMap map, std::string gestureName, eGestureType gestureType, std::string expectedAs) :
	rawTraces(map),
	Name(gestureName),
	GestureType(gestureType),
	ExpectedAs(expectedAs)
{
}


// Performs scale normalization with shape preservation into[0..1]x[0..1]
vector<Point> Gesture::Scale(vector<Point> points)
{
	float minx = FLT_MAX, miny = FLT_MAX, maxx = FLT_MIN, maxy = FLT_MIN;
	for (int i = 0; i < points.size(); i++)
	{
		if (minx > points[i].X) minx = points[i].X;
		if (miny > points[i].Y) miny = points[i].Y;
		if (maxx < points[i].X) maxx = points[i].X;
		if (maxy < points[i].Y) maxy = points[i].Y;
	}

	vector<Point> newPoints = vector<Point>();

	float scale = max(maxx - minx, maxy - miny);

	for (int i = 0; i < points.size(); i++)
		newPoints[i] = Point((points[i].X - minx) / scale, (points[i].Y - miny) / scale, points[i].StrokeID);
	return newPoints;
}


// Translates the array of points by p
vector<Point> Gesture::TranslateTo(vector<Point> points, Point p)
{
	vector<Point> newPoints = vector<Point>();
	for (int i = 0; i < points.size(); i++)
		newPoints[i] = Point(points[i].X - p.X, points[i].Y - p.Y, points[i].StrokeID);
	return newPoints;
}


// Computes the centroid for an array of points
Point Gesture::Centroid(vector<Point> points)
{
	float cx = 0, cy = 0;
	for (int i = 0; i < points.size(); i++)
	{
		cx += points[i].X;
		cy += points[i].Y;
	}
	return Point(cx / points.size(), cy / points.size(), 0);
}


// Resamples the array of points into n equally-distanced points
vector<Point> Gesture::Resample(vector<Point> points, int n)
{
	vector<Point> newPoints = vector<Point>();
	newPoints[0] = Point(points[0].X, points[0].Y, points[0].StrokeID, points[0].Timestamp);
	int numPoints = 1; //should it be zero or one?


	float I = PathLength(points) / (n - 1); // computes interval length
	if (I < 0.00001)
	{
		cout << ("***********************");
		cout << ("I is less than 0.00001");
		cout << ("Which too little points.");
		cout << ("***********************");
		return points;
	}
	float D = 0; //current step distance
	for (int i = 1; i < points.size(); i++)
	{
		if (points[i].StrokeID == points[i - 1].StrokeID)
		{
			float d = Geometry::EuclideanDistance(points[i - 1], points[i]);
			if (D + d >= I)
			{
				Point firstPoint = points[i - 1];
				while (D + d >= I)
				{
					// add interpolated point
					float t = min(max((I - D) / d, 0.0f), 1.0f);
					if (isnan(t)) t = 0.5f;
					newPoints[numPoints++] = Point(
						(1.0f - t) * firstPoint.X + t * points[i].X,
						(1.0f - t) * firstPoint.Y + t * points[i].Y,
						points[i].StrokeID,
						points[i].Timestamp
					);

					// update partial length
					d = D + d - I;
					D = 0;
					firstPoint = newPoints[numPoints - 1];
				}
				D = d;
			}
			else D += d;
		}
	}

	if (numPoints == n - 1) // sometimes we fall a rounding-error short of adding the last point, so add it if so
		newPoints[numPoints++] = Point(points[points.size() - 1].X, points[points.size() - 1].Y, points[points.size() - 1].StrokeID, points[points.size() - 1].Timestamp);
	return newPoints;
}



// Computes the path length for an array of points.
// Centroid calculation is included here to avoid calling PathLength and then 
// Centroid (this iterating over the same points twice) for every resampling of a CircGesture
float Gesture::PathLength(vector<Point> points, Point& centroid)			// VERIFY PASSING BY REFERENCE = 'OUT' MODIFIER IN C#
{
	float length = 0;
	float cx = 0, cy = 0;

	cx += points[0].X;
	cy += points[0].Y;

	for (int i = 1; i < points.size(); i++)
	{
		if (points[i].StrokeID == points[i - 1].StrokeID)
		{
			length += Geometry::EuclideanDistance(points[i - 1], points[i]);
			cx += points[i].X;
			cy += points[i].Y;

		}
	}

	centroid = Point(cx / points.size(), cy / points.size(), points[0].StrokeID);

	return length;
}



float Gesture::PathLength(vector<Point> points)
{
	Point centroid;
	return PathLength(points, centroid);
}