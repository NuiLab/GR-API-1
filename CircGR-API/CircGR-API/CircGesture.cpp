#include "CircGesture.h"


using namespace std;
using namespace MTCircGR;



//Main Construct O(n)

CircGesture::CircGesture(PointMap points, string gestureName = "", eGestureType gestureType = eGestureType::Template): Gesture(points, gestureName, gestureType)
{
	map<int, TraceData> Traces = map<int, TraceData>();
	vector<Point> anchorPoints = vector<Point>();
	vector<int> anchorIDs = vector<int>();
	vector<int> activeTraceIDs = vector<int>();

	vector<int> traceIDs = points.getTraceIDs();

	map<Directions, vector<double>> directionalEvents = map<Directions, vector<double>>();
	map<Directions, vector<double>> temporalEvents = map<Directions, vector<double>>();
	map<Directions, ResultantVector> dirResultants = map<Directions, ResultantVector>();
	map<Directions, double> dirS = map<Directions, double>();
	map<Directions, double> dirC = map<Directions, double>();
	map<Directions, int> dirCounts = map<Directions, int>();
	map<int, vector<double>> observations = map<int, vector<double>>();



	auto pathInfo = PathInfo(points);
	map<int, double> pathLengths = get<0>(pathInfo);
	map<int, Point> centroids = get<1>(pathInfo);
	map<int, Point> firstPoints = get<2>(pathInfo);
	map<int, int> idCounts = get<3>(pathInfo);
	long startTime = get<4>(pathInfo);
	long endTime = get<5>(pathInfo);

	map<int, double> intervalLengths = map<int, double>();


	for(int id : traceIDs)
	{
		double I = pathLengths[id] / (SAMPLING_RESOLUTION - 1);				

		if (I < 0.5)
		{
			anchorPoints.push_back(centroids[id]);
			anchorIDs.push_back(id);
		}
		else
		{
			intervalLengths.insert({ id, I });
			activeTraceIDs.push_back(id);
			observations.insert({ id, vector<double>() });
		}

	}

	//serves as the centroid of the entire gesture
	Point gestureCentroid;

	// Create initializer list
	vector<Point> vec = vector<Point>();
	for (auto element : firstPoints)
		vec.push_back(element.second);
		


	if (anchorPoints.size() == 0)
	{
		gestureCentroid = CalculateCentroid(vec);
	}
	else
	{
		gestureCentroid = CalculateCentroid(anchorPoints);
	}


	for(int id : activeTraceIDs)
	{
		vector<Point> pts = points.getPoints(id);				// VERIFY THIS PERFORMS WHAT IS SUPPOSED TO

		Traces.insert({ id, ProcessTrace(pts, intervalLengths[id], gestureCentroid, directionalEvents, temporalEvents, dirCounts, dirC, dirS, startTime, endTime) });
	}



	for (int iter = CircGesture::Directions::U; iter != CircGesture::Directions::EndOfDirectionsEnum; iter++)			// MIGHT NEED TO REPLACE ENUM IMPLEMENTATION FOR SOMETHING ELSE
	{
		auto dir = static_cast<CircGesture::Directions>(iter);

		if (!(dirC.find(dir) != dirC.end()))
			continue;
		double C = dirC[dir];
		double S = dirS[dir];

		double angle = atan2(S, C);

		if (angle < 0)
			angle += 2 * M_PI;

		double magnitude = sqrt(pow(C, 2) + pow(S, 2));
		ResultantVector r = ResultantVector(angle, magnitude, dirCounts[dir] - magnitude);
		dirResultants.insert({ dir, r });
	}

	//calculate gesture resultant
	double gC = 0;
	double gS = 0;

	for(int id : activeTraceIDs)
	{
		double angle = Traces[id].resultant.getAngle();
		gC += cos(angle);
		gS += sin(angle);
	}


	double gAngle = atan2(gS, gC);
	if (gAngle < 0)
		gAngle += 2 * M_PI;

	double gMagnitude = sqrt(pow(gC, 2) + pow(gS, 2));

	GestureResultant = ResultantVector(gAngle, gMagnitude, traceIDs.size() - gMagnitude);

	this->temporalEvents = TemporalEvents(startTime, endTime, temporalEvents);
	this->directionalEvents = DirectionalEvents(directionalEvents, dirResultants);

}



vector<double> CircGesture::getOBSERVATIONS()
{

	//modified [FRO]
	vector<double> obs = vector<double>();

	for (auto t : Traces)
	{
		auto listTraces = t.second.getOBSERVATIONS();

		obs.insert(obs.end(), listTraces.begin(), listTraces.end());
	}

	return obs;

}



