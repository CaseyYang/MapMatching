#pragma once
#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <queue>
#include "GeoPoint.h"
using namespace std;


#define INF  1000000000 //最短路径所用
#define MAXSPEED 50 //最大速度
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#define max(a,b)	(((a) > (b)) ? (a) : (b))
typedef list<GeoPoint*> Figure; //代表一条路形，每个GeoPoint*代表路形点，首尾节点即路的两个端点
typedef std::pair<double, double> simplePoint; //内部类型，勿改动

struct  Edge
{
	Figure* figure;  //路形信息
	double lengthM;  //记录路段总长，单位为m
	int startNodeId; //路段起始点id
	int endNodeId;  //路段终止点id
	bool visited;  //辅助字段，外部调用勿改动
};

struct AdjNode //邻接表结点
{
	int endPointId;
	int edgeId;
	AdjNode* next;
};

typedef list<GeoPoint*> Traj;

class Map
{
public:
	vector<Edge*> edges; //保存所有边的集合，如果边的两个端点至少一个不在范围内则为NULL，【逐个遍历需手动判断NULL】
	vector<GeoPoint*> nodes; //保存所有点的集合,如果点不在范围内则为NULL，【逐个遍历需手动判断NULL】
	vector<AdjNode*> adjList; //邻接表

	//以gridWidth列的粒度来创建索引
	Map(string folderDir, int gridWidth);  

	//返回距离(lat, lon)点严格小于threshold米的所有Edge*
	vector<Edge*> getNearEdges(double lat, double lon, double threshold) const;
	//返回距离(lat, lon)点最近的Edge*
	Edge* getNearestEdge(double lat, double lon,double &shortestDist);
	//返回(lat,lon)点到edge的距离，单位为米
	double distM(double lat, double lon, Edge* edge) const;
	//返回(lat,lon)点到edge的距离，单位为米；同时记录投影点到edge起点的距离存入prjDist，无投影则记为0
	double distM(double lat, double lon, Edge* edge, double& prjDist) const;
	//判断startNodeId与endNodeId之间有无边,没有边返回-1，有边返回edgeId
	int hasEdge(int startNodeId, int endNodeId) const; 
	void insertNode(double lat, double lon);
	//在当前图中插入边
	void insertEdge(Edge* edge, int startNodeId, int endNodeId); 
	void delEdge(int edgeId);
	//A路段起点到B路段起点的最小路网距离
	//参数：
	//ID1：A路段起点
	//ID2：B路段起点
	//dist1：基于隐马尔科夫模型地图匹配算法中轨迹点到B路段起点的距离，默认值为0
	//dist2：基于隐马尔科夫模型地图匹配算法中轨迹点到A路段起点的距离，默认值为0
	//deltaT：基于隐马尔科夫模型地图匹配算法中两轨迹点的时间差，默认为100000000
	double shortestPathLength(int ID1, int ID2, double dist1 = 0, double dist2 = 0, double deltaT = 100000000);//A路段起点到B路段起点的最小路网距离


private:
	int gridWidth, gridHeight;
	double gridSizeDeg;
	double strictThreshold = 0.1;
	list<Edge*>* **grid;
	/*double minLat = 1.22;
	double maxLat = 1.5;
	double minLon = 103.620;
	double maxLon = 104.0;*/
	double minLat = 0.99999;
	double maxLat = 1.6265;
	double minLon = 103.548;
	double maxLon = 104.1155;


	int getRowId(double lat) const;
	int getColId(double lon) const;
	//给定行号row和列号row，找出相应单元格中离(lat,lon)点最近的路段，保存在currentResultEdge中，shortestDist保存相应的最短距离
	void getNearestEdgeInAGridCell(double lat, double lon, int row, int col, Edge*& currentResultEdge, double &shortestDist);
	//返回(lat,lon)点到edge的距离上界,提前预判优化版本	
	double distM_withThres(double lat, double lon, Edge* edge, double threshold) const; 
	double calEdgeLength(Figure* figure) const;
	bool inArea(double lat, double lon) const;
	bool inArea(int nodeId) const;
	void createGridIndex();
	void createGridIndexForEdge(Edge *edge);
	void insertEdgeIntoGrid(Edge* edge, int row, int col);
	void insertEdge(int edgeId, int startNodeId, int endNodeId);

	void split(const string& src, const string& separator, vector<string>& dest);
	double cosAngle(GeoPoint* pt1, GeoPoint* pt2, GeoPoint* pt3) const;
	void test();
};

