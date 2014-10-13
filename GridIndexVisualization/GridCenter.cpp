#include "GridCenter.h"


GridCenter::GridCenter()
{
}

GridCenter::GridCenter(double lat, double lon) :lat(lat), lon(lon), matchedCount(0), matchingCount(0), correctRate(0.0){}

string GridCenter::toJsonStr()
{
	string lonStr;
	string latStr;
	string correctRateStr;
	stringstream ss;
	ss << lon;
	ss >> lonStr;
	ss.clear();
	ss >> lat;
	ss << latStr;
	ss.clear();
	ss >> correctRate;
	ss << correctRateStr;
	return "lon:" + lonStr + ",lat:" + latStr + ",correctRate:" + correctRateStr;
}

void GridCenter::calculateCorrectRate()
{	
	this->correctRate = static_cast<double>(this->matchedCount) / this->matchingCount;
}
