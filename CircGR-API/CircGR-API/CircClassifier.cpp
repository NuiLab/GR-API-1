#include "CircClassifier.h"

using namespace std;
using namespace MTCircGR;

// This class is under review. It is the first version of CircGR before Recognizer abstraction.
// It should be removed after all usefull functionality is extracted.





// Make sure these global variables are static 
string MTCircGR::CircClassifier::LastClass = "No Classification";
bool MTCircGR::CircClassifier::useHungarian = false;
bool MTCircGR::CircClassifier::useDecay = false;
bool MTCircGR::CircClassifier::useTimeScaling = false;
bool MTCircGR::CircClassifier::usePercentAdjustment = false;
int MTCircGR::CircClassifier::TrialNum = 0;
// Q95% Table for 3 - 10 values
vector<double> MTCircGR::CircClassifier::QTable = { 0.941, 0.765, 0.642, 0.560, 0.507, 0.468, 0.437, 0.412 }; //{0.970, 0.829, 0.71, 0.625, 0.568, 0.526, 0.493,0.466};


string CircClassifier::Classify(CircGesture candidate, vector<CircGesture> templates, bool& dropWindow, int windowNum = 1, bool verbose = true, string expectedGesture = "")
{
	dropWindow = false;


	if ((windowNum < 2 || LastClass == "No Classification") || !useDecay)
		LastClass = Classify6(candidate, templates, verbose);
	else
	{						//templates.Find(t => t.Name.Equals(LastClass))   VERIFY THE CODE BELOW IS THE ACCURATE TRANSLATION

		vector<CircGesture>::iterator temp = find_if(templates.begin(), templates.end(), [](const CircGesture &t) 
														{ return t.Name.compare(LastClass) == 0; } );
		
		CircGesture tempt = *temp;



		double dist = ObservationDistance(tempt.GestureResultant.getAngle(), candidate.GestureResultant.getAngle());
		if (dist > 0.52)
		{
			LastClass = Classify4(candidate, templates, verbose);
			dropWindow = true;
		}
	}

	return LastClass;
}


// Classification based on total distances from one set of observations to the other.
string CircClassifier::Classify2(CircGesture candidate, vector<CircGesture> templates)
{
	double minDistance = DBL_MAX;
	string c = "No Classification";

	vector<double> cObs = candidate.getOBSERVATIONS();

	sort(cObs.begin(),cObs.end());


	vector<CircGesture> inContext = vector<CircGesture>();

	for(CircGesture t : templates)
	{
		if (GetGestureContext(candidate, t) == 0 || GetApplicationContext() == 0)
			continue;

		inContext.push_back(t);
	}

	cout << ("========== Distances ==========");

	for(CircGesture t : inContext)
	{

		if (t.getOBSERVATIONS().size() != cObs.size())
			continue;

		double distance = 0;

		vector<double> tObs = t.getOBSERVATIONS();
		sort(tObs.begin(), tObs.end());

		for (size_t i = 0; i < tObs.size(); i++)
		{
			distance += ObservationDistance(cObs.at(i), tObs.at(i));
		}

		if (distance < minDistance)
		{
			c = t.Name;
			minDistance = distance;
		}

		cout << (t.Name + " " + to_string(distance));
	}


	return c;
}


string CircClassifier::Classify3(CircGesture candidate, vector<CircGesture> templates)
{
	double minDistance = DBL_MAX;
	string c = "No Classification";

	vector<double> cObs = candidate.getOBSERVATIONS();


	vector<CircGesture> inContext = vector<CircGesture>();

	for(CircGesture t : templates)
	{
		if (GetGestureContext(candidate, t) == 0 || GetApplicationContext() == 0)
			continue;

		inContext.push_back(t);
	}

	cout << ("========== Distances ==========");

	for(CircGesture t : inContext)
	{

		if (t.getOBSERVATIONS().size() != cObs.size())
			continue;

		double distance = 0;

		vector<double> tObs = vector<double>(t.getOBSERVATIONS());


		for (size_t i = 0; i < tObs.size(); i++)
		{
			auto tup = HungarianMatch(cObs.at(i), tObs);

			distance += (1 / (i + 1)) * get<1>(tup);

			tObs.erase(tObs.begin() + get<0>(tup));				// VERIFY THIS IS THE SAME AS RemoveAt() in C# and not off by 1 position

		}



		if (distance < minDistance)
		{
			c = t.Name;
			minDistance = distance;
		}

		cout << (t.Name + " " + to_string(distance));
	}


	return c;
}


