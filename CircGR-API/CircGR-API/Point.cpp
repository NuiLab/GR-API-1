#include "Point.h"

using namespace std;
using namespace MTCircGR;

/*
	Implements a 2D Point that exposes X, Y, and StrokeID properties.
    StrokeID is the stroke index the point belongs to (e.g., 0, 1, 2, ...) 
	that is filled by counting pen down/up events.
*/


// Constructors-Double
Point::Point():						// ADDED THIS DEFAULT CONSTRUCTOR
	Point(0, 0, -1, -1)
{
}

Point::Point(double x, double y):
	Point(x, y, -1, -1)
{
}

Point::Point(double x, double y, int strokeId):
	Point(x,y,strokeId, -1)
{
}

Point::Point(double x, double y, int strokeId, long timestamp):
	X(x),
	Y(y),
	_x(x),
	_y(y),	
	StrokeID(strokeId),
	Timestamp(timestamp)
{
}

//void Point::initPoints(float x, float y, int strokeId, long timestamp)
//{
//	this->_x = this->X = x;
//	this->_y = this->Y = y;
//	this->StrokeID = strokeId;
//	this->Timestamp = timestamp;
//
//}