CircGesture::TraceData CircGesture::ProcessTrace(vector<Point> points, double I, Point gestureCentroid, map<CircGesture::Directions, vector<double>> directionalEvents, map<Directions, vector<double>> temporalEvents, map<Directions, int> dirCounts, map<Directions, double> dirC, map<Directions, double> dirS, long startTime, long endTime)
{

	double D = 0, C = 0, S = 0;

	double prevObservation = -1;
	Directions prevDir = Directions::CCW; //for Complement Direction
	vector<double> observations = vector<double>();

	for (int i = 1; i < points.size(); i++)
	{
		//Console.WriteLine("Processing Point {0} ID: {1} X:{2} Y:{3} T:{4}", i, points[i].StrokeID, points[i].X, points[i].Y, points[i].timestamp);

		double d = Geometry::EuclideanDistance(points[i - 1], points[i]);
		//Console.WriteLine("\t d = {0}", d);
		if (D + d >= I)
		{
			Point prevPt = points[i - 1];
			while (D + d >= I)
			{
				// add interpolated point
				double t = min(max((I - D) / d, 0.0), 1.0);				// REMOVED FLOAT CAST HERE
				Point nPt = Point(
					(1.0 - t) * prevPt.X + t * points[i].X,
					(1.0 - t) * prevPt.Y + t * points[i].Y,
					points[i].StrokeID,
					points[i].Timestamp
				);

				double observation = CalculateObservation(prevPt, nPt);
				observations.push_back(observation);
				double c = cos(observation);
				double s = sin(observation);
				C += c;
				S += s;
				CircGesture::Directions direction = GetDirection(observation);
				CircGesture::Directions centripetalDirection = GetCentripetalDirection(prevPt, nPt, gestureCentroid);
				UpdateCounts(dirCounts, direction);
				UpdateCounts(dirCounts, centripetalDirection);
				UpdateResultants(dirC, direction, c);
				UpdateResultants(dirS, direction, s);
				UpdateResultants(dirC, centripetalDirection, c);
				UpdateResultants(dirS, centripetalDirection, s);

				UpdateEventsMaps(directionalEvents, direction, observation);
				UpdateEventsMaps(directionalEvents, centripetalDirection, observation);

				double temporalObservation = CalculateTemporalObservation(nPt.Timestamp, startTime, endTime);

				UpdateEventsMaps(temporalEvents, direction, temporalObservation);
				UpdateEventsMaps(temporalEvents, centripetalDirection, temporalObservation);



				if (!(prevObservation < 0))
				{
					Directions clockDirection = GetRotationalDirection(prevObservation, observation);
					UpdateResultants(dirC, clockDirection, c);
					UpdateResultants(dirS, clockDirection, s);

					UpdateEventsMaps(directionalEvents, clockDirection, observation);
					UpdateEventsMaps(temporalEvents, clockDirection, temporalObservation);
					UpdateCounts(dirCounts, clockDirection);

					//Complement direction is disabled for now, was concived after experiment
					//if (IsComplement(direction, prevDir))
					//{
					//    UpdateResultants(dirC, Directions.COMPLEMENT, c);
					//    UpdateResultants(dirS, Directions.COMPLEMENT, s);
					//    UpdateEventsMaps(directionalEvents, Directions.COMPLEMENT, observation);
					//    UpdateEventsMaps(temporalEvents, Directions.COMPLEMENT, temporalObservation);
					//    UpdateCounts(dirCounts, Directions.COMPLEMENT);
					//}

				}

				prevObservation = observation;
				prevDir = direction;

				// update partial length
				d = D + d - I;
				//Console.WriteLine("\t\t Updated d={0} stepDistance(D)={1} ", d, D);
				D = 0;
				prevPt = nPt;
			}
			D = d;
			//Console.WriteLine("\t\tD going out = {0}", D);
		}
		else
		{
			D += d;
			//Console.WriteLine("\tWhile-Loop skipped D={0}", D);
		}
	}

	double angle = atan2(S, C);

	if (angle < 0)
		angle += 2 * M_PI;

	double magnitude = sqrt(pow(C, 2) + pow(S, 2));
	return TraceData(observations, ResultantVector(angle, magnitude, observations.size() - magnitude));
}



