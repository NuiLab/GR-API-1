#ifndef __CIRCGESTURE__
#define __CIRCGESTURE__
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <tuple>
#include "PointMap.h"
#include "Gesture.h"
#include <ostream>
#include <memory>

using namespace std; 


namespace MTCircGR
{

	class CircGesture : public Gesture
	{

	public: 
		//friend bool operator< (const CircGesture& lhs, const CircGesture& rhs);
			
		
		bool operator< (const MTCircGR::CircGesture& rhs)  {
			MTCircGR::CircGesture g1 = const_cast<MTCircGR::CircGesture &>(*this);
			MTCircGR::CircGesture g2 = const_cast<MTCircGR::CircGesture &>(rhs);
			int cmp = g1.ToString().compare(g2.ToString());
			if (cmp < 0) return true;
			else return false;

		}
		CircGesture(PointMap points, string gestureName, eGestureType gestureType);
		CircGesture(Gesture gesture) : Gesture(gesture.rawTraces, gesture.Name, gesture.GestureType) { this->ExpectedAs = gesture.ExpectedAs; }; //Construct a CircGesture from gesture object
		enum Directions
		{
			U, //observation direction  U,D,L,R and its combinations    // MAINTAIN AT BEGINNING OF ENUM
			D,
			L,
			R,
			UL,
			UR,
			DL,
			DR,
			I, // Centripetal Direction: Inside- toward a centroid
			O, //outside - away from a centroid
			//C, //directions relative to the centroid
			CW,//clockwise movement
			CCW, //counter-clockwise movement
			//COMPLEMENT //observation's direction is the  exact opposite of previous one
			EndOfDirectionsEnum			// MAINTAIN AT END OF THE ENUM
		};


		int getNumOfAnchors() { return anchorPoints.size(); };
		int getNumOfFingers() { return Traces.size() + anchorPoints.size(); };
		std::vector<double> getOBSERVATIONS();
		
		
		#pragma region Accessory structs

		struct ResultantVector
		{
			public:


				ResultantVector(double Angle, double Magnitude, double Dispersion)
				{
					setAngle(Angle);
					setMagnitude(Magnitude);
					setDispersion(Dispersion);
				}
				ResultantVector()
				{
					setAngle(NULL);
					setMagnitude(NULL);
					setDispersion(NULL);
				}

				double getAngle() { return angle; }
				void setAngle(double value) { angle = value; }

				double getMagnitude() { return magnitude; }
				void setMagnitude(double value) { magnitude = value; }

				double getDispersion() { return dispersion; }
				void setDispersion(double value) { dispersion = value; }


			

				std::string ToString()   		// CHECK POSSIBLE ERROR:"you are hiding virtual method by same name".
				{
					std::string s = "";
					s = s + "Resultant Angle:" + "\n";
					s = s + std::to_string(getAngle()) + "\n";

					s = s + "Resultant Magnitude:" + "\n";
					s = s + std::to_string(getMagnitude()) + "\n";

					s = s + "Resultant Dispersion:" + "\n";
					s = s + std::to_string(getDispersion()) + "\n";

					return s;
				}

			private:

				double angle;
				double magnitude;
				double dispersion;


		};
		struct TraceData
		{

		public:
			ResultantVector resultant;
			std::vector<double> getOBSERVATIONS() { return observations; }

			TraceData(std::vector<double> Observations, ResultantVector Resultant) : observations(Observations), resultant(Resultant)
			{
			}
			//this may have implications 
			TraceData() { }

		private:
			//vector of observations
			std::vector<double> observations;

		};


#pragma region DirectionalEvents Class

		class DirectionalEvents
		{

		public:

			//struct ResultantVector;   // CHECK HOW TO FORWARD STRUCT TO CLASS													// VERIFY THIS IS THE CORRECT WAY TO FIX ISSUE
			std::map<CircGesture::Directions, std::vector<double>> observations;
			std::map<CircGesture::Directions, CircGesture::ResultantVector> resultants;
			std::map<CircGesture::Directions, double> percentages;
			CircGesture::ResultantVector getResultant(CircGesture::Directions dir);
			std::vector<double> getObservations(CircGesture::Directions dir);

