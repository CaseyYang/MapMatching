/* 
 * Last Updated at [2014/4/3 11:42] by wuhao
 */
#pragma once
#include "GeoPoint.h"
#include <map>
#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <queue>
#include "MapDrawer.h"
#define eps 1e-10
#define INF  1e7 //最短路径所用
#define MAXSPEED 50 //最大速度
using namespace std;
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#define max(a,b)	(((a) > (b)) ? (a) : (b))
typedef list<GeoPoint*> Figure; //代表一条路形，每个GeoPoint*代表路形点，首尾节点即路的两个端点
typedef std::pair<double, double> simplePoint; //内部类型，勿改动
typedef std::map<pair<double, double>, int> NodeMap;

struct  Edge
{
	Figure* figure;  //路形信息
	double lengthM;  //记录路段总长，单位为m
	int startNodeId; //路段起始点id
	int endNodeId;  //路段终止点id
	bool visited;  //辅助字段，外部调用勿改动

	int rank; //道路功能等级分为1到5级,暂时无用
	string level; //路段等级，前两位
	string property; //路段属性，后两位


	int id; //不用百度的编号，用Edge在edges数组中的索引号作为id
};

struct AdjNode //邻接表结点
{
	int endPointId;
	int edgeId;
	AdjNode* next;
};

//最短路径长度所用数据结构
struct NODE_DIJKSTRA
{
	int t; double dist;
	NODE_DIJKSTRA(int i, double dist)
	{
		this->t = i;
		this->dist = dist;
	}
	bool operator < (const NODE_DIJKSTRA &rhs) const
	{
		return dist > rhs.dist;
	}
};

struct cmp_node
{
	bool operator()(const pair<double, double>& keyA, const pair<double, double>& keyB) const
	{
		if (keyA.first < keyB.first)
			return true;
		else if (keyA.first > keyB.first)
			return false;
		else
			return keyA.second < keyB.second;
	}
};


class BaiduMap
{
public:
	vector<Edge*> edges; //保存所有边的集合，如果边的两个端点至少一个不在范围内则为NULL，【逐个遍历需手动判断NULL】
	vector<GeoPoint*> nodes; //保存所有点的集合,如果点不在范围内则为NULL，【逐个遍历需手动判断NULL】
	vector<AdjNode*> adjList; //邻接表，最初每个位置放一个next指针指向NULL的无用邻接节点

	void setArea(MapDrawer& md); //将区域与md的区域保持一致,需在open之前或有参构造函数前调用
	BaiduMap(); //默认构造函数,需要手动调用open()函数来初始化
	BaiduMap(string folderDir, int gridWidth);  //在folderDir路径下载入地图文件,并以gridWidth列的粒度来创建索引

	void open(string folderDir, int gridWidth);  //在folderDir路径下载入地图文件,并以gridWidth列的粒度来创建索引,适用于无参构造函数	
	vector<Edge*> getNearEdges(double lat, double lon, double threshold) const; //返回距离(lat, lon)点严格小于threshold米的所有Edge*,会产生内存泄露
	void getNearEdges(double lat, double lon, double threshold, vector<Edge*>& dest); //推荐版本
	void getNearEdges(double lat, double lon, size_t k, vector<Edge*>& dest); //返回离(lat, lon)点距离最近的k条路段，存入dest
	double shortestPathLength(int ID1, int ID2, list<Edge*> &shortestPath, double dist1 = 0, double dist2 = 0, double deltaT = INF);
	double distM(double lat, double lon, Edge* edge) const; //返回(lat,lon)点到edge的距离，单位为米
	double distM(double lat, double lon, Edge* edge, double& prjDist) const;//同上，同时记录投影点到edge起点的距离存入prjDist，无投影则记为0	
	double distMFromTransplantFromSRC(double lat, double lon, Edge* edge, double& prjDist);//移植SRC版本：返回(lat,lon)点到edge的距离，单位为米；同时记录投影点到edge起点的距离存入prjDist
	int hasEdge(int startNodeId, int endNodeId) const; //判断startNodeId与endNodeId之间有无边,没有边返回-1，有边返回edgeId
	int insertNode(double lat, double lon); //插入一个新结点,返回新结点id
	int insertEdge(Figure* figure, int startNodeId, int endNodeId); //在当前图中插入边,返回新边id
	int splitEdge(int edgeId, double lat, double lon); //将edge在(lat,lon)点处分开成两段,(lat,lon)作为新结点加入,返回新结点的nodeId
	void delEdge(int edgeId);
	void getMinMaxLatLon(string nodeFilePath);

	//可视化部分
	void drawMap(Gdiplus::Color color, MapDrawer& md);
	void drawGridLine(Gdiplus::Color color, MapDrawer& md);

	//private:
	NodeMap node_map;
	int gridWidth, gridHeight;
	double gridSizeDeg;
	double strictThreshold = 0;
	list<Edge*>* **grid;

	//hefei full
	double minLat = 31.853;
	double maxLat = 31.893;
	double minLon = 117.250;
	double maxLon = 117.336;

	int getRowId(double lat) const;
	int getColId(double lon) const;
	double distM_withThres(double lat, double lon, Edge* edge, double threshold) const; //返回(lat,lon)点到edge的距离上界,提前预判优化版本	
	double calEdgeLength(Figure* figure) const;
	bool inArea(double lat, double lon) const;
	bool inArea(int nodeId) const;
	void createGridIndex();
	void createGridIndexForEdge(Edge *edge);
	void createGridIndexForSegment(Edge *edge, GeoPoint* fromPT, GeoPoint* toPt);
	void insertEdgeIntoGrid(Edge* edge, int row, int col);
	void insertEdge(int edgeId, int startNodeId, int endNodeId);

	void split(const string& src, const string& separator, vector<string>& dest);
	void split(const string& src, const char& separator, vector<string>& dest);
	double cosAngle(GeoPoint* pt1, GeoPoint* pt2, GeoPoint* pt3) const;
	void test();
};

