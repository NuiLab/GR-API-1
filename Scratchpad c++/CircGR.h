#ifndef __CircGR__
#define __CircGR__
#define _USE_MATH_DEFINES
#include <vector>
#include <string>						// REMOVE UNNECESSARY LIBRARIES THAT PROPAGATED FROM OTHER CLASSES
#include <algorithm>
#include "Recognizer.h"
#include "CircGesture.h"
#include "Gesture.h"
#include "Recognizer.h"


namespace MTCircGR
{
	class CircGR : Recognizer
	{
		public:
			CircGR(bool verbose = false) : Recognizer("CircGR", verbose) {};
			std::vector<CircGesture> Templates;				// Set as null?
			void SetTemplates(std::vector<Gesture> templates);
			std::string Classify(Gesture inputGesture);
			std::string gestureToString(Gesture inputGesture);
			

		private:
			static double CalculateBothDistance(CircGesture candidate, CircGesture _template, CircGesture::Directions direction);
			static double ObservationDistance(double o1, double o2) { return M_PI - abs(M_PI - abs(o1 - o2)); };
			static bool InContext(CircGesture _template, CircGesture candidate) { return !(GetGestureContext(candidate, _template) == 0 || GetApplicationContext() == 0); };
			static int GetApplicationContext() { return 1; };
			static int GetGestureContext(CircGesture candidate, CircGesture _template) { return (candidate.getNumOfFingers() != _template.getNumOfFingers() || candidate.getNumOfTraces() != _template.getNumOfTraces()) ? 0 : 1; };
	};
}


#endif