#pragma once
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>
#include "GeoPoint.h"
#include "Area.h"
using namespace std;

class PointGridIndex
{
public:
	void createIndex(list<GeoPoint*>& pts, Area* area, int gridWidth);
	void createIndex(Area* area, int gridWidth);
	pair<int, int> getRowCol(GeoPoint* pt);
	//void drawGridLine(Gdiplus::Color color, MapDrawer& md);
	void getNearPts(GeoPoint* pt, double thresholdM, vector<GeoPoint*>& dest); //将距离pt不长于thresholdM米的所有点存入dest
	void getNearPts(GeoPoint* pt, int gridRange, vector<GeoPoint*>& dest); //将以pt所在网格为中心，边长为2*gridRange+1格中的所有点存入dest
	void kNN(GeoPoint* pt, int k, double thresholdM, vector<GeoPoint*>& dest);

//private:
	list<GeoPoint*>* **grid;
	int gridWidth;
	int gridHeight;
	double gridSizeDeg; //记录grid边长代表多少实际距离（单位为度）
	Area* area;

	void initialization();
	void insertOnePt(GeoPoint* pt);

};