tuple<int, double> CircClassifier::HungarianMatch(double observation, vector<double> observations)
{
	double minDistance = DBL_MAX;
	int minIndex = -1;

	for (size_t i = 0; i < observations.size(); i++)
	{
		double distance = ObservationDistance(observation, observations.at(i));

		if (distance < minDistance)
		{
			minIndex = i;
			minDistance = distance;
		}
	}


	return tuple<int, double>(minIndex, minDistance);
}


// Measure of disimmilarity implemented from "Statistical Analysis of Circular Data"  by Fisher pg. 122 
double CircClassifier::GetCircularDisimilarity(CircGesture candidate, CircGesture _template)
{
	//Test statistic
	double W = 0;


	tuple<double, double> cRanks = CalculateCircularRanks(candidate.getOBSERVATIONS());
	tuple<double, double> tRanks = CalculateCircularRanks(_template.getOBSERVATIONS());

	W += (pow(get<0>(cRanks), 2) + pow(get<1>(cRanks), 2)) / candidate.getOBSERVATIONS().size();
	W += (pow(get<0>(tRanks), 2) + pow(get<1>(tRanks), 2)) / _template.getOBSERVATIONS().size();

	W *= 2;
	return W;
}


tuple<double, double> CircClassifier::CalculateCircularRanks(vector<double> observations)
{
	//i doubt this is necessary, the book says to sort them in the circular rank section, but this test does not actually depend on the observations
	
	sort(observations.begin(), observations.end());			// MAKE SURE THIS SORT IS ACTUALLY CORRECT

	double C = 0, S = 0;

	for (size_t i = 1; i <= observations.size(); i++)
	{
		double gamma = (2 * M_PI * i) / observations.size();
		C += cos(gamma);
		S += sin(gamma);
	}
	return tuple<double, double>(C, S);
}


// Dissimilarity over the decomposition of direction or time
// filters based on context, the calculated distances
// contains more looping that necessary as directions are iterated twice over;
// context is binary
string CircClassifier::Classify4(CircGesture candidate, vector<CircGesture> templates, bool verbose = false)
{
	double minDistance = DBL_MAX;
	string c = "No Classification";

	CircGesture gestureClass = candidate;
/*
	struct comp {
		 bool operator() ( const CircGesture &lhs,  const CircGesture &rhs) const {
			 CircGesture g1 = const_cast<CircGesture &>(lhs);
			 CircGesture g2 = const_cast<CircGesture &>(rhs); 
			 int cmp = g1.ToString().compare(g2.ToString());
			 if (cmp < 0) return true;
			 else return false; 
		}
	};
*/

	//note that we are removing CircGesture as key 

	map<std::string, double> results;// = map<CircGesture, double, >();



	if (verbose)
		cout << ("========== Distances ==========");

	for(CircGesture t : templates)
	{
		if (!InContext(t, candidate))
			continue;

		//can be time or distance or both
		double distance = CalculateTimeDistance(candidate, t);
		//distance += CalculateDistance(candidate, t);

		//double distance = Math.Max(CalculateTimeDistance(candidate, t), CalculateDistance(candidate, t));

		if (distance < minDistance)
		{
			c = t.Name;
			minDistance = distance;
			gestureClass = t;
		}
		results.insert({ t.Name, distance });					// VERIFY THIS IS CORRECTLY IMPLEMENTED AND NO ADDITIONAL VALUES ARE ADDED IF NOT FOUND

		if (verbose)
			cout << (t.Name + " " + to_string(distance));
	}

	//if (inContext.Count != 0)
	//{
	//double p_val = CalculateRubineRejection(gestureClass, results);
	//Console.WriteLine("P-Value: " + p_val);
	//if (p_val < 0.98)
	//    c = "Classification Rejected";

	//if (CalculateDixonsRejection(gestureClass, results))
	//    c = "Classification Rejected";


	//}

	return c;
}


