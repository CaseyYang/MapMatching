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
	//使用一个Area对象和一个索引经度轴上的单元格数构造一个网格索引
	//因为只确定网格索引的必要参数，并不真正申请存储空间，因此较省内存
	//area：索引所覆盖的区域
	//gridWidth：网格索引在经度轴上的单元格数
	PointGridIndex(Area* area, int gridWidth);
	//使用一个Area对象和一个索引经度轴上的单元格数构造一个网格索引，并把pts集合中的所有坐标点填充到网格索引中
	//确定网格索引的必要参数并申请存储空间，较费内存
	//area：索引所覆盖的区域
	//gridWidth：网格索引在经度轴上的单元格数
	//pts：要放入网格索引中的坐标点集合
	PointGridIndex(Area* area, int gridWidth, list<GeoPoint*>& pts);
	//析构函数
	~PointGridIndex();
	//获取给定坐标点所在的单元格坐标
	//pt：要获取所在单元格的坐标点
	pair<int, int> getRowCol(GeoPoint* pt);
	//将距离pt不长于thresholdM米的所有点存入dest
	//pt：中心坐标点
	//thresholdM：距中心点半径
	//dest：坐标点集合
	void getNearPts(GeoPoint* pt, double thresholdM, vector<GeoPoint*>& dest);
	//将以pt所在网格为中心，边长为2*gridRange+1格中的所有点存入dest
	//pt：中心坐标点
	//gridRange：单元格范围
	//dest：坐标点集合
	void getNearPts(GeoPoint* pt, int gridRange, vector<GeoPoint*>& dest);
	//KNN：找出离坐标点pt最近的k个坐标点，并保存在集合dest中
	//pt：中心坐标点
	//k：近邻数
	//dest：坐标点集合
	void kNN(GeoPoint* pt, int k, double thresholdM, vector<GeoPoint*>& dest);
	//void drawGridLine(Gdiplus::Color color, MapDrawer& md);

private:
	list<GeoPoint*>* **grid;//网格索引：指向二维list<GeoPoint*> *类型数组的指针
	int gridWidth;//网格索引经度轴上的单元格数
	int gridHeight;//网格索引纬度轴上的单元格数
	double gridSizeDeg; //记录grid边长代表多少实际距离（单位为度）
	Area* area;//网格索引所覆盖的区域

	//构造PointGridIndex对象时根据给定的Area对象和gridWidth计算gridHeight和gridSizeDeg
	void init();
	void insertOnePt(GeoPoint* pt);

};