tuple<map<int, double>, map<int, Point>, map<int, Point>, map<int, int>, long, long> CircGesture::PathInfo(PointMap points)
{
	map<int, double> pathLengths = map<int, double>();
	map<int, Point> centroids = map<int, Point>();
	map<int, Point> firstPoints = map<int, Point>();
	map<int, int> idCounts = map<int, int>();

	long startTime = LONG_MAX;
	long endTime = LONG_MAX;

	for (int traceId : points.getTraceIDs())
	{
		vector<Point> pts = vector<Point>(points.getPoints(traceId));
		Point prev = pts[0];
		firstPoints.insert({ traceId, prev });
		centroids.insert({ traceId, Point(prev.X, prev.Y, traceId, -1) });
		idCounts.insert({ traceId, 1 });
		pathLengths.insert({ traceId, 0 });

		for (int i = 1; i < pts.size(); i++)
		{
			Point p = pts[i];

			//update the amount of points with this id
			idCounts[traceId] += 1;

			//update the centroid for a particular trace id
			centroids[traceId].X += p.X;
			centroids[traceId].Y += p.Y;

			//update start and end time
			if (p.Timestamp > endTime)
				endTime = p.Timestamp;
			if (p.Timestamp < startTime)
				startTime = p.Timestamp;


			//update pathLength
			pathLengths[traceId] += Geometry::EuclideanDistance(prev, p);

			prev = p;

		}
		Point c = centroids[traceId];
		centroids[traceId] = Point(c.X / idCounts[traceId], c.Y / idCounts[traceId], c.StrokeID, c.Timestamp);

	}

	auto t = make_tuple(pathLengths, centroids, firstPoints, idCounts, startTime, endTime);

	return t;
}



void CircGesture::UpdateCounts(map<CircGesture::Directions, int> dirCounts, CircGesture::Directions direction)
{
	if (dirCounts.find(direction) != dirCounts.end())
		dirCounts[direction] += 1;
	else
		dirCounts.insert({ direction, 1 });
}



// Small utility function to update the various maps / dictionary
void CircGesture::UpdateEventsMaps(map<CircGesture::Directions, vector<double>> eventMap, CircGesture::Directions direction, double observation)
{
	if (eventMap.find(direction) != eventMap.end())
	{
		eventMap.at(direction).push_back(observation);		// VERIFY THIS IS AN ACCURATE APPROACH
	}
	else
	{
		eventMap.insert({ direction, vector<double>() });
		eventMap.at(direction).push_back(observation);
	}
}



// NOT REFERENCED
//void CircGesture::UpdateResultants(map<CircGesture::Directions, double> dirResultant, CircGesture::Directions direction, double observation, UpdateFunction function)
//{
//	if (dirResultant.find(direction) != dirResultant.end())
//	{
//		dirResultant[direction] += function(observation);       // VERIFY THIS IS CORRECT
//	}
//	else
//	{
//		dirResultant.insert({ direction, function(observation) });
//
//	}
//}


void CircGesture::UpdateResultants(map<CircGesture::Directions, double> dirResultant, CircGesture::Directions direction, double value)
{
	if (dirResultant.find(direction) != dirResultant.end())
	{
		dirResultant[direction] += value;		// VERIFY THIS IS CORRECT
	}
	else
	{
		dirResultant.insert({ direction, value });
	}
}



// Calculate an observation based on a time interval
double CircGesture::CalculateTemporalObservation(long time, long startTime, long endtime)
{
	if (time > endtime || time < startTime)
		throw exception("Time not in range");

	return ((time - startTime) / (endtime - startTime)) * 2 * M_PI;
}



CircGesture::Directions CircGesture::GetRotationalDirection(double prev, double curr)
{
	//work in degress for the mean while
	double p_angle = prev * (180 / M_PI);
	double c_angle = curr * (180 / M_PI);

	double max_ccw = p_angle + 180;

	//test if P_angle + 180 crossed 0
	if (max_ccw <= 359)
	{
		if (p_angle <= c_angle && c_angle <= max_ccw)
			return Directions::CCW;
		else
			return Directions::CW;
	}
	else
	{
		//angle between p_angle and 0
		double split = 360 - p_angle;
		//angle between 0 and max_ccw
		double adjusted = max_ccw - 360;

		if ((p_angle <= c_angle && c_angle <= 360) || (0 <= c_angle && c_angle <= adjusted))
			return Directions::CCW;
		else
			return Directions::CW;


	}
}