			// Properties
			std::vector<double> getUP() { return observations[CircGesture::Directions::U]; };
			std::vector<double> getDOWN() { return observations[CircGesture::Directions::D]; };
			std::vector<double> getLEFT() { return observations[CircGesture::Directions::L]; };
			std::vector<double> getRIGHT() { return observations[CircGesture::Directions::R]; };
			std::vector<double> getUP_RIGHT() { return observations[CircGesture::Directions::UR]; };
			std::vector<double> getUP_LEFT() { return observations[CircGesture::Directions::UL]; };
			std::vector<double> getDOWN_RIGHT() { return observations[CircGesture::Directions::DR]; };
			std::vector<double> getDOWN_LEFT() { return observations[CircGesture::Directions::DL]; };

			CircGesture::ResultantVector getIN_RESULTANT();

			double getIN_PERCENT();
			double getOUT_PERCENT();
			double getR_PERCENT();
			double getL_PERCENT();
			double getU_PERCENT();
			double getD_PERCENT();
			double getUR_PERCENT();
			double getUL_PERCENT();
			double getDR_PERCENT();
			double getDL_PERCENT();
			double getIN_ANGLE();

			DirectionalEvents() : observations(std::map<CircGesture::Directions, std::vector<double>>())
			{
				resultants = std::map<CircGesture::Directions, CircGesture::ResultantVector>();
				percentages = std::map<CircGesture::Directions, double >();
			}

			DirectionalEvents(std::map<CircGesture::Directions, std::vector<double>> o, std::map<CircGesture::Directions, CircGesture::ResultantVector> r)
				: observations(o), resultants(r), percentages(std::map<CircGesture::Directions, double>()) {};

			DirectionalEvents(std::map<CircGesture::Directions, std::vector<double>> o,
				std::map<CircGesture::Directions, CircGesture::ResultantVector> r,
				std::map<CircGesture::Directions, double> p) : observations(o), resultants(r), percentages(p) {};

			static DirectionalEvents  CreateDirectionalEvents(std::vector<double> traceObservations);
			void LogDirection(CircGesture::Directions direction, double observation);
			void CalculateResultants();
			void CalculatePercentages(int numTraces);
			std::string ToString();
			double getPercentage(CircGesture::Directions direction);
			std::string percentToString();



		};
#pragma endregion 



		struct TemporalEvents
		{

		public:
			std::map<CircGesture::Directions, std::vector<double>> events;
			long startTime, endTime, duration;
			std::map<CircGesture::Directions, CircGesture::ResultantVector> resultants;

			long getTOTAL_DURATION() { return duration; }


			TemporalEvents(long startTime, long endTime)
			{
				events = std::map<CircGesture::Directions, std::vector<double>>();
				resultants = std::map<CircGesture::Directions, CircGesture::ResultantVector>();
				this->startTime = startTime;
				this->endTime = endTime;
				duration = endTime - startTime;

			}

			TemporalEvents(long startTime, long endTime, std::map<Directions, std::vector<double>> temporalevents)
			{
				events = temporalevents;
				this->startTime = startTime;
				this->endTime = endTime;
				resultants = std::map<CircGesture::Directions, CircGesture::ResultantVector>();
				duration = endTime - startTime;
			}

			TemporalEvents() {
				events = std::map<CircGesture::Directions, std::vector<double>>();
				startTime, endTime, duration = NULL;
				resultants = std::map<CircGesture::Directions, CircGesture::ResultantVector>();
			}

			std::vector<double> GetEvents(Directions dir)
			{
				if (events.find(dir) != events.end())
					return events[dir];

				return std::vector<double>();
			}

			ResultantVector GetEventResultant(Directions dir)
			{
				if (resultants.find(dir) != resultants.end())
					return resultants[dir];

				return ResultantVector(0, 1, 0);
			}

			void LogEvent(Directions direction, double time)
			{
				if (time > endTime || time < startTime)
					throw std::invalid_argument("Invalid Time");

				//generate a normalized 0 - 360 value for a time range, 360 = 0 on the circle but it can help separate begining observations (0) to ending (360)
				double observation = ((time - startTime) / duration) * 360;
				observation *= (M_PI / 180); 

				
				std::map<CircGesture::Directions, std::vector<double>>::iterator it = events.find(direction);

				if (it != events.end())
				{
					
					it->second.push_back(observation);
				}
				else
				{
					events.insert({ direction, std::vector<double>() });
					events[direction].push_back(observation);
				}

			}

