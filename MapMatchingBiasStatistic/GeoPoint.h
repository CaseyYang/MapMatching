/*
* Last Updated at [2014/9/4 20:23] by wuhao
*/

#pragma once
#include <iostream>
#define PI180 0.0174532925199432957694
#define LENGTH_PER_RAD 111226.29021121707545

#define INVALID_TIME -999

class GeoPoint
{
public:
	double lat;
	double lon;
	int time;
	int matchedEdgeId;

	GeoPoint();
	GeoPoint(double lat, double lon, int time, int mmRoadId);
	GeoPoint(double lat, double lon, int time);
	GeoPoint(double lat, double lon);
	GeoPoint(std::pair<double, double>& lat_lon_pair);

	static double geoScale;
	static double distM(double lat1, double lon1, double lat2, double lon2);
	static double distM(GeoPoint& pt1, GeoPoint& pt2);
	static double distM(GeoPoint* pt1, GeoPoint* pt2);
	double distM(double lat1, double lat2);
	double distM(GeoPoint* pt);
	double distM(GeoPoint& pt);
	static double distDeg(double lat1, double lon1, double lat2, double lon2);
	static double distDeg(GeoPoint pt1, GeoPoint pt2);
	static double distDeg(GeoPoint* pt1, GeoPoint* pt2);
	double distDeg(double lat1, double lat2);
	double distDeg(GeoPoint* pt);
	double distDeg(GeoPoint& pt);

	void print();
};