CircGesture::ResultantVector CircGesture::CalculateGestureResultant(map<int, CircGesture::TraceData> traces)
{
	if (traces.size() < 2)
		return traces.begin()->second.resultant;			// VERIFY THIS IS ACCURATE TRANSLATION

	double maxMagnitude = DBL_MIN;

	//find max Magnitude for normalization
	for (auto element : traces)
	{
		if (element.second.resultant.getMagnitude() > maxMagnitude)
			maxMagnitude = element.second.resultant.getMagnitude();
	}

	double C = 0;
	double S = 0;


	for (auto element : traces)
	{
		//contributions to the gesture resultant trace are weighted by the magnitude, the higher the dispersion, the less it contributes
		//magnitude is a measure of dispersion in this case because all traces have the same N (number of observations) in Dispersion = N - Magnitude
		C += (element.second.resultant.getMagnitude() / maxMagnitude) * cos(element.second.resultant.getAngle());
		S += (element.second.resultant.getMagnitude() / maxMagnitude) * sin(element.second.resultant.getAngle());
	}

	double angle = atan2(S, C);

	if (angle < 0)
		angle += 2 * M_PI;

	double magnitude = sqrt(pow(C, 2) + pow(S, 2));

	return CircGesture::ResultantVector(angle, magnitude, traces.size() - magnitude);

}



CircGesture::Directions CircGesture::GetCentripetalDirection(Point prev, Point cur, Point center)
{
	if (Geometry::EuclideanDistance(prev, center) > Geometry::EuclideanDistance(cur, center))
		return Directions::I;
	return Directions::O;
}


CircGesture::Directions CircGesture::GetDirection(double observation)
{
	//This method deals with degrees because its easier to deal with
	double o = observation * (180 / M_PI);

	if (o > 180) //UP region
	{

		if (o >= 360 - DIRECTIONAL_MARGIN)
			return Directions::R;

		if (o >= 270 + DIRECTIONAL_MARGIN)
			return Directions::UR;

		if (o >= 270 - DIRECTIONAL_MARGIN)
			return Directions::U;

		if (o >= 180 + DIRECTIONAL_MARGIN)
			return Directions::UL;

		return Directions::L;


	}
	else //DOWN Region
	{
		if (o >= 180 - DIRECTIONAL_MARGIN)
			return Directions::L;

		if (o >= 90 + DIRECTIONAL_MARGIN)
			return Directions::DL;

		if (o >= 90 - DIRECTIONAL_MARGIN)
			return Directions::D;

		if (o >= 0 + DIRECTIONAL_MARGIN)
			return Directions::DR;

		return Directions::R;
	}


}



/*
Calculates the angle of an observation generated from a point and its subsequent point.

@param prev = the reference, or "before" point
@param cur = the point on which the observation will be made
@return = observation in radians

*/
double CircGesture::CalculateObservation(Point prev, Point cur)
{
	Point o = Point(cur.X - prev.X, cur.Y - prev.Y);

	double angle = atan2(o.Y, o.X);

	if (angle < 0)
		angle += (2 * M_PI);

	return angle;
}



CircGesture::ResultantVector CircGesture::CalculateResultant(vector<double> observations)
{
	double C = 0;
	double S = 0;

	for (double o : observations)
	{
		C += cos(o);
		S += sin(o);
	}

	double angle = atan2(S, C);

	if (angle < 0)
		angle += 2 * M_PI;

	double magnitude = sqrt(pow(C, 2) + pow(S, 2));

	return ResultantVector(angle, magnitude, observations.size() - magnitude);
}



Point CircGesture::CalculateCentroid(vector<Point> anchorPoints)
{
	double cx = 0, cy = 0;

	for (Point p : anchorPoints)
	{
		cx += p.X;
		cy += p.Y;
	}

	return Point(cx / anchorPoints.size(), cy / anchorPoints.size());
}



Point CircGesture::CalculateCentroid(map<int, vector<Point>> resampledPoints)
{
	double cx = 0, cy = 0;

	int numTraces = resampledPoints.size();

	for (auto element : resampledPoints)
	{
		cx += resampledPoints[element.first][0].X;
		cy += resampledPoints[element.first][0].Y;
	}

	return Point(cx / numTraces, cy / numTraces);
}



bool CircGesture::IsComplement(CircGesture::Directions A, CircGesture::Directions B)
{
	if (A == B)
		return false;


	if (A == Directions::U)
		return B == Directions::D;
	if (A == Directions::D)
		return B == Directions::U;
	if (A == Directions::UL)
		return B == Directions::DR;
	if (A == Directions::UR)
		return B == Directions::DL;

	if (A == Directions::DL)
		return B == Directions::UR;
	if (A == Directions::DR)
		return B == Directions::UL;
	if (A == Directions::L)
		return B == Directions::R;
	if (A == Directions::R)
		return B == Directions::L;

	return false;
}



