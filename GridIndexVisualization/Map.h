#pragma once
#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <queue>
#include <set>
#include <map>
#include "GeoPoint.h"
#include "Area.h"
using namespace std;

#define eps 1e-10
#define INF  1e7 //���·������
#define MAXSPEED 50 //����ٶ�
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#define max(a,b)	(((a) > (b)) ? (a) : (b))
typedef list<GeoPoint*> Figure; //����һ��·�Σ�ÿ��GeoPoint*����·�ε㣬��β�ڵ㼴·�������˵�
typedef std::pair<double, double> simplePoint; //�ڲ����ͣ���Ķ�
typedef list<GeoPoint*> Traj;//�켣����

//���·�������������ݽṹ
struct NODE_DIJKSTRA {
	int t; //��Ӧ��·���յ�id
	double dist; //����ʼ�����С����

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
public:
	Figure* figure;  //·����Ϣ
	double lengthM;  //��¼·���ܳ�����λΪm
	int startNodeId; //·����ʼ��id
	int endNodeId;  //·����ֹ��id
	bool visited;  //�����ֶΣ��ⲿ������Ķ�
	int id;
	~Edge(){
		for (auto iter = this->figure->begin(); iter != this->figure->end(); ++iter){
			delete *iter;
		}
		delete this->figure;
	}
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

	//Ĭ�Ϲ��캯��,��Ҫ�ֶ�����open()��������ʼ��
	Map();
	//��folderDir·���������ͼ�ļ�,����gridWidth�е���������������
	Map(string folderDir, int gridWidth);
	//��������
	~Map();

	//��folderDir·���������ͼ�ļ�,����gridWidth�е���������������,�������޲ι��캯��	
	void open(string folderDir, int gridWidth);
	//�������о���(lat, lon)���ϸ�С��threshold�׵�����Edge*
	vector<Edge*> getNearEdges(double lat, double lon, double threshold) const;
	//�ҳ����о���(lat, lon)���ϸ�С��threshold�׵�����Edge*��������dest������
	void getNearEdges(double lat, double lon, double threshold, vector<Edge*>& dest); //�Ƽ��汾
	//������(lat, lon)����������k��·�Σ�����dest
	void getNearEdges(double lat, double lon, size_t k, vector<Edge*>& dest);
	//���ؾ���(lat, lon)�������Edge*
	Edge* getNearestEdge(double lat, double lon, double &shortestDist);
	//�ҳ�����(lat, lon)�������k��·��
	vector<Edge*> getKNearEdges(double lat, double lon, size_t k);
	//����(lat,lon)�㵽edge�ľ��룬��λΪ��
	double distM(double lat, double lon, Edge* edge) const;
	//ͬ�ϣ�ͬʱ��¼ͶӰ�㵽edge���ľ������prjDist����ͶӰ���Ϊ0
	double distM(double lat, double lon, Edge* edge, double& prjDist) const;
	//��ֲSRC�汾������(lat,lon)�㵽edge�ľ��룬��λΪ�ף�ͬʱ��¼ͶӰ�㵽edge���ľ������prjDist
	double distMFromTransplantFromSRC(double lat, double lon, Edge* edge, double& prjDist);
	//����(lat,lon)�����ڵ�����������
	pair<int, int> findGridCellIndex(double lat, double lon);
	//�ж�startNodeId��endNodeId֮�����ޱ�,û�б߷���-1���б߷���edgeId
	int hasEdge(int startNodeId, int endNodeId) const;
	//����һ���½��,�����½��id
	int insertNode(double lat, double lon);
	//�ڵ�ǰͼ�в����,�����±�id
	int insertEdge(Figure* figure, int startNodeId, int endNodeId);
	//��edge��(lat,lon)�㴦�ֿ�������,(lat,lon)��Ϊ�½�����,�����½���nodeId
	int splitEdge(int edgeId, double lat, double lon);
	void delEdge(int edgeId);
	//�ӵ㼯���ļ��л�õ�ͼ��Χ�����ڵ㼯���ҳ��߽羭γ��
	void setMapRange(string nodeFilePath);
	//��һ��Area����ʵ������ʽ���ص�ͼ�ı߽羭γ��
	Area* getMapRange();
	/*
	A·����㵽B·��������С·������
	������
	ID1��A·�����
	ID2��B·�����
	dist1������������Ʒ�ģ�͵�ͼƥ���㷨�й켣�㵽B·�����ľ��룬Ĭ��ֵΪ0
	dist2������������Ʒ�ģ�͵�ͼƥ���㷨�й켣�㵽A·�����ľ��룬Ĭ��ֵΪ0
	deltaT������������Ʒ�ģ�͵�ͼƥ���㷨�����켣���ʱ��Ĭ��Ϊ100000000
	*/
	double shortestPathLength(int ID1, int ID2, list<Edge*> &shortestPath, double dist1 = 0, double dist2 = 0, double deltaT = INF);

private:
	int gridWidth, gridHeight;
	double gridSizeDeg;
	double strictThreshold = 0;
	list<Edge*>* **grid;
	//singapore half
	//double minLat = 1.294788;
	//double maxLat = 1.327723;
	//double minLon = 103.784667;
	//double maxLon = 103.825200;

	//singapore full
	double minLat = 0.99999;
	double maxLat = 1.6265;
	double minLon = 103.548;
	double maxLon = 104.1155;

	//washington full
	//double minLat = 45.0;
	//double maxLat = 49.5;
	//double minLon = -125.0;
	//double maxLon = -116.5;

	int getRowId(double lat) const;
	int getColId(double lon) const;
	//�����к�row���к�row���ҳ���Ӧ��Ԫ������(lat,lon)�������·�Σ�������currentResultEdge�У�shortestDist������Ӧ����̾���
	void getNearestEdgeInAGridCell(double lat, double lon, int row, int col, Edge*& currentResultEdge, double &shortestDist);
	//�����к�row���к�row��������Ӧ��Ԫ��������·�α�����resultEdges�У��������Ӧ��EdgeId������visitedEdgeIdSet��
	bool Map::getEdgesInAGridCell(double lat, double lon, int row, int col, vector<pair<Edge*, double>> &resultEdges, set<int> &visitedEdgeIdSet);
	//����(lat,lon)�㵽edge�ľ����Ͻ�,��ǰԤ���Ż��汾
	double distM_withThres(double lat, double lon, Edge* edge, double threshold) const;
	//����·�εĳ��ȣ���λΪm
	double calEdgeLength(Figure* figure) const;
	bool inArea(double lat, double lon) const;
	bool inArea(int nodeId) const;
	//��ȫͼ������������
	void createGridIndex();
	void createGridIndexForEdge(Edge *edge);
	//��·��edge�е�fromPt->toPt�β����������������������񶼼�����ָ�룬����������ཻ���ȹ�С�򲻼�������
	void createGridIndexForSegment(Edge *edge, GeoPoint* fromPT, GeoPoint* toPt);
	//��·��edge����grid[row][col]������������Ѿ�����������
	void insertEdgeIntoGrid(Edge* edge, int row, int col);
	//���ڽӱ�adjList�в���һ���ߵ���ͨ��ϵ�����ι���ͼʱʹ�ã�˽�а汾���������ⲿ����
	void insertEdge(int edgeId, int startNodeId, int endNodeId);

	void split(const string& src, const string& separator, vector<string>& dest);
	void split(const string& src, const char& separator, vector<string>& dest);
	double cosAngle(GeoPoint* pt1, GeoPoint* pt2, GeoPoint* pt3) const;
};