// Dissimilarity over the decomposition of direction or time.
// Filters while looping and context can contribute to distance itself, rather than just filter.
// Context is fuzzy
string CircClassifier::Classify5(CircGesture candidate, vector<CircGesture> templates)
{
	double minDistance = DBL_MAX;
	string c = "No Classification";

	CircGesture gestureClass = candidate;

	//remember the key was circgesture
	map<string, double> results;// = map<CircGesture, double>();

	//Console.WriteLine("========== Distances ==========");
	for(CircGesture t : templates)
	{
		double context = GetFuzzyGestureContext(candidate, t);
		if (context < 0 || GetApplicationContext() < 0)
			continue;

		//can be time or distance or both
		double distance = CalculateTimeDistance(candidate, t);
		distance += context * CalculateDistance(candidate, t);

		//double distance = Math.Max(CalculateTimeDistance(candidate, t), CalculateDistance(candidate, t));

		if (distance < minDistance)
		{
			c = t.Name;
			minDistance = distance;
			gestureClass = t;
		}
		results.insert({ t.Name, distance });
	}

	//if (inContext.size() != 0)
	//{
	//double p_val = CalculateRubineRejection(gestureClass, results);
	//Console.WriteLine("P-Value: " + p_val);
	//if (p_val < 0.98)
	//    c = "Classification Rejected";

	//if (CalculateDixonsRejection(gestureClass, results))
	//    c = "Classification Rejected";


	//}

	return c;
}


// Returns context in non binary, or "fuzzy", terms. 
double CircClassifier::GetFuzzyGestureContext(CircGesture candidate, CircGesture _template)
{
	if (candidate.getNumOfFingers() != _template.getNumOfFingers() || candidate.getNumOfTraces() != _template.getNumOfTraces())
		return -1;
	double maxDisp = max(candidate.GestureResultant.getDispersion(), _template.GestureResultant.getDispersion());

	return 1.0 + abs((candidate.GestureResultant.getDispersion() - _template.GestureResultant.getDispersion()) / maxDisp);
}


// Leanest version of Classify, attempts to keep looping to a minimum
string CircClassifier::Classify6(CircGesture candidate, vector<CircGesture> templates, bool verbose = false)
{
	double minDistance = DBL_MAX;
	string c = "No Classification";


	if (verbose)
		cout << ("========== Distances ==========");


	for(CircGesture t : templates)
	{
		if (!InContext(t, candidate))
			continue;

		double distance = 0;

		//calc distance
		for (int iter = CircGesture::Directions::U; iter != CircGesture::Directions::EndOfDirectionsEnum; iter++) 
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

		if (verbose)
			cout << (t.Name + " " + to_string(distance));
	}

	return c;
}


double CircClassifier::CalculateDistance(CircGesture candidate, CircGesture _template, CircGesture::Directions direction)
{
	double distance = 0;

	vector<double> cDirections = candidate.directionalEvents.getObservations(direction);
	vector<double> tDirections = _template.directionalEvents.getObservations(direction);

	vector<double> larger;
	vector<double> smaller;

	bool tBig;

	if (cDirections.size() > tDirections.size())
	{
		larger = cDirections;
		smaller = tDirections;
		tBig = false;
	}
	else
	{
		larger = tDirections;
		smaller = cDirections;
		tBig = true;
	}


	if (!useHungarian)
	{
		sort(larger.begin(), larger.end());
		sort(smaller.begin(), smaller.end());

		for (size_t i = 0; i < smaller.size(); i++)
		{
			if (usePercentAdjustment)
				distance += (1 + abs(_template.getPercentage(direction) - candidate.getPercentage(direction))) * ObservationDistance(smaller.at(i), larger.at(i));
			else
				distance += ObservationDistance(smaller.at(i), larger.at(i));
		}


		for (size_t i = smaller.size(); i < larger.size(); i++)
		{
			if (tBig)
				distance += ObservationDistance(larger.at(i), candidate.directionalEvents.getResultant(direction).getAngle());
			else
				distance += ObservationDistance(larger.at(i), _template.directionalEvents.getResultant(direction).getAngle());
		}

	}
	else
	{
		vector<double> vObs = vector<double>(larger);



		for(double obs : smaller)
		{
			tuple<int, double> h = HungarianMatch(obs, vObs);
			if (usePercentAdjustment)
				distance += (1 + abs(_template.getPercentage(direction) - candidate.getPercentage(direction))) * get<1>(h);
			else
				distance += (vObs.size() / larger.size()) * get<1>(h);

			vObs.erase(vObs.begin() + get<0>(h));
		}

		for(double obs : vObs)
		{
			if (tBig)
				distance += ObservationDistance(obs, candidate.directionalEvents.getResultant(direction).getAngle());
			else
				distance += ObservationDistance(obs, _template.directionalEvents.getResultant(direction).getAngle());
		}
	}


	return distance;
}


