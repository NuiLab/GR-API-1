#ifndef __GESTUREPARSER__
#define __GESTUREPARSER__
#include <string>
#include "tinystr.h"
#include "tinyxml.h"
#include "Point.h"
#include "Pointmap.h"
#include "Gesture.h"

namespace MTCircGR
{
	class GestureParser
	{

	public:

		// ----------------------------------------------------------------------
		// STDOUT dump and indenting utility functions
		// ----------------------------------------------------------------------
		static const unsigned int NUM_INDENTS_PER_SPACE = 2;
		

		static const char * getIndent(unsigned int numIndents);

		// same as getIndent but no "+" at the end
		static const char * getIndentAlt(unsigned int numIndents);

		// print all attributes of pElement.
		// returns the number of attributes printed
		static void dump_attribs_to_stdout(TiXmlElement* pElement, unsigned int indent, const TiXmlString elementName);



		static void dump_to_stdout(TiXmlNode* pParent, unsigned int indent);


		// load the named file and dump its structure to STDOUT
		static Gesture dump_to_stdout(const char* pFilename, Gesture::eGestureType gestureType, bool verboseFlag);


	};
}

#endif
