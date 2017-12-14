#include "GestureParser.h"

using namespace std;
using namespace MTCircGR;


// Variables to store Gesture information
int currentStrokeIndex;
string gestureName;
string expectedAs;
PointMap ptMap = PointMap();
bool verbose;			// Modify to dump parsed gesture to standard output

// Variables to store Point
double x;
double y;
int strokeID;
long timestamp;

// ----------------------------------------------------------------------
// STDOUT dump and indenting utility functions
// ----------------------------------------------------------------------


const char * GestureParser::getIndent(unsigned int numIndents)
{
	static const char * pINDENT = "                                      + ";
	static const unsigned int LENGTH = strlen(pINDENT);
	unsigned int n = numIndents*NUM_INDENTS_PER_SPACE;
	if (n > LENGTH) n = LENGTH;

	return &pINDENT[LENGTH - n];
}

// same as getIndent but no "+" at the end
const char * GestureParser::getIndentAlt(unsigned int numIndents)
{
	static const char * pINDENT = "                                        ";
	static const unsigned int LENGTH = strlen(pINDENT);
	unsigned int n = numIndents*NUM_INDENTS_PER_SPACE;
	if (n > LENGTH) n = LENGTH;

	return &pINDENT[LENGTH - n];
}

// print all attributes of pElement.
// returns the number of attributes printed
void GestureParser::dump_attribs_to_stdout(TiXmlElement* pElement, unsigned int indent, const TiXmlString elementName)
{
	if (!pElement) return;


	TiXmlAttribute* pAttrib = pElement->FirstAttribute();
	double dval;
	int ival;
	const char* pIndent = getIndent(indent);
	if(verbose) printf("\n");

	if (elementName == "Point")
	{
		// Clear Point variables
		x = 0.0;
		y = 0.0;
		strokeID = 0;
		timestamp = 0;

		// Handle Point attributes
		while (pAttrib)
		{
			
			if(verbose) printf("%s%s: value=[%s]", pIndent, pAttrib->Name(), pAttrib->Value());

			if (pAttrib->NameTStr() == "X")
			{
				if (pAttrib->QueryDoubleValue(&dval) == TIXML_SUCCESS)
				{
					if(verbose) printf(" d=%1.1f", dval);
					x = dval;
				}

			}
			else if (pAttrib->NameTStr() == "Y")
			{
				if (pAttrib->QueryDoubleValue(&dval) == TIXML_SUCCESS)
				{
					if(verbose) printf(" d=%1.1f", dval);
					y = dval;
				}

			}
			else if (pAttrib->NameTStr() == "T") // StrokeID
			{
				if (pAttrib->QueryIntValue(&ival) == TIXML_SUCCESS)
				{
					if(verbose) printf(" int=%d", ival);
					strokeID = ival;
				}

			}
			else if (pAttrib->NameTStr() == "Timestamp") // Note Timestamp is long not int
			{
				if (pAttrib->QueryIntValue(&ival) == TIXML_SUCCESS)
				{
					if(verbose) printf(" long=%d", ival);
					timestamp = static_cast<long>(ival);
				}

			}
			else if (pAttrib->NameTStr() == "Pressure")
			{
				if (pAttrib->QueryIntValue(&ival) == TIXML_SUCCESS)
				{
					if(verbose) printf(" int=%d", ival);
				}

			}
			if(verbose) printf("\n");
			pAttrib = pAttrib->Next();
		}

		ptMap.Add(Point(x, y, strokeID, timestamp));

	}
	else
	{


		while (pAttrib)
		{
			if(verbose) printf("%s%s: value=[%s]", pIndent, pAttrib->Name(), pAttrib->Value());
		

			if (pAttrib->NameTStr() == "Name")
			{
				gestureName = string(pAttrib->Value());
			}

			if (pAttrib->NameTStr() == "As")
			{
				expectedAs = string(pAttrib->Value());
			}

			if(verbose) printf("\n");
			pAttrib = pAttrib->Next();
		}

	}


}


void GestureParser::dump_to_stdout(TiXmlNode* pParent, unsigned int indent = 0)
{
	if (!pParent) return;

	TiXmlNode* pChild;
	TiXmlText* pText;
	int t = pParent->Type();
	if(verbose) printf("%s", getIndent(indent));


	switch (t)
	{
	case TiXmlNode::TINYXML_DOCUMENT:
		if(verbose) printf("Document");
		break;

	case TiXmlNode::TINYXML_ELEMENT:
	
		if(verbose) printf("Element [%s]", pParent->Value());


		if (pParent->ValueTStr() == "Gesture")
		{
			dump_attribs_to_stdout(pParent->ToElement(), indent + 1, pParent->ValueTStr());
		}
		else if (pParent->ValueTStr() == "Expected")
		{
			dump_attribs_to_stdout(pParent->ToElement(), indent + 1, pParent->ValueTStr());
		}
		else if (pParent->ValueTStr() == "Stroke")
		{
			currentStrokeIndex++;
		}
		else if (pParent->ValueTStr() == "Point")
		{
			dump_attribs_to_stdout(pParent->ToElement(), indent + 1, pParent->ValueTStr());
		}
		break;

	case TiXmlNode::TINYXML_COMMENT:
		if (verbose) printf("Comment: [%s]", pParent->Value());
		break;

	case TiXmlNode::TINYXML_UNKNOWN:
		if (verbose) printf("Unknown");
		break;

	case TiXmlNode::TINYXML_TEXT:
		pText = pParent->ToText();
		if (verbose) printf("Text: [%s]", pText->Value());
		break;

	case TiXmlNode::TINYXML_DECLARATION:
		if (verbose) printf("Declaration");
		break;
	default:
		break;
	}
	if (verbose) printf("\n");

	// Iterate over nodes
	for (pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
	{
		dump_to_stdout(pChild, indent + 1);
	}
}

// load the named file and dump its structure to STDOUT
Gesture GestureParser::dump_to_stdout(const char* pFilename, Gesture::eGestureType gestureType, bool verboseFlag = false)
{
	verbose = verboseFlag;

	// clear variables 
	ptMap.Clear();
	currentStrokeIndex = -1;

	TiXmlDocument doc(pFilename);
	bool loadOkay = doc.LoadFile();
	if (loadOkay)
	{
		if(verbose) printf("\n%s:\n", pFilename);
		dump_to_stdout(&doc); 
		return MTCircGR::Gesture::Gesture(ptMap, gestureName, gestureType, expectedAs);
	}
	else
	{
		printf("Failed to load gesture file \"%s\"\n", pFilename);
		exit(1);
	}
}







