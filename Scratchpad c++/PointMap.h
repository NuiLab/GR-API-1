#ifndef __POINTMAP__
#define __POINTMAP__
#include <map>
#include <vector>
#include "Point.h"



namespace MTCircGR
{
	class PointMap
	{
	public:
		PointMap();
		PointMap(std::map<int, std::vector<Point>> ptMap);
		int getMaxNumberOfPoints();
		int getNumberOfPoints();
		void Add(Point p);
		std::vector<Point> getPoints(int strokeID) { return pointMap[strokeID]; };
		PointMap getSubset(int count);
		void Clear();
		std::vector<int> getTraceIDs() { return traceIDs; };
		int getNumOfTraces() { return getTraceIDs().size(); };
		std::vector<Point> concatPoints();

	private:
		std::vector<int> traceIDs;
		std::map<int, std::vector<Point>> pointMap;
		
	};

}


#endif