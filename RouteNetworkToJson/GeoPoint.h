/* 
 * Last Updated at [2014/1/24 18:15] by wuhao
 */
#pragma once
#define INVALID_TIME -1

#define PI180 0.0174532925199432957694

class GeoPoint
{
public:
	double lat;
	double lon;
	int time;

	GeoPoint(double lat, double lon, int time);
	GeoPoint(double lat, double lon);

	static double geoScale;
	static double distM(double lat1, double lon1, double lat2, double lon2);
	static double distM(GeoPoint pt1, GeoPoint pt2);
	static double distM(GeoPoint* pt1, GeoPoint* pt2);
	double distM(double lat1, double lat2);
	double distM(GeoPoint pt);
	static double distDeg(double lat1, double lon1, double lat2, double lon2);
	static double distDeg(GeoPoint pt1, GeoPoint pt2);
	static double distDeg(GeoPoint* pt1, GeoPoint* pt2);
	double distDeg(double lat1, double lat2);
	double distDeg(GeoPoint pt);
};