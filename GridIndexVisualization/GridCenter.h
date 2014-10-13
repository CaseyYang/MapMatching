#pragma once
#include <string>
#include <sstream>
using namespace std;
class GridCenter
{
public:
	double lon;
	double lat;
	int matchedCount;
	int matchingCount;
	double correctRate;
	GridCenter();
	GridCenter(double lat, double lon);
	string toJsonCell();
	void calculateCorrectRate();
};