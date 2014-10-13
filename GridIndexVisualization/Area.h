#pragma once
#include <iostream>
using namespace std;

//��ʾ������࣬MapDrawer��Map����ͬһ��Area�����Ա��������һ����
class Area
{
public:
	double minLat;
	double maxLat;
	double minLon;
	double maxLon;

	void setArea(double _minLat, double _maxLat, double _minLon, double _maxLon)
	{
		this->minLat = _minLat;
		this->maxLat = _maxLat;
		this->minLon = _minLon;
		this->maxLon = _maxLon;
	}

	Area()
	{
		minLat = 0;
		maxLat = 0;
		minLon = 0;
		maxLon = 0;
	}

	Area(double _minLat, double _maxLat, double _minLon, double _maxLon)
	{
		setArea(_minLat, _maxLat, _minLon, _maxLon);
	}

	bool inArea(double lat, double lon)
	{
		//����(lat,lon)�Ƿ��ڸ�������
		return (lat > minLat && lat < maxLat && lon > minLon && lon < maxLon);
	}

	void print()
	{
		printf("area: minLat = %lf, maxLat = %lf, minLon = %lf, maxLat = %lf\n", minLat, maxLat, minLon, maxLon);
	}
};