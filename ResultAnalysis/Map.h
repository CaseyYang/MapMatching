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

#define INF  1000000000 //���·������
#define MAXSPEED 50 //����ٶ�
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#define max(a,b)	(((a) > (b)) ? (a) : (b))
typedef list<GeoPoint*> Figure; //����һ��·�Σ�ÿ��GeoPoint*����·�ε㣬��β�ڵ㼴·�������˵�
typedef std::pair<double, double> simplePoint; //�ڲ����ͣ���Ķ�
typedef list<GeoPoint*> Traj;//�켣����

//���·�������������ݽṹ
struct NODE_DIJKSTRA {
	int t; double dist;
	NODE_DIJKSTRA(int i, double dist){
		this->t = i;
		this->dist = dist;
	}
	bool operator < (const NODE_DIJKSTRA &rhs) const {
		return dist > rhs.dist;
	}
};

struct  Edge
{
	Figure* figure;  //·����Ϣ
	double lengthM;  //��¼·���ܳ�����λΪm
	int startNodeId; //·����ʼ��id
	int endNodeId;  //·����ֹ��id
	bool visited;  //�����ֶΣ��ⲿ������Ķ�
	int id;
};

struct AdjNode //�ڽӱ���
{
	int endPointId;
	int edgeId;
	AdjNode* next;
};

class Map
{
public:
	vector<Edge*> edges; //�������бߵļ��ϣ�����ߵ������˵�����һ�����ڷ�Χ����ΪNULL��������������ֶ��ж�NULL��
	vector<GeoPoint*> nodes; //�������е�ļ���,����㲻�ڷ�Χ����ΪNULL��������������ֶ��ж�NULL��
	vector<AdjNode*> adjList; //�ڽӱ�

	//void setArea(MapDrawer& md); //��������md�����򱣳�һ��,����open֮ǰ���вι��캯��ǰ����
	Map(); //Ĭ�Ϲ��캯��,��Ҫ�ֶ�����open()��������ʼ��
	Map(string folderDir, int gridWidth);  //��folderDir·���������ͼ�ļ�,����gridWidth�е���������������

	//��folderDir·���������ͼ�ļ�,����gridWidth�е���������������,�������޲ι��캯��	
	void open(string folderDir, int gridWidth);
	//���ؾ���(lat, lon)���ϸ�С��threshold�׵�����Edge*
	vector<Edge*> getNearEdges(double lat, double lon, double threshold) const;
	//�ҳ����о���(lat, lon)���ϸ�С��threshold�׵�����Edge*��������dest������
	void getNearEdges(double lat, double lon, double threshold, vector<Edge*>& dest);
	//���ؾ���(lat, lon)�������Edge*
	Edge* getNearestEdge(double lat, double lon, double &shortestDist);
	//����(lat,lon)�㵽edge�ľ��룬��λΪ��
	double distM(double lat, double lon, Edge* edge) const;
	//ͬ�ϣ�ͬʱ��¼ͶӰ�㵽edge���ľ������prjDist����ͶӰ���Ϊ0
	double distM(double lat, double lon, Edge* edge, double& prjDist) const;
	//�ж�startNodeId��endNodeId֮�����ޱ�,û�б߷���-1���б߷���edgeId
	int hasEdge(int startNodeId, int endNodeId) const;
	//�ڵ�ǰͼ�в����
	void insertNode(double lat, double lon);
	//�ڵ�ǰͼ�в����
	void insertEdge(Edge* edge, int startNodeId, int endNodeId);
	void delEdge(int edgeId);
	/*
	A·����㵽B·��������С·������
	������
	ID1��A·�����
	ID2��B·�����
	dist1������������Ʒ�ģ�͵�ͼƥ���㷨�й켣�㵽B·�����ľ��룬Ĭ��ֵΪ0
	dist2������������Ʒ�ģ�͵�ͼƥ���㷨�й켣�㵽A·�����ľ��룬Ĭ��ֵΪ0
	deltaT������������Ʒ�ģ�͵�ͼƥ���㷨�����켣���ʱ��Ĭ��Ϊ100000000
	*/
	double shortestPathLength(int ID1, int ID2, double dist1 = 0, double dist2 = 0, double deltaT = 100000000);//A·����㵽B·��������С·������


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
	//�����к�row���к�row���ҳ���Ӧ��Ԫ������(lat,lon)�������·�Σ�������currentResultEdge�У�shortestDist������Ӧ����̾���
	void getNearestEdgeInAGridCell(double lat, double lon, int row, int col, Edge*& currentResultEdge, double &shortestDist);
	//����(lat,lon)�㵽edge�ľ����Ͻ�,��ǰԤ���Ż��汾
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

