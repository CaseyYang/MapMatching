#include "GridCenter.h"

double GridCenter::maxMatchingCount = 0;

GridCenter::GridCenter()
{
}

GridCenter::GridCenter(double lat, double lon) :lat(lat), lon(lon), matchedCount(0), matchingCount(0), correctRate(0.0), matchingRate(0.0){}

string GridCenter::toJsonStr()
{
	string lonStr, latStr, correctRateStr, matchingStr;
	stringstream ss;
	ss.precision(13);
	ss << lon;
	ss >> lonStr;
	ss.clear();
	ss << lat;
	ss >> latStr;
	ss.clear();
	ss << correctRate;
	ss >> correctRateStr;
	ss.clear();
	ss << matchingRate;
	ss >> matchingStr;
	return "lon:" + lonStr + ",lat:" + latStr + ",matchingRate:" + matchingStr + ",correctRate:" + correctRateStr;
}

void GridCenter::calculateCorrectRateAndMatchingRate()
{
	this->correctRate = static_cast<double>(this->matchedCount) / this->matchingCount;
	this->matchingRate = static_cast<double>(this->matchingCount) / GridCenter::maxMatchingCount;
}
