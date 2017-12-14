#ifndef __POINT__
#define __POINT__

namespace MTCircGR
{

	class Point
	{
		
	public:				// accessible to everyone
		double X, Y;
		int StrokeID;
		long Timestamp;
		Point();
		Point(double x, double y);
		Point(double x, double y, int strokeId);
		Point(double x, double y, int strokeId, long timestamp);
		static long TimeDifference(Point a, Point b) { return a.Timestamp - b.Timestamp; };
		Point operator -(const Point& b) { return Point(X - b.X, b.Y - b.Y); };			// MAY BE WRONG IN ORIGINAL 
		int getCountFromAverage() { return CountFromAverage; };
		int setCountFromAverage(int value) { CountFromAverage = value; };

		// const Point Empty = Point(0,0); // added for compatibility with $N


		
	private:			// accessbile to members of this class only
		
		//void initPoints(float x, float y, int strokeId, long timestamp);
		double _x, _y; // temp, just to check
		int CountFromAverage = 1;

	};

}


#endif 