double CircClassifier::CalculateTimeDistance(CircGesture candidate, CircGesture _template, CircGesture::Directions direction)
{
	double distance = 0;

	vector<double> cDirections, tDirections;


	if (useTimeScaling)
	{
		auto scaledDist = ScaleTimeDistances(candidate, _template, direction);
		cDirections = get<0>(scaledDist);
		tDirections = get<0>(scaledDist);
	}
	else
	{
		cDirections = candidate.temporalEvents.GetEvents(direction);
		tDirections = _template.temporalEvents.GetEvents(direction);
	}


	vector<double> larger;
	vector<double> smaller;

	bool tBig;

	if (cDirections.size() > tDirections.size())
	{
		larger = cDirections;
		smaller = tDirections;
		tBig = false;
	}
	else
	{
		larger = tDirections;
		smaller = cDirections;
		tBig = true;
	}

	//larger.Sort();
	//smaller.Sort();

	for (size_t i = 0; i < smaller.size(); i++)
	{
		distance += ObservationDistance(smaller.at(i), larger.at(i));
	}

	for (size_t i = smaller.size(); i < larger.size(); i++)
	{
		if (tBig)
			distance += M_PI;//ObservationDistance(larger.ElementAt(i), candidate.temporalEvents.GetEventResultant(dir).Angle);
		else
			distance += M_PI; //ObservationDistance(larger.ElementAt(i), template.temporalEvents.GetEventResultant(dir).Angle);
	}

	return distance;
}


double CircClassifier::CalculateBothDistance(CircGesture candidate, CircGesture _template, CircGesture::Directions direction)
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

	for (size_t i = 0; i < get<0>(smaller).size(); i++)
	{

		distance += ObservationDistance(get<0>(smaller).at(i), get<0>(larger).at(i));
		tDistance += ObservationDistance(get<1>(smaller).at(i), get<1>(larger).at(i));

	}
	for (size_t i = get<0>(smaller).size(); i < get<0>(larger).size(); i++)
	{
		if (tBig)
			distance += ObservationDistance(get<0>(larger).at(i), candidate.directionalEvents.getResultant(direction).getAngle());
		else
			distance += ObservationDistance(get<0>(larger).at(i), _template.directionalEvents.getResultant(direction).getAngle());
		tDistance += M_PI;
	}
	return distance + tDistance;
}


		

bool CircClassifier::CalculateDixonsRejection(CircGesture gestureClass, map<CircGesture, double> results)
{
	int keys = 0;
	vector<double> values;

	for (auto const& element : results) {
		keys++;
		values.push_back(element.second);
	}

	if (keys < 3)			
		return false;

	vector<double> r = vector<double>(values);

	sort(r.begin(), r.end());

	double Q = (r[1] - r[0]) / (r[r.size() - 1] - r[0]);


	return Q < QTable[r.size() - 1];
}


