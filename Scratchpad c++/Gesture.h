#ifndef __GESTURE__
#define __GESTURE__
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "Point.h"
#include "PointMap.h"
#include "Geometry.h"


namespace MTCircGR
{
	class Gesture
	{

	public:
		int getNumOfTraces() { return rawTraces.getNumOfTraces(); };
		int getNumOfPoints() { return rawTraces.getNumberOfPoints(); };
		enum eGestureType {
			Candidate,	//an unclassified gesture that is coming in
			Template,	//a gesture that is to be used to define the gesture set
			Example		//an example of a template gesture
		};
		std::string Name = "";          // Label for the gesture 
		std::string ExpectedAs = "";	//The expected Template
		static const int SAMPLING_RESOLUTION = 64;
		eGestureType GestureType;		//legacy enum, might not have use
		PointMap rawTraces;				//traces without any preprocessing
		Gesture();
		Gesture(PointMap map, std::string gestureName, eGestureType gestureType);
		Gesture(PointMap map, std::string gestureName, eGestureType gestureType, std::string expectedAs);
		std::vector<Point> Scale(std::vector<Point> points);
		std::vector<Point> Resample(std::vector<Point> points, int n);
		static float PathLength(std::vector<Point> points, Point& centroid);
		static float PathLength(std::vector<Point> points);
		PointMap getPointMap() { return rawTraces; };

	protected:
		static std::vector<Point> TranslateTo(std::vector<Point> points, Point p);
		static Point Centroid(std::vector<Point> points);



	};

}

#endif