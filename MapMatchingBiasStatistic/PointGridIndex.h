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
	//ʹ��һ��Area�����һ�������������ϵĵ�Ԫ��������һ����������
	//��Ϊֻȷ�����������ı�Ҫ������������������洢�ռ䣬��˽�ʡ�ڴ�
	//area�����������ǵ�����
	//gridWidth�����������ھ������ϵĵ�Ԫ����
	PointGridIndex(Area* area, int gridWidth);
	//ʹ��һ��Area�����һ�������������ϵĵ�Ԫ��������һ����������������pts�����е������������䵽����������
	//ȷ�����������ı�Ҫ����������洢�ռ䣬�Ϸ��ڴ�
	//area�����������ǵ�����
	//gridWidth�����������ھ������ϵĵ�Ԫ����
	//pts��Ҫ�������������е�����㼯��
	PointGridIndex(Area* area, int gridWidth, list<GeoPoint*>& pts);
	//��������
	~PointGridIndex();
	//��ȡ������������ڵĵ�Ԫ������
	//pt��Ҫ��ȡ���ڵ�Ԫ��������
	pair<int, int> getRowCol(GeoPoint* pt);
	//������pt������thresholdM�׵����е����dest
	//pt�����������
	//thresholdM�������ĵ�뾶
	//dest������㼯��
	void getNearPts(GeoPoint* pt, double thresholdM, vector<GeoPoint*>& dest);
	//����pt��������Ϊ���ģ��߳�Ϊ2*gridRange+1���е����е����dest
	//pt�����������
	//gridRange����Ԫ��Χ
	//dest������㼯��
	void getNearPts(GeoPoint* pt, int gridRange, vector<GeoPoint*>& dest);
	//KNN���ҳ��������pt�����k������㣬�������ڼ���dest��
	//pt�����������
	//k��������
	//dest������㼯��
	void kNN(GeoPoint* pt, int k, double thresholdM, vector<GeoPoint*>& dest);
	//void drawGridLine(Gdiplus::Color color, MapDrawer& md);

private:
	list<GeoPoint*>* **grid;//����������ָ���άlist<GeoPoint*> *���������ָ��
	int gridWidth;//���������������ϵĵ�Ԫ����
	int gridHeight;//��������γ�����ϵĵ�Ԫ����
	double gridSizeDeg; //��¼grid�߳��������ʵ�ʾ��루��λΪ�ȣ�
	Area* area;//�������������ǵ�����

	//����PointGridIndex����ʱ���ݸ�����Area�����gridWidth����gridHeight��gridSizeDeg
	void init();
	void insertOnePt(GeoPoint* pt);

};