			void CalculateResultants()
			{
				resultants = std::map<Directions, ResultantVector>();
				for (auto element : events)
				{
					resultants.insert({ element.first, CalculateResultant(events[element.first]) });

				}
			}

			std::string ToString()
			{
				std::string s = "";

				s += "Temporal Events:\n";
				for (auto element : events)
				{
					s += "Direction: " + std::to_string(element.first) + "\n";
					std::vector<double> obs = events[element.first];

					for (double o : obs)
						s += std::to_string(o) + "\n";
				}


				return s;
			}

			std::string resultantsToString()
			{
				std::string s = "Resultants\n ";

				for (auto element : resultants)
				{
					s += std::to_string(element.first) + "-\n";
					s += resultants[element.first].ToString();					// VERIFY YOU CAN SAFELY CAST RESULTANT VECTOR TO STRING
				    //s += resultants[dir].Dispersion + " ";
				}

				return s;
			}

		};

		#pragma endregion
		
		//Defines how large the margin, in degrees, around U,D,R,L should be.
		static const int DIRECTIONAL_MARGIN = 10;
		std::vector<Point> anchorPoints;
		std::map<int, TraceData> Traces;
		ResultantVector GestureResultant;
		TemporalEvents temporalEvents;
		// More CircGesture Class 
		DirectionalEvents directionalEvents;			// Check how to create an instance of a struct from Directional Events class
		bool strongResultant = false;

		std::tuple<std::map<int, double>, std::map<int, Point>, std::map<int, Point>, std::map<int, int>, long, long> PathInfo(PointMap points);
		static Directions GetCentripetalDirection(Point prev, Point cur, Point center);
		static Directions GetDirection(double observation);
		static double CalculateObservation(Point prev, Point cur);
		static ResultantVector CalculateResultant(std::vector<double> observations);
		std::string ToString();
		double getPercentage(Directions direction);


	private:
		TraceData ProcessTrace(	std::vector<Point> points, double I, Point gestureCentroid, std::map<Directions, std::vector<double>> directionalEvents, 
								std::map<Directions, std::vector<double>> temporalEvents, std::map<Directions, int> dirCounts, std::map<Directions, 
								double> dirC, std::map<Directions, double> dirS, long startTime, long endTime);
		void UpdateCounts(std::map<Directions, int > dirCounts, Directions direction);
		void UpdateEventsMaps(std::map<Directions, std::vector<double>> eventMap, Directions direction, double observation);
		
		// function pointer used to update the resultant vectors, its either Cos(x) or Sin(x)
		//double UpdateFunction(double observation);	// Check whether this is a correct translation of delegate double UpdateFunction(double observation)
		//double(*pUF) (double);
		//void UpdateResultants(std::map<Directions, double> dirResultant, Directions direction, double observation, UpdateFunction function);

		void UpdateResultants(std::map<Directions, double> dirResultant, Directions direction, double value);
		double CalculateTemporalObservation(long time, long startTime, long endtime);
		Directions GetRotationalDirection(double prev, double curr);
		ResultantVector CalculateGestureResultant(std::map<int, TraceData> traces);
		Point CalculateCentroid(std::vector<Point> anchorPoints);
		Point CalculateCentroid(std::map<int, std::vector<Point>> resampledPoints);
		bool IsComplement(Directions A, Directions B);


	};

}

//
// bool operator< (const MTCircGR::CircGesture& lhs, const MTCircGR::CircGesture& rhs) {
//	MTCircGR::CircGesture g1 = const_cast<MTCircGR::CircGesture &>(lhs);
//	MTCircGR::CircGesture g2 = const_cast<MTCircGR::CircGesture &>(rhs);
//	int cmp = g1.ToString().compare(g2.ToString());
//	if (cmp < 0) return true;
//	else return false;
//
//}


#endif


/*std::ostream& operator<<(std::ostream &strm, const  MTCircGR::CircGesture &cg) {

return strm; // << cg.ToString() << std::endl;
}*/