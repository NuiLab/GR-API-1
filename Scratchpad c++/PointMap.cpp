#include "PointMap.h"

using namespace std;
using namespace MTCircGR;


PointMap::PointMap()
{
	pointMap = map<int, vector<Point>>();
	traceIDs = vector<int>();
}


PointMap::PointMap(map<int, vector<Point>> ptMap)
{
	pointMap = ptMap;
	traceIDs = vector<int>();

	for(auto element: ptMap)
	{
		traceIDs.push_back(element.first);
	}
}


int PointMap::getMaxNumberOfPoints()
{

	int max = INT_MIN;

	for(auto element: pointMap)
	{
		int numPts = pointMap[element.first].size();
		if (numPts > max)
			max = numPts;
	}

	return max;
	
}


int PointMap::getNumberOfPoints()
{
	int numPoints = 0;

	for(auto element: pointMap)
	{
		numPoints += pointMap[element.first].size();
	}

	return numPoints;
}


vector<Point> PointMap::concatPoints() {

	int numPoints = this->getNumberOfPoints();
	int index = numPoints;
	
	vector<Point> points(numPoints);

	for(auto element: pointMap)
	{
		vector<Point> pts = pointMap[element.first];

		for(Point p: pts) {

			points[numPoints - index] = Point(p.X, p.Y, p.StrokeID, p.Timestamp);
			index--;
		}
	}

	return points;
}


void PointMap::Add(Point p)
{
	if (pointMap.find(p.StrokeID) != pointMap.end())
	{
		pointMap.at(p.StrokeID).push_back(p);
	}
	else
	{
		pointMap.insert({p.StrokeID, vector<Point>()});
		pointMap.at(p.StrokeID).push_back(p);
		traceIDs.push_back(p.StrokeID);
	}
}


PointMap PointMap::getSubset(int count)
{
	map<int, vector<Point>> n_ptMap;

	for(auto element: pointMap)
	{
		vector<Point> pVector = pointMap[element.first];

		int elements;

		if (pVector.size() > static_cast<unsigned int>(count))				// check whether count can be considered unsigned int for comparison
			elements = count;
		else
			elements = pVector.size();


		vector<Point> pV = vector<Point>(pVector.begin(), pVector.begin() + elements);

		n_ptMap.insert({ element.first, pV });
	}

	return PointMap(n_ptMap);
}


void PointMap::Clear()			
{
	traceIDs.clear();
	pointMap.clear();
}



//public vector<Point> this[int id]					// IMPLEMENT THIS IN C++
//{
//	get
//	{
//		return pointMap[id];
//	}
//}


