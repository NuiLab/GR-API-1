#include <experimental/filesystem>
#include <sys/stat.h>

#include <iostream>
#include <string>
#include <vector>
#include "Point.h"
#include "PointMap.h"
#include "Geometry.h"
#include "Gesture.h"
#include "Recognizer.h"
#include "CircGR.h"
#include "GestureParser.h"
#include "tinyxml.h"
#include "tinystr.h"


using namespace std;
namespace M = MTCircGR;
namespace fs = std::experimental::filesystem;


void test()
{

	cout << "************** STARTING API TEST **************\n" << endl;
	

	/*
	cout << "\n************** POINT CLASS ********************\n" << endl;
	cout << "Creating Point1" << endl;
	M::Point p1 = M::Point(1, 2, 3, 4);
	
	double x1 = p1.X;
	double y1 = p1.Y;
	double id1 = p1.StrokeID;
	double ts1 = p1.Timestamp;
	cout << "X = " + to_string(x1) << endl;
	cout << "Y = " + to_string(y1) << endl;
	cout << "Stroke ID = " + to_string(id1) << endl;
	cout << "Timestamp = " + to_string(ts1) << endl;

	cout << endl;

	cout << "Creating Point2" << endl;
	M::Point p2 = M::Point(5, 6, 7, 8);

	double x2 = p2.X;
	double y2 = p2.Y;
	double id2 = p2.StrokeID;
	double ts2 = p2.Timestamp;
	cout << "X = " + to_string(x2) << endl;
	cout << "Y = " + to_string(y2) << endl;
	cout << "Stroke ID = " + to_string(id2) << endl;
	cout << "Timestamp = " + to_string(ts2) << endl;

	cout << "Creating Point3" << endl;
	M::Point p3 = M::Point(9, 10, 3, 11);

	double x3 = p3.X;
	double y3 = p3.Y;
	double id3 = p3.StrokeID;
	double ts3 = p3.Timestamp;
	cout << "X = " + to_string(x3) << endl;
	cout << "Y = " + to_string(y3) << endl;
	cout << "Stroke ID = " + to_string(id3) << endl;
	cout << "Timestamp = " + to_string(ts3) << endl;

	cout << endl;

	cout << "\n************** POINT MAP CLASS ********************\n" << endl;
	cout << "Creating Point Map" << endl;
	M::PointMap pm = M::PointMap();

	cout << "Adding point1, point2, point3..." << endl;
	pm.Add(p1);
	pm.Add(p2);
	pm.Add(p3);
	
	cout << "Getting number of points" << endl;
	int numOfPoints = pm.getNumberOfPoints();
	cout << "# of Points inside PointMap = " + to_string(numOfPoints) << endl;

	vector<int> strokeIds = pm.getTraceIDs();
	cout << "Stroke IDs: " << endl;
	for (int id : strokeIds)
		cout << "\t" << id << endl;
	

	cout << "\n************** GEOMETRY CLASS ********************\n" << endl;
	cout << "Using geometry class to calculate different things" << endl;
	
	double anglebetween = M::Geometry::AngleBetween(p1, p2);
	double distancebetween = M::Geometry::EuclideanDistance(p1, p2);
	
	cout << "Angle between p1 and p2 = " + to_string(anglebetween) << endl;
	cout << "Distance between p1 and p2 = " + to_string(distancebetween) << endl;

	*/



	cout << "\n***************** GESTURE CLASS ******************\n" << endl;



	cout << "Setting gesture recognizer as CircGR" << endl;
	M::CircGR recognizer = M::CircGR::CircGR(false);			// MODIFY BOOL FLAG FOR VERBOSE 


	cout << "Loading Template gestures:" << endl;

	M::Gesture tg;
	vector<M::Gesture> Templates;

	string templateGestureSetPath = "GestureSet/TemplateGestures/";
	for (auto & p : fs::directory_iterator(templateGestureSetPath))
	{
		if (fs::path(p).extension() == ".xml")
		{
			tg = M::GestureParser::parseGesture(fs::path(p).string().c_str(), M::Gesture::eGestureType::Template, false);

			// Add template gesture to recognizer
			Templates.push_back(tg);


			// Output gesture info
			cout << fs::path(p).filename() << ":" << endl;
			cout << "________________" << endl;

			// Test PointMap

			cout << "Name: " << tg.Name << endl;
			cout << "Expected As: " << tg.ExpectedAs << endl;
			cout << "# of Traces: " << tg.getNumOfTraces() << endl;
			cout << "# of Points: " << tg.getNumOfPoints() << endl;
			cout << "# of Anchors: " << endl;
			// Get pointmap from gesture
			M::PointMap pm = tg.getPointMap();

			// Get stroke ids from pointmap
			vector<int> strokeIds = pm.getTraceIDs();
		

			/*
			cout << "Stroke IDs: " << endl;
			for (int id : strokeIds)
			cout << "\t" << id << endl;
			*/

			// Get points from pointmap
			//vector<M::Point> points = pm.getPoints();


			//cout << recognizer.gestureToString(tg);

			cout << endl;
		}
	}

	recognizer.SetTemplates(Templates);


	//cout << "\n******************* CIRCGESTURE CLASS & DIRECTIONAL EVENTS CLASS *****************\n" << endl;


	cout << "\n******************* CLASSIFYING CANDIDATE GESTURES ***********************\n" << endl;


	cout << "Classifying input gesture" << endl;
	
	string classification;
	string candidateGestureSetPath = "GestureSet/CandidateOutput/";
	//string candidateGestureSetPath = "GestureSet/TemplateGestures/";		// Compare templates against templates
	for (auto & p : fs::directory_iterator(candidateGestureSetPath))
	{
		if (fs::path(p).extension() == ".xml")
		{
			classification = recognizer.Classify( M::GestureParser::parseGesture(fs::path(p).string().c_str(), M::Gesture::eGestureType::Candidate, false) ) ;
			cout << fs::path(p).filename().stem().string() + " classified as: " + classification << endl;
		}
	}
	


	//cout << "Gesture B3 : " << endl;

	//cout << recognizer.gestureToString(g) << endl;


	//cout << endl;
	system("PAUSE");
}