double CircClassifier::CalculateDistance(CircGesture candidate, CircGesture _template)
{
	double distance = 0;

	for (int iter = CircGesture::Directions::U; iter != CircGesture::Directions::EndOfDirectionsEnum; iter++)
	{

		auto dir = static_cast<CircGesture::Directions>(iter);
			

		vector<double> cDirections = candidate.directionalEvents.getObservations(dir);
		vector<double> tDirections = _template.directionalEvents.getObservations(dir);

		vector<double> larger;
		vector<double> smaller;

		bool tBig;

		if (cDirections.size() > tDirections.size())
		{
			larger = cDirections;
			smaller = tDirections;
			tBig = false;
		}
		else
		{
			larger = tDirections;
			smaller = cDirections;
			tBig = true;
		}


		if (!useHungarian)
		{
			sort(larger.begin(), larger.end());
			sort(smaller.begin(), smaller.end());

			for (size_t i = 0; i < smaller.size(); i++)
			{
				if (usePercentAdjustment)
					distance += (1 + abs(_template.getPercentage(dir) - candidate.getPercentage(dir)))*ObservationDistance(smaller.at(i), larger.at(i));
				else
					distance += ObservationDistance(smaller.at(i), larger.at(i));
			}


			for (size_t i = smaller.size(); i < larger.size(); i++)
			{
				if (tBig)
					distance += ObservationDistance(larger.at(i), candidate.directionalEvents.getResultant(dir).getAngle());
				else
					distance += ObservationDistance(larger.at(i), _template.directionalEvents.getResultant(dir).getAngle());
			}

		}
		else
		{
			vector<double> vObs = vector<double>(larger);

			for(double obs : smaller)
			{
				tuple<int, double> h = HungarianMatch(obs, vObs);
				if (usePercentAdjustment)
					distance += (1 + abs(_template.getPercentage(dir) - candidate.getPercentage(dir))) * get<1>(h);
				else
					distance += (vObs.size() / larger.size()) * get<1>(h);

				vObs.erase(vObs.begin() + get<0>(h));
			}

			for(double obs : vObs)
			{
				if (tBig)
					distance += ObservationDistance(obs, candidate.directionalEvents.getResultant(dir).getAngle());
				else
					distance += ObservationDistance(obs, _template.directionalEvents.getResultant(dir).getAngle());
			}
		}
	}

	return distance;
}


double CircClassifier::CalculateTimeDistance(CircGesture candidate, CircGesture _template)
{
	double distance = 0;

	for (int iter = CircGesture::Directions::U; iter != CircGesture::Directions::EndOfDirectionsEnum; iter++)
	{
		auto dir = static_cast<CircGesture::Directions>(iter);
		vector<double> cDirections, tDirections;

		if (useTimeScaling)
		{
			auto scaledDist = ScaleTimeDistances(candidate, _template, dir);
			cDirections = get<0>(scaledDist);
			tDirections = get<0>(scaledDist);
		}
		else
		{
			cDirections = candidate.temporalEvents.GetEvents(dir);
			tDirections = _template.temporalEvents.GetEvents(dir);
		}


		vector<double> larger;
		vector<double> smaller;

		bool tBig;

		if (cDirections.size() > tDirections.size())
		{
			larger = cDirections;
			smaller = tDirections;
			tBig = false;
		}
		else
		{
			larger = tDirections;
			smaller = cDirections;
			tBig = true;
		}

		//larger.Sort();
		//smaller.Sort();

		for (size_t i = 0; i < smaller.size(); i++)
		{
			distance += ObservationDistance(smaller.at(i), larger.at(i));
		}

		for (size_t i = smaller.size(); i < larger.size(); i++)
		{
			if (tBig)
				distance += M_PI;//ObservationDistance(larger.ElementAt(i), candidate.temporalEvents.GetEventResultant(dir).Angle);
			else
				distance += M_PI; //ObservationDistance(larger.ElementAt(i), template.temporalEvents.GetEventResultant(dir).Angle);
		}

	}

	return distance;
}


tuple<vector<double>, vector<double>> CircClassifier::ScaleTimeDistances(CircGesture candidate, CircGesture _template, CircGesture::Directions dir)
{
	vector<double> cDirections = candidate.temporalEvents.GetEvents(dir);
	vector<double> tDirections = _template.temporalEvents.GetEvents(dir);

	//time scaling based on just the template seems to work better in practice
	double maxDur = _template.temporalEvents.getTOTAL_DURATION(); 

	double cDir;

	for (size_t i = 0; i < cDirections.size(); i++)
	{
		cDir = cDirections[i];
		cDirections[i] = fmod( (cDir * (candidate.temporalEvents.getTOTAL_DURATION() / maxDur)) , (M_PI * 2) );  // MAKE SURE THIS IS CORRECT; MODULES WITH DOUBLES instead casting values to INT
	}

	//for (int i = 0; i < tDirections.size(); i++)
	//{
	//    tDirections[i] = (tDirections[i] * (template.temporalEvents.TOTAL_DURATION / maxDur)) % (Math.PI*2);

	//}

	return tuple<vector<double>, vector<double>>(cDirections, tDirections);

}