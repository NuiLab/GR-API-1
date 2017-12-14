#include "CircGR.h"

using namespace std;
using namespace MTCircGR;



void CircGR::SetTemplates(vector<Gesture> templates)
{
	//Templates = vector<CircGesture>();
	for(Gesture t : templates)
	{
		Templates.push_back(CircGesture(t));
	}
}


string CircGR::Classify(Gesture inputGesture)
{

	if (Templates.empty())				// Check whether this is same as Templates == NULL in c#
	{
		return "Templates have not been set for " + this->Name;
	}

	CircGesture candidate = CircGesture(inputGesture);

	double minDistance = DBL_MAX;
	string c = "No Classification";

	if (Verbose)
		cout << ("========== Distances ==========") << endl;


	for(CircGesture t : Templates)
	{
		if (!InContext(t, candidate))
			continue;

		double distance = 0;

		// Iterate through enum IMPLEMENT more stable way


		//calc distance
		for (int iter = CircGesture::Directions::U; iter != CircGesture::Directions::EndOfDirectionsEnum; iter++)				// VERIFY THIS IS EQUAL TO Enum.GetValues(typeof(CircGesture::Directions))
		{
			//can be time or distance or both
			//distance += CalculateTimeDistance(candidate, t, dir);
			//distance += CalculateDistance(candidate, t, dir);
			distance += CalculateBothDistance(candidate, t, static_cast<CircGesture::Directions>(iter));
		}

		//double distance = Math.Max(CalculateTimeDistance(candidate, t), CalculateDistance(candidate, t));

		if (distance < minDistance)
		{
			c = t.Name;
			minDistance = distance;
		}

		if (Verbose)
			cout << t.Name + " "  << distance << endl;
	}

	return c;
}


string CircGR::gestureToString(Gesture inputGesture)
{
	return CircGesture(inputGesture).ToString();
}


double CircGR::CalculateBothDistance(CircGesture candidate, CircGesture _template, CircGesture::Directions direction)
{
	double distance = 0;

	vector<double> cDirections = candidate.directionalEvents.getObservations(direction);
	vector<double> tDirections = _template.directionalEvents.getObservations(direction);

	//cDirections.Sort();
	//tDirections.Sort();

	double tDistance = 0;

	vector<double> cTDirections = candidate.temporalEvents.GetEvents(direction);
	vector<double> tTDirections = _template.temporalEvents.GetEvents(direction);


	tuple<vector<double>, vector<double>> larger;
	tuple<vector<double>, vector<double>> smaller;

	bool tBig;

	if (cDirections.size() > tDirections.size())
	{
		larger = tuple<vector<double>, vector<double>>(cDirections, cTDirections);
		smaller = tuple<vector<double>, vector<double>>(tDirections, tTDirections);
		tBig = false;
	}
	else
	{
		larger = tuple<vector<double>, vector<double>>(tDirections, tTDirections);
		smaller = tuple<vector<double>, vector<double>>(cDirections, cTDirections);
		tBig = true;
	}

	for (int i = 0; i < get<0>(smaller).size(); i++)
	{

		distance += ObservationDistance(get<0>(smaller).at(i), get<0>(larger).at(i));
		tDistance += ObservationDistance(get<1>(smaller).at(i), get<1>(larger).at(i));

	}
	for (int i = get<0>(smaller).size(); i < get<0>(larger).size(); i++)
	{
		if (tBig)
			distance += ObservationDistance(get<0>(larger).at(i), candidate.directionalEvents.getResultant(direction).getAngle());
		else
			distance += ObservationDistance(get<0>(larger).at(i), _template.directionalEvents.getResultant(direction).getAngle());
		tDistance += M_PI;
	}
	return distance + tDistance;
}


