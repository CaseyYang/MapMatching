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
using namespace std;

#define eps 1e-10
#define INF  1e7
#define MAXSPEED 50
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#define max(a,b)	(((a) > (b)) ? (a) : (b))
typedef list<GeoPoint*> Figure;
typedef std::pair<double, double> simplePoint;
typedef list<GeoPoint*> Traj;

struct NODE_DIJKSTRA {
	int t;
	double dist;

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
	Figure* figure;
	double lengthM;
	int startNodeId;
	int endNodeId;
	bool visited;
	int id;
};

struct AdjNode
{
	int endPointId;
	int edgeId;
	AdjNode* next;
};

class Map
{
public:
	vector<Edge*> edges;
	vector<GeoPoint*> nodes;
	vector<AdjNode*> adjList;

	Map();
	Map(string folderDir, int gridWidth);

	void open(string folderDir, int gridWidth);
	vector<Edge*> getNearEdges(double lat, double lon, double threshold) const;
	void getNearEdges(double lat, double lon, double threshold, vector<Edge*>& dest);
	void getNearEdges(double lat, double lon, size_t k, vector<Edge*>& dest);
	Edge* getNearestEdge(double lat, double lon, double &shortestDist);
	vector<Edge*> getKNearEdges(double lat, double lon, size_t k);
	double distM(double lat, double lon, Edge* edge) const;
	double distM(double lat, double lon, Edge* edge, double& prjDist) const;
	double Map::distMFromTransplantFromSRC(double lat, double lon, Edge* edge, double& prjDist);
	int hasEdge(int startNodeId, int endNodeId) const;
	int insertNode(double lat, double lon);
	int insertEdge(Figure* figure, int startNodeId, int endNodeId);
	int splitEdge(int edgeId, double lat, double lon);
	void delEdge(int edgeId);
	void getMinMaxLatLon(string nodeFilePath);
	double shortestPathLength(int ID1, int ID2, list<Edge*> &shortestPath, double dist1 = 0, double dist2 = 0, double deltaT = INF);

	int gridWidth, gridHeight;
	double gridSizeDeg;
	double strictThreshold = 0;
	list<Edge*>* **grid;

	//singapore full
	double minLat = 0.99999;
	double maxLat = 1.6265;
	double minLon = 103.548;
	double maxLon = 104.1155;

	int getRowId(double lat) const;
	int getColId(double lon) const;
	void getNearestEdgeInAGridCell(double lat, double lon, int row, int col, Edge*& currentResultEdge, double &shortestDist);
	bool Map::getEdgesInAGridCell(double lat, double lon, int row, int col, vector<pair<Edge*, double>> &resultEdges, set<int> &visitedEdgeIdSet);
	double distM_withThres(double lat, double lon, Edge* edge, double threshold) const;
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
};

