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
	//��������������Ҫ�Ĳ���
	void setGridIndexParameters(Area* area, int gridWidth);
	//��������������������
	void createIndex(list<GeoPoint*>& pts, Area* area, int gridWidth);
	pair<int, int> getRowCol(GeoPoint* pt);
	//������pt������thresholdM�׵����е����dest
	void getNearPts(GeoPoint* pt, double thresholdM, vector<GeoPoint*>& dest);
	//����pt��������Ϊ���ģ��߳�Ϊ2*gridRange+1���е����е����dest
	void getNearPts(GeoPoint* pt, int gridRange, vector<GeoPoint*>& dest);
	void kNN(GeoPoint* pt, int k, double thresholdM, vector<GeoPoint*>& dest);
	//void drawGridLine(Gdiplus::Color color, MapDrawer& md);

	//private:
	list<GeoPoint*>* **grid;
	int gridWidth;
	int gridHeight;
	double gridSizeDeg; //��¼grid�߳��������ʵ�ʾ��루��λΪ�ȣ�
	Area* area;

	void initialization();
	void insertOnePt(GeoPoint* pt);

};