string CircGesture::ToString()
{
	string s = "";

	s += "# Anchor Points: " + to_string(anchorPoints.size()) + "\n";

	for (Point pt : anchorPoints)
	{
		s += " X:" + to_string(pt.X) + " Y:" + to_string(pt.Y) + "\n";
	}


	s += "# of Traces: " + to_string(Traces.size()) + "\n";

	for (auto element : Traces)
	{
		s += "T.ID: " + to_string(element.first) + "\n";

		TraceData t = Traces[element.first];

		s = s + "Observations:" + "\n";

		for (double o : t.getOBSERVATIONS())
		{
			s = s + to_string(o) + "\n";
		}


		s = s + t.resultant.ToString();

	}

	//s += "Gesture Resultant\n";
	s += GestureResultant.ToString();

	s += directionalEvents.percentToString();
	s += temporalEvents.resultantsToString();
	s += directionalEvents.ToString();



	return s;
}


double CircGesture::getPercentage(Directions direction)
{
	return this->directionalEvents.getPercentage(direction);
}



#pragma region DirectionalEvents Nested Class


vector<double> CircGesture::DirectionalEvents::getObservations(CircGesture::Directions dir)
{
	if (observations.find(dir) != observations.end())		// CHECK WHETHER THIS DOES INDEED DETERMINE IF KEY EXISTS
		return observations[dir];

	return vector<double>();
}


CircGesture::ResultantVector CircGesture::DirectionalEvents::getResultant(CircGesture::Directions dir)
{
	if (resultants.find(dir) != resultants.end())			// CHECK WHETHER THIS DOES INDEED DETERMINE IF KEY EXISTS
		return resultants[dir];

	return CircGesture::ResultantVector(0, 1, 0);
}


CircGesture::ResultantVector CircGesture::DirectionalEvents::getIN_RESULTANT()
{
	if (resultants.find(CircGesture::Directions::I) != resultants.end())		// CHECK WHETHER THIS DOES INDEED DETERMINE IF KEY EXISTS
		return resultants[CircGesture::Directions::I];
	else
		return CircGesture::ResultantVector(-1, -1, -1);
}


// Not referenced
//CircGesture::ResultantVector getC_RESULTANT()
//{
//        if (resultants.find(CircGesture::Directions::C) != resultants.end())
//            return resultants[CircGesture::Directions::C];
//        else
//            return new CircGesture::ResultantVector(-1, -1, -1);
//}


double CircGesture::DirectionalEvents::getIN_PERCENT()
{

	if (percentages.find(CircGesture::Directions::I) != percentages.end())
		return percentages[CircGesture::Directions::I];
	else
		return 0;

}

double CircGesture::DirectionalEvents::getOUT_PERCENT()
{

	if (percentages.find(CircGesture::Directions::O) != percentages.end())
		return percentages[CircGesture::Directions::O];
	else
		return 0;

}

double CircGesture::DirectionalEvents::getR_PERCENT()
{

	if (percentages.find(CircGesture::Directions::R) != percentages.end())
		return percentages[CircGesture::Directions::R];
	else
		return 0;

}

double CircGesture::DirectionalEvents::getL_PERCENT()
{

	if (percentages.find(CircGesture::Directions::L) != percentages.end())
		return percentages[CircGesture::Directions::L];
	else
		return 0;

}

double CircGesture::DirectionalEvents::getU_PERCENT()
{

	if (percentages.find(CircGesture::Directions::U) != percentages.end())
		return percentages[CircGesture::Directions::U];
	else
		return 0;
}

double CircGesture::DirectionalEvents::getD_PERCENT()
{

	if (percentages.find(CircGesture::Directions::D) != percentages.end())
		return percentages[CircGesture::Directions::D];
	else
		return 0;

}

double CircGesture::DirectionalEvents::getUR_PERCENT()
{

	if (percentages.find(CircGesture::Directions::UR) != percentages.end())
		return percentages[CircGesture::Directions::UR];
	else
		return 0;

}

double CircGesture::DirectionalEvents::getUL_PERCENT()
{

	if (percentages.find(CircGesture::Directions::UL) != percentages.end())
		return percentages[CircGesture::Directions::UL];
	else
		return 0;

}

double CircGesture::DirectionalEvents::getDR_PERCENT()
{

	if (percentages.find(CircGesture::Directions::DR) != percentages.end())
		return percentages[CircGesture::Directions::DR];
	else
		return 0;

}

double CircGesture::DirectionalEvents::getDL_PERCENT()
{

	if (percentages.find(CircGesture::Directions::DL) != percentages.end())
		return percentages[CircGesture::Directions::DL];
	else
		return 0;

}

double CircGesture::DirectionalEvents::getIN_ANGLE()
{

	if (resultants.find(CircGesture::Directions::I) != resultants.end())
	{
		return resultants[CircGesture::Directions::I].getAngle();
	}
	else
	{
		return -1;
	}

}


CircGesture::DirectionalEvents  CircGesture::DirectionalEvents::CreateDirectionalEvents(vector<double> traceObservations)
{
	map<CircGesture::Directions, vector<double>> observations = map<CircGesture::Directions, vector<double>>();

	for (double o : traceObservations) {
		CircGesture::Directions d = CircGesture::GetDirection(o);

		if (observations.find(d) != observations.end())
		{
			observations[d].push_back(o);
		}
		else
		{
			observations.emplace(d, vector<double>());
			observations[d].push_back(o);
		}

	}


	map<CircGesture::Directions, CircGesture::ResultantVector> resultants = map<CircGesture::Directions, CircGesture::ResultantVector>();
	map<CircGesture::Directions, double> percentages = map<CircGesture::Directions, double>();


	for (auto element : observations)				// VERIFY THIS IS A CORRECT TRANSLATION
	{
		resultants.emplace(element.first, CircGesture::CalculateResultant(observations[element.first]));
		percentages.emplace(element.first, observations[element.first].size() / traceObservations.size());
	}

	return DirectionalEvents(observations, resultants, percentages);
}


void CircGesture::DirectionalEvents::LogDirection(CircGesture::Directions direction, double observation)
{
	if (observations.find(direction) != observations.end())				// FIX ALL MAP INSERTIONS!!!!
		observations.at(direction).push_back(observation);
	else
	{
		observations.emplace(direction, vector<double>());
		observations.at(direction).push_back(observation);
	}
}


void CircGesture::DirectionalEvents::CalculateResultants()
{
	resultants = map<CircGesture::Directions, CircGesture::ResultantVector>();
	for (auto element : observations)
	{
		resultants.emplace(element.first, CircGesture::CalculateResultant(observations[element.first]));			// FIX ALL MAP INSERTIONS

	}
}


void CircGesture::DirectionalEvents::CalculatePercentages(int numTraces)       // MAY NEED TO HAVE EQUIVALENT INTERNAL SPECIFIER
{
	percentages = map<CircGesture::Directions, double>();
	for (auto element : observations)
	{
		percentages.emplace(element.first, static_cast<double>(observations[element.first].size()) / (numTraces * (Gesture::SAMPLING_RESOLUTION - 1)));
	}
}


string CircGesture::DirectionalEvents::ToString()				// CHECK WHETHER THIS NEEDS TO BE OVERRIDEN
{
	string s = "";

	s += "Directional Observations:\n";

	for (auto element : observations)
	{
		s += "Direction: " + to_string(element.first) + "\n";

		vector<double> obs = observations[element.first];

		for (double o : obs)
			s += to_string(o) + "\n";
	}

	s += "Directional Resultants:\n";
	for (auto element : resultants)
	{
		s += "Direction: " + to_string(element.first) + "\n";
		CircGesture::ResultantVector resultant = resultants[element.first];

		s = s + "Resultant Angle:" + "\n";
		s += to_string(resultant.getAngle()) + "\n";

		s = s + "Resultant Magnitude:" + "\n";
		s += to_string(resultant.getMagnitude()) + "\n";

		s = s + "Resultant Dispersion:" + "\n";
		s += to_string(resultant.getDispersion()) + "\n";

	}

	s += "Directional Percentages:\n";
	for (auto element : percentages)
	{
		s += "Direction: " + to_string(element.first) + " Percentage: " + to_string(percentages[element.first]) + "\n";


	}
	return s;
}


double CircGesture::DirectionalEvents::getPercentage(CircGesture::Directions direction)
{
	if (percentages.find(direction) != percentages.end())
		return percentages[direction];
	else
		return 0;
}


string CircGesture::DirectionalEvents::percentToString()
{
	string s = "Directional Percentages:\n";
	for (auto element : percentages)
	{
		s += "Direction: " + to_string(element.first) + " Percentage: " + to_string(element.second) + "\n";

	}
	return s;
}

#pragma endregion