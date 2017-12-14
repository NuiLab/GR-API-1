#ifndef __CIRCCLASSIFIER__
#define __CIRCCLASSIFIER__
#define _USE_MATH_DEFINES
#include <string>
#include <tuple>
#include <algorithm>
#include <math.h>
#include "CircGesture.h"

namespace MTCircGR
{
	class CircClassifier
	{
		public:
		    // Add default constructor
			//CircClassifier();
			static bool useHungarian;
			static bool useDecay;
			static bool useTimeScaling;
			static bool usePercentAdjustment;
			static int TrialNum;									
			static std::string LastClass;
			static vector<double> QTable;

			// CHECK THAT PASS BY REFERENCE WAS IMPLEMENTED FOR FUNCTION BELOW
			static std::string Classify(CircGesture candidate, std::vector<CircGesture> templates, bool& dropWindow, int windowNum, bool verbose, std::string expectedGesture);
			static std::string Classify2(CircGesture candidate, std::vector<CircGesture> templates);
			static std::string Classify3(CircGesture candidate, std::vector<CircGesture> templates);
			static std::tuple<double, double> CalculateCircularRanks(std::vector<double> observations);
			static std::string Classify4(CircGesture candidate, std::vector<CircGesture> templates, bool verbose);
			static std::string Classify5(CircGesture candidate, std::vector<CircGesture> templates);
			static std::string Classify6(CircGesture candidate, std::vector<CircGesture> templates, bool verbose);
			static double CalculateDistance(CircGesture candidate, CircGesture _template);
			static double CalculateTimeDistance(CircGesture candidate, CircGesture _template);
			static std::tuple<std::vector<double>, std::vector<double>> ScaleTimeDistances(CircGesture candidate, CircGesture _template, CircGesture::Directions dir);


		private:

			static int GetApplicationContext(){ return 1; }					// REPLACE WITH A LAMBDA?
			static int GetGestureContext(CircGesture candidate, CircGesture _template) { return (candidate.getNumOfFingers() != _template.getNumOfFingers() || candidate.getNumOfTraces() != _template.getNumOfTraces()) ? 0 : 1; };
			static double ObservationDistance(double o1, double o2) { return M_PI - abs(M_PI - abs(o1 - o2)); };
			static std::tuple<int, double> HungarianMatch(double observation, std::vector<double> observations);
			static double GetCircularDisimilarity(CircGesture candidate, CircGesture _template);
			static bool InContext(CircGesture _template, CircGesture candidate){ return !(GetGestureContext(candidate, _template) == 0 || GetApplicationContext() == 0); };
			static double GetFuzzyGestureContext(CircGesture candidate, CircGesture _template);
			static double CalculateDistance(CircGesture candidate, CircGesture _template, CircGesture::Directions direction);
			static double CalculateTimeDistance(CircGesture candidate, CircGesture _template, CircGesture::Directions direction);
			static double CalculateBothDistance(CircGesture candidate, CircGesture _template, CircGesture::Directions direction);
			static bool CalculateDixonsRejection(CircGesture gestureClass, std::map<CircGesture, double> results);
	};
}


#endif