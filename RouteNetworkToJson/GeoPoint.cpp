/* 
 * Last Updated at [2014/1/24 18:16] by wuhao
 */
#include "GeoPoint.h"
#include <math.h>

double GeoPoint::geoScale = 6371004 * 3.141592965 / 180;

GeoPoint::GeoPoint(double lat, double lon, int time)
{
	this->lat = lat;
	this->lon = lon;
	this->time = time;
}

GeoPoint::GeoPoint(double lat, double lon)
{
	this->lat = lat;
	this->lon = lon;
	this->time = INVALID_TIME;
}

double GeoPoint::distM(double lat1, double lon1, double lat2, double lon2)
{
	
	return sqrt((lat1 - lat2) * (lat1 - lat2) + (lon1 - lon2) * (lon1 - lon2)) * GeoPoint::geoScale;
}

double GeoPoint::distM(GeoPoint pt1, GeoPoint pt2)
{
	return GeoPoint::distM(pt1.lat, pt1.lon, pt2.lat, pt2.lon);
}

double GeoPoint::distM(GeoPoint* pt1, GeoPoint* pt2)
{
	return GeoPoint::distM(pt1->lat, pt1->lon, pt2->lat, pt2->lon);
}

double GeoPoint::distM(double lat, double lon)
{
	double lat1 = this->lat;
	double lon1 = this->lon;
	return sqrt((lat1 - lat) * (lat1 - lat) + (lon1 - lon) * (lon1 - lon)) * GeoPoint::geoScale;
}

double GeoPoint::distM(GeoPoint pt)
{
	return GeoPoint::distM(pt.lat, pt.lon);
}

double GeoPoint::distDeg(double lat1, double lon1, double lat2, double lon2)
{
	return sqrt((lat1 - lat2) * (lat1 - lat2) + (lon1 - lon2) * (lon1 - lon2));
}

double GeoPoint::distDeg(GeoPoint pt1, GeoPoint pt2)
{
	return GeoPoint::distDeg(pt1.lat, pt1.lon, pt2.lat, pt2.lon);
}

double GeoPoint::distDeg(GeoPoint* pt1, GeoPoint* pt2)
{
	return GeoPoint::distDeg(pt1->lat, pt1->lon, pt2->lat, pt2->lon);
}

double GeoPoint::distDeg(double lat, double lon)
{
	return GeoPoint::distDeg(this->lat, this->lon, lat, lon);
}

double GeoPoint::distDeg(GeoPoint pt)
{
	return GeoPoint::distDeg(this->lat, this->lon, pt.lat, pt.lon);
}