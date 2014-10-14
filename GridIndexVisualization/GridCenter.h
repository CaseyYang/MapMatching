#pragma once
#include <string>
#include <sstream>
using namespace std;
class GridCenter
{
public:
	static double maxMatchingCount;
	double lon;
	double lat;
	int matchedCount;
	int matchingCount;
	double correctRate;
	double matchingRate;
	GridCenter();
	GridCenter(double lat, double lon);
	string toJsonStr();
	void calculateCorrectRateAndMatchingRate();
};