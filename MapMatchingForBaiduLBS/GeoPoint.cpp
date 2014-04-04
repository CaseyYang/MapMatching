/* 
 * Last Updated at [2014/2/10 11:20] by wuhao
 */
#include "GeoPoint.h"
#include <math.h>

double GeoPoint::geoScale = 6371004 * 3.141592965 / 180;

GeoPoint::GeoPoint()
{
	this->lat = 0;
	this->lon = 0;
	this->time = INVALID_TIME;
}

GeoPoint::GeoPoint(double lat, double lon, int time, int mmRoadId)
{
	this->lat = lat;
	this->lon = lon;
	this->time = time;
	this->mmRoadId = mmRoadId;
}

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

GeoPoint::GeoPoint(std::pair<double, double>& lat_lon_pair)
{
	this->lat = lat_lon_pair.first;
	this->lon = lat_lon_pair.second;
	this->time = INVALID_TIME;
}

double GeoPoint::distM(double lat1, double lon1, double lat2, double lon2)
{
	
	return sqrt((lat1 - lat2) * (lat1 - lat2) + (lon1 - lon2) * (lon1 - lon2)) * GeoPoint::geoScale;
}

double GeoPoint::distM(GeoPoint& pt1, GeoPoint& pt2)
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

double GeoPoint::distM(GeoPoint& pt)
{
	return distM(pt.lat, pt.lon);
}

double GeoPoint::distM(GeoPoint* pt)
{
	return distM(pt->lat, pt->lon);
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

double GeoPoint::distDeg(GeoPoint& pt)
{
	return distDeg(this->lat, this->lon, pt.lat, pt.lon);
}

double GeoPoint::distDeg(GeoPoint* pt)
{
	return distDeg(this->lat, this->lon, pt->lat, pt->lon);
}