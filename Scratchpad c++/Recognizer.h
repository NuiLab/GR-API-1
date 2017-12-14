#ifndef __RECOGNIZER__
#define __RECOGNIZER__
#include <string>
#include <vector>
#include "Gesture.h"

namespace MTCircGR
{
	// Interface that represents a generic Template based Recognizer.
	class Recognizer
	{

	public:
		// Converts a generic set of template gestures into the Recognizer's specific gesture representation.
		// templates = set of generic Gesture Templates
		virtual void SetTemplates(std::vector<Gesture> templates) = 0;

		// Classify an input gesture based on a recognizer's specific classification scheme.
		// inputGesture = The raw unprocessed input gesture
		// returns classification for the input gesture
		virtual std::string Classify(Gesture inputGesture) = 0;

		// Returns a string version of the Recognizer's gesture representation (the processed input gesture as a string)
		virtual std::string gestureToString(Gesture inputGesture) = 0;

		std::string getName() { return Name; };

	protected:
		std::string Name;
		bool Verbose;
		Recognizer(std::string name, bool verbose = false) : Name(name), Verbose(verbose) {};	


	};

}

#endif