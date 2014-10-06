#include "Map.h"


bool smallerInX(simplePoint pt1, simplePoint pt2);

//////////////////////////////////////////////////////////////////////////
///public part
//////////////////////////////////////////////////////////////////////////
Map::Map()
{

}

Map::Map(string folderDir, int gridWidth)
{
	this->open(folderDir, gridWidth);
}

void Map::open(string folderDir, int gridWidth)
{
	/*文件目录结构为
	* folderDir
	* |-WA_Nodes.txt
	* |-WA_EdgeGeometry.txt
	* |-WA_Edges.txt
	*/
	this->gridWidth = gridWidth;
	int count = 0;
	//////////////////////////////////////////////////////////////////////////
	//读取WA_Nodes.txt
	//格式：nodeId lat lon
	//////////////////////////////////////////////////////////////////////////
	ifstream nodeIfs(folderDir + "WA_Nodes.txt");
	if (!nodeIfs)
	{
		cout << "open " + folderDir + "WA_Nodes.txt" + " error!\n";
		system("pause");
		exit(0);
	}
	while (nodeIfs)
	{
		double lat, lon;
		int nodeId;
		GeoPoint* pt;
		nodeIfs >> nodeId >> lat >> lon;
		if (nodeIfs.fail()){
			break;
		}
		if (inArea(lat, lon)){
			pt = new GeoPoint(lat, lon);
		}
		else
		{
			pt = NULL;
			count++;
		}
		nodes.push_back(pt);
	}
	printf("nodes count = %d\n", nodes.size());
	printf("nodes not in area count = %d\n", count);
	nodeIfs.close();

	//////////////////////////////////////////////////////////////////////////
	//读取WA_EdgeGeometry.txt
	//格式：edgeId^^Highway^1^起始端点纬度^起始端点经度[^中间点1纬度^中间点1经度^中间点2纬度^中间点2经度.....]^结束端点纬度^结束端点经度    
	//////////////////////////////////////////////////////////////////////////
	count = 0;
	std::ifstream geometryIfs(folderDir + "WA_EdgeGeometry.txt");
	if (!geometryIfs)
	{
		cout << "open " + folderDir + "WA_EdgeGeometry.txt" + " error!\n";
		system("pause");
		exit(0);
	}
	std::string strLine;
	while (getline(geometryIfs, strLine))
	{
		if (geometryIfs.fail()){
			break;
		}
		std::vector<std::string> substrs;
		split(strLine, "^", substrs);
		Figure* figure = new Figure();
		int edgeId = atoi(substrs[0].c_str());
		double startLat = atof(substrs[3].c_str());
		double startLon = atof(substrs[4].c_str());
		double endLat = atof(substrs[substrs.size() - 2].c_str());
		double endLon = atof(substrs[substrs.size() - 1].c_str());
		if (!inArea(startLat, startLon) || !inArea(endLat, endLon))
		{
			edges.push_back(NULL);
			count++;
			continue;
		}
		for (int i = 3; i < substrs.size() - 1; i += 2)
		{
			double lat, lon;
			lat = atof(substrs[i].c_str());
			lon = atof(substrs[i + 1].c_str());
			GeoPoint* pt = new GeoPoint(lat, lon);
			figure->push_back(pt);
		}
		Edge* edge = new Edge();
		edge->id = edgeId;
		edge->visited = false;
		edge->figure = figure;
		edge->lengthM = calEdgeLength(figure);
		edges.push_back(edge);
	}
	printf("edges count = %d\n", edges.size());
	printf("not in area edges count = %d\n", count);
	geometryIfs.close();

	//////////////////////////////////////////////////////////////////////////
	//读取WA_Edges.txt
	//格式：edgeId startNodeId endNodeId 1
	//////////////////////////////////////////////////////////////////////////
	//初始化邻接表
	count = 0;
	int edgesCount = 0;
	for (int i = 0; i < edges.size(); ++i)
	{
		AdjNode* head = new AdjNode();
		head->endPointId = i;
		head->next = NULL;
		adjList.push_back(head);
	}
	std::ifstream edgeIfs(folderDir + "WA_Edges.txt");
	if (!edgeIfs)
	{
		cout << "open " + folderDir + "WA_Edges.txt" + " error!\n";
		system("pause");
		exit(0);
	}
	while (edgeIfs)
	{
		int edgeId, startNodeId, endNodeId, dummy;
		edgeIfs >> edgeId >> startNodeId >> endNodeId >> dummy;
		if (edgeIfs.fail()){
			break;
		}
		if (inArea(startNodeId) && inArea(endNodeId))
		{
			insertEdge(edgeId, startNodeId, endNodeId);
		}
		else
			count++;
	}
	edgeIfs.close();

	printf(">> reading map finished\n");
	createGridIndex();
	printf(">> creating grid index finished\n");
}

//void Map::setArea(MapDrawer& md)
//{
//	this->minLat = md.minLat;
//	this->maxLat = md.maxLat;
//	this->minLon = md.minLon;
//	this->maxLon = md.maxLon;
//}

//返回距离(lat, lon)点严格小于threshold米的所有Edge*
vector<Edge*> Map::getNearEdges(double lat, double lon, double threshold) const
{
	//////////////////////////////////////////////////////////////////////////
	///返回(lat, lon)周围距离小于threshold米的所有路段
	//////////////////////////////////////////////////////////////////////////
	vector<Edge*> result;
	vector<Edge*> fail;
	int gridSearchRange = int(threshold / (gridSizeDeg * GeoPoint::geoScale)) + 1;
	int rowPt = getRowId(lat);
	int colPt = getColId(lon);
	int row1 = rowPt - gridSearchRange;
	int col1 = colPt - gridSearchRange;
	int row2 = rowPt + gridSearchRange;
	int col2 = colPt + gridSearchRange;
	if (row1 < 0) {
		row1 = 0;
	}
	if (row2 >= gridHeight) {
		row2 = gridHeight - 1;
	}
	if (col1 < 0) {
		col1 = 0;
	}
	if (col2 >= gridWidth) {
		col2 = gridWidth - 1;
	}
	for (int row = row1; row <= row2; ++row)
	{
		for (int col = col1; col <= col2; ++col)
		{
			for (list<Edge*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); ++iter)
			{
				if (!((*iter)->visited))
				{
					(*iter)->visited = true;
					double dist = distM_withThres(lat, lon, (*iter), threshold);
					if (dist < threshold){
						result.push_back((*iter));
					}
					else{
						fail.push_back((*iter));
					}
				}
			}
		}
	}
	for (int i = 0; i < result.size(); ++i)
	{
		result[i]->visited = false;
	}
	for (int i = 0; i < fail.size(); ++i)
	{
		fail[i]->visited = false;
	}
	return result;
}

//找出所有距离(lat, lon)点严格小于threshold米的所有Edge*，保存在dest容器内
void Map::getNearEdges(double lat, double lon, double threshold, vector<Edge*>& dest)
{
	//////////////////////////////////////////////////////////////////////////
	///返回(lat, lon)周围距离小于threshold米的所有路段
	//////////////////////////////////////////////////////////////////////////
	dest.clear();
	vector<Edge*> fail;
	int gridSearchRange = int(threshold / (gridSizeDeg * GeoPoint::geoScale)) + 1;
	int rowPt = getRowId(lat);
	int colPt = getColId(lon);
	int row1 = rowPt - gridSearchRange;
	int col1 = colPt - gridSearchRange;
	int row2 = rowPt + gridSearchRange;
	int col2 = colPt + gridSearchRange;
	if (row1 < 0) {
		row1 = 0;
	}
	if (row2 >= gridHeight) {
		row2 = gridHeight - 1;
	}
	if (col1 < 0) {
		col1 = 0;
	}
	if (col2 >= gridWidth) {
		col2 = gridWidth - 1;
	}
	for (int row = row1; row <= row2; ++row)
	{
		for (int col = col1; col <= col2; ++col)
		{
			for (list<Edge*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); ++iter)
			{
				if (!((*iter)->visited))
				{
					(*iter)->visited = true;
					double dist = distM_withThres(lat, lon, (*iter), threshold);
					if (dist < threshold){
						dest.push_back((*iter));
					}
					else{
						fail.push_back((*iter));
					}
				}
			}
		}
	}
	for (int i = 0; i < dest.size(); ++i)
	{
		dest[i]->visited = false;
	}
	for (int i = 0; i < fail.size(); ++i)
	{
		fail[i]->visited = false;
	}
}

//返回距离(lat, lon)点最近的Edge*
Edge* Map::getNearestEdge(double lat, double lon, double &shortestDist) {
	//////////////////////////////////////////////////////////////////////////
	///返回离(lat, lon)点最近的路段
	///具体算法：
	///首先找出(lat, lon)点所在的单元格，以此为起点按正方形逐渐外扩（即range）、进行广度优先搜索，找到第一个离得最近的路段（while中的for循环）
	///按找到的离得最近的路段到(lat, lon)点的距离计算出广度优先搜索的最大搜索范围，即maxSearchRange，即最大外扩正方形（语句if (currentResultEdge != NULL)）
	///当外扩的range值大于等于maxSearchRange时，能保证找到一个离得最近的路段，同时没有其他路段比该路段更近，则结束寻找过程（语句noNearerEdge = true）
	///否则就循环外扩（语句while (!foundNearsetEdge || !noNearerEdge)）
	///最后返回找到的路段Edge指针，并保持相应的距离shortestDist
	//////////////////////////////////////////////////////////////////////////
	int rowPt = getRowId(lat);
	int colPt = getColId(lon);
	int range = 0;
	int maxSearchRange = 0;
	bool foundNearsetEdge = false;
	bool noNearerEdge = false;
	Edge* currentResultEdge = NULL;
	double currentShortestDist = 100000000.0;
	while ((!foundNearsetEdge || !noNearerEdge) && range <= max(gridHeight, gridWidth)){
		for (int row = rowPt - range; row <= rowPt + range; row++){
			if (row >= 0 && row < gridHeight){
				if (row == rowPt - range || row == rowPt + range){
					for (int col = colPt - range; col <= colPt + range; col++){
						if (col >= 0 && col < gridWidth){
							getNearestEdgeInAGridCell(lat, lon, row, col, currentResultEdge, currentShortestDist);
						}
						else{
							continue;
						}
					}
				}
				else{
					int col = colPt - range;
					if (col >= 0 && col < gridWidth){
						getNearestEdgeInAGridCell(lat, lon, row, col, currentResultEdge, currentShortestDist);
					}
					col = colPt + range;
					if (col >= 0 && col < gridWidth){
						getNearestEdgeInAGridCell(lat, lon, row, col, currentResultEdge, currentShortestDist);
					}
				}
			}
			else
			{
				continue;
			}
		}
		if (currentResultEdge != NULL){
			foundNearsetEdge = true;
			shortestDist = currentShortestDist;
			maxSearchRange = int(shortestDist / (gridSizeDeg * GeoPoint::geoScale)) + 1;
			if (range >= maxSearchRange) {
				noNearerEdge = true;
				break;
			}
		}
		range++;
	}
	return currentResultEdge;
}

//返回(lat,lon)点到edge的距离，单位为米
double Map::distM(double lat, double lon, Edge* edge) const
{
	//////////////////////////////////////////////////////////////////////////
	///返回点(lat, lon)到边edge的精确距离
	///距离定义为：min(点到可投影边的投影距离，点到所有形状点的欧氏距离)
	//////////////////////////////////////////////////////////////////////////
	double minDist = 100000000.0;
	//遍历端点距离
	for (Figure::iterator iter = edge->figure->begin(); iter != edge->figure->end(); iter++)
	{
		double tmpDist = GeoPoint::distM(lat, lon, (*iter)->lat, (*iter)->lon);
		if (tmpDist < minDist){
			minDist = tmpDist;
		}
	}
	//遍历投影距离
	Figure::iterator iter = edge->figure->begin();
	Figure::iterator nextIter = edge->figure->begin();
	nextIter++;
	while (nextIter != edge->figure->end())
	{
		//有投影
		GeoPoint* pt = new GeoPoint(lat, lon);
		if (cosAngle(pt, (*iter), (*nextIter)) <= 0 && cosAngle(pt, (*nextIter), (*iter)) <= 0)
		{
			double A = ((*nextIter)->lat - (*iter)->lat);
			double B = -((*nextIter)->lon - (*iter)->lon);
			double C = (*iter)->lat * ((*nextIter)->lon - (*iter)->lon)	- (*iter)->lon * ((*nextIter)->lat - (*iter)->lat);
			double tmpDist = abs(A * pt->lon + B * pt->lat + C) / sqrt(A * A + B * B);
			tmpDist *= GeoPoint::geoScale;
			if (minDist > tmpDist){
				minDist = tmpDist;
			}
		}
		delete pt;
		iter++;
		nextIter++;
	}
	return minDist;
}

//返回(lat,lon)点到edge的距离，单位为米；同时记录投影点到edge起点的距离存入prjDist，无投影则记为0
double Map::distM(double lat, double lon, Edge* edge, double& prjDist) const
{
	//////////////////////////////////////////////////////////////////////////
	///返回点(lat, lon)到边edge的精确距离
	///距离定义为：min(点到可投影边的投影距离，点到所有形状点的欧氏距离)
	///如果有投影的话，prjDist记录投影点到轨迹起点的距离，没有的话prjDist为0
	//////////////////////////////////////////////////////////////////////////
	Figure::iterator iter = edge->figure->begin();
	Figure::iterator nextIter = edge->figure->begin();
	nextIter++;
	prjDist = 0;
	double frontSegmentDist = 0;
	double tempTotalPrjDist = 0;
	double minDist = 100000000.0;
	//遍历端点距离
	while (nextIter != edge->figure->end())
	{
		double tmpDist = GeoPoint::distM(lat, lon, (*iter)->lat, (*iter)->lon);
		if (tmpDist < minDist)
		{
			minDist = tmpDist;
			tempTotalPrjDist = frontSegmentDist;
		}
		frontSegmentDist += GeoPoint::distM((*iter), (*nextIter));
		iter++;
		nextIter++;
	}
	//补最后一个点
	double tmpDist = GeoPoint::distM(lat, lon, (*iter)->lat, (*iter)->lon);
	if (tmpDist < minDist)
	{
		minDist = tmpDist;
		tempTotalPrjDist = frontSegmentDist;
	}
	//遍历投影距离
	frontSegmentDist = 0;
	iter = edge->figure->begin();
	nextIter = edge->figure->begin();
	nextIter++;
	while (nextIter != edge->figure->end())
	{
		//有投影
		GeoPoint* pt = new GeoPoint(lat, lon);
		if (cosAngle(pt, (*iter), (*nextIter)) <= 0 && cosAngle(pt, (*nextIter), (*iter)) <= 0)
		{
			double A = ((*nextIter)->lat - (*iter)->lat);
			double B = -((*nextIter)->lon - (*iter)->lon);
			double C = (*iter)->lat * ((*nextIter)->lon - (*iter)->lon)	- (*iter)->lon * ((*nextIter)->lat - (*iter)->lat);
			double tmpDist = abs(A * pt->lon + B * pt->lat + C) / sqrt(A * A + B * B);
			tmpDist *= GeoPoint::geoScale;
			if (minDist > tmpDist)
			{
				minDist = tmpDist;
				double tmpPjDist = GeoPoint::distM(pt, (*iter));
				tmpPjDist *= -cosAngle(pt, (*iter), (*nextIter));
				tempTotalPrjDist = frontSegmentDist + tmpPjDist;
			}
		}
		frontSegmentDist += GeoPoint::distM((*iter), (*nextIter));
		delete pt;
		iter++;
		nextIter++;
	}
	prjDist = tempTotalPrjDist;
	return minDist;
}

//判断startNodeId与endNodeId之间有无边,没有边返回-1，有边返回edgeId
int Map::hasEdge(int startNodeId, int endNodeId) const
{
	AdjNode* current = adjList[startNodeId]->next;
	while (current != NULL)
	{
		if (current->endPointId == endNodeId)
		{
			return current->edgeId;
		}
		else{
			current = current->next;
		}
	}
	return -1;
}

//在当前图中插入边
void Map::insertNode(double lat, double lon)
{
	if (!inArea(lat, lon)){
		return;
	}
	GeoPoint* pt = new GeoPoint(lat, lon);
	nodes.push_back(pt);
	AdjNode* adjNode = new AdjNode();
	adjNode->endPointId = adjList.size();
	adjNode->next = NULL;
	adjList.push_back(adjNode);
}

//在当前图中插入边
void Map::insertEdge(Edge* edge, int startNodeId, int endNodeId)
{
	edges.push_back(edge);
	AdjNode* current = adjList[startNodeId];
	insertEdge(edges.size() - 1, startNodeId, endNodeId);
}

void Map::delEdge(int edgeId)
{
	//【注意】会发生内存泄露
	edges[edgeId] = NULL;
}

/*
A路段起点到B路段起点的最小路网距离
参数：
ID1：A路段起点
ID2：B路段起点
dist1：基于隐马尔科夫模型地图匹配算法中轨迹点到B路段起点的距离，默认值为0
dist2：基于隐马尔科夫模型地图匹配算法中轨迹点到A路段起点的距离，默认值为0
deltaT：基于隐马尔科夫模型地图匹配算法中两轨迹点的时间差，默认为100000000
*/
double Map::shortestPathLength(int ID1, int ID2, double dist1, double dist2, double deltaT){
	int maxNodeNum = nodes.size();
	int size = maxNodeNum;
	//double *dist = new double[size];
	double dist[100000];
	//bool *flag = new bool[size];
	bool flag[100000];
	memset(flag, 0, sizeof(flag));
	for (int i = 0; i < size; ++i) {
		dist[i] = INF;
	}
	dist[ID1] = 0;
	priority_queue<NODE_DIJKSTRA> Q;
	NODE_DIJKSTRA tmp(ID1, 0);
	Q.push(tmp);
	while (!Q.empty()) {
		NODE_DIJKSTRA x = Q.top();
		Q.pop();
		int u = x.t;
		if (x.dist + dist1 - dist2 > deltaT*MAXSPEED){
			return INF;
		}
		if (flag[u]) {
			continue;
		}
		flag[u] = true;
		if (u == ID2) {
			break;
		}
		for (AdjNode* i = adjList[u]->next; i != NULL; i = i->next) {
			if (dist[i->endPointId] > dist[u] + edges[i->edgeId]->lengthM) {
				dist[i->endPointId] = dist[u] + edges[i->edgeId]->lengthM;
				NODE_DIJKSTRA tmp(i->endPointId, dist[i->endPointId]);
				Q.push(tmp);
			}
		}
	}
	double resultLength = dist[ID2];
	//delete []dist;
	//delete []flag;
	return resultLength;
}

//////////////////////////////////////////////////////////////////////////
///private part
//////////////////////////////////////////////////////////////////////////

//给定行号row和列号row，找出相应单元格中离(lat,lon)点最近的路段，保存在currentResultEdge中，shortestDist保存相应的最短距离
void Map::getNearestEdgeInAGridCell(double lat, double lon, int row, int col, Edge*& currentResultEdge, double &shortestDist){
	for (list<Edge*>::iterator edgeIter = grid[row][col]->begin(); edgeIter != grid[row][col]->end(); edgeIter++)
	{
		if (distM(lat, lon, (*edgeIter)) < shortestDist){
			shortestDist = distM(lat, lon, (*edgeIter));
			currentResultEdge = (*edgeIter);
		}
	}
}

double Map::distM_withThres(double lat, double lon, Edge* edge, double threshold) const
{
	//////////////////////////////////////////////////////////////////////////
	///返回点(lat, lon)到边edge的距离上界 【注意】不可用于计算精确距离！
	///距离定义为：min(点到可投影边的投影距离，点到所有形状点的欧氏距离)
	///如果更新上界时发现已经低于threshold(单位米)则直接返回
	//////////////////////////////////////////////////////////////////////////
	double minDist = 100000000.0;
	//遍历端点距离
	for (Figure::iterator iter = edge->figure->begin(); iter != edge->figure->end(); iter++)
	{
		double tmpDist = GeoPoint::distM(lat, lon, (*iter)->lat, (*iter)->lon);
		if (tmpDist < threshold){
			return tmpDist;
		}
		if (tmpDist < minDist){
			minDist = tmpDist;
		}
	}
	//遍历投影距离
	Figure::iterator iter = edge->figure->begin();
	Figure::iterator nextIter = edge->figure->begin();
	nextIter++;
	while (nextIter != edge->figure->end())
	{
		//有投影
		GeoPoint* pt = new GeoPoint(lat, lon);
		if (cosAngle(pt, (*iter), (*nextIter)) <= 0 && cosAngle(pt, (*nextIter), (*iter)) <= 0)
		{
			double A = ((*nextIter)->lat - (*iter)->lat);
			double B = -((*nextIter)->lon - (*iter)->lon);
			double C = (*iter)->lat * ((*nextIter)->lon - (*iter)->lon)	- (*iter)->lon * ((*nextIter)->lat - (*iter)->lat);
			double tmpDist = abs(A * pt->lon + B * pt->lat + C) / sqrt(A * A + B * B);
			tmpDist *= GeoPoint::geoScale;
			if (tmpDist < threshold){
				return tmpDist;
			}
			if (minDist > tmpDist){
				minDist = tmpDist;
			}
		}
		delete pt;
		iter++;
		nextIter++;
	}
	return minDist;
}

double Map::calEdgeLength(Figure* figure) const
{
	//////////////////////////////////////////////////////////////////////////
	///计算路段的长度，单位为m
	//////////////////////////////////////////////////////////////////////////
	double lengthM = 0;
	Figure::iterator ptIter = figure->begin(), nextPtIter = ptIter;
	nextPtIter++;
	while (1)
	{
		if (nextPtIter == figure->end()){
			break;
		}
		lengthM += GeoPoint::distM((*ptIter)->lat, (*ptIter)->lon, (*nextPtIter)->lat, (*nextPtIter)->lon);
		ptIter++;
		nextPtIter++;
	}
	return lengthM;
}

bool Map::inArea(double lat, double lon) const
{
	return (lat > minLat && lat < maxLat && lon > minLon && lon < maxLon);
}

bool Map::inArea(int nodeId) const
{
	return (nodes[nodeId] != NULL);
}

void Map::test()
{
	int* flag = new int[nodes.size()];
	for (int i = 0; i < nodes.size(); ++i)
	{
		flag[i] = -1;
	}
	for (int i = 0; i < adjList.size(); ++i)
	{
		if (i % 1000 == 0)
		{
			cout << i << endl;
		}
		AdjNode* current = adjList[i]->next;
		while (current != NULL)
		{
			int j = current->endPointId;
			/*int edgeIJId = current->edgeId;
			int edgeJIId = hasEdge(j, i);
			if (edgeJIId != -1)
			{
			Edge* edgeIJ = edges[edgeIJId];
			Edge* edgeJI = edges[edgeJIId];
			if (edgeIJ->size() != edgeJI->size())
			{
			cout << "XXXXXX" << endl;
			}
			Edge::iterator iter1 = edgeIJ->begin();
			Edge::iterator iter2 = edgeJI->end();
			iter2--;
			while (iter2 != edgeJI->begin())
			{
			if (abs((*iter1)->lat - (*iter2)->lat) > 1e-8 ||
			abs((*iter1)->lon - (*iter2)->lon) > 1e-8)
			{
			cout << "YYYYYY" << endl;
			printf("%lf,%lf,%lf,%lf\n", (*iter1)->lat, (*iter2)->lat, (*iter1)->lon, (*iter2)->lon);
			printf("edgeIJ = %d, edgeJI = %d", edgeIJId, edgeJIId);
			system("pause");
			}
			//printf("%lf,%lf,%lf,%lf\n", (*iter1)->lat, (*iter2)->lat, (*iter1)->lon, (*iter2)->lon);
			//system("pause");
			iter2--;
			iter1++;
			}
			}*/
			if (flag[j] == i)
			{
				cout << "ZZZZZZZ" << endl;
				printf("%d, %d\n", i, j);
				system("pause");
			}
			flag[j] = i;
			current = current->next;
		}
	}
}

void Map::createGridIndex()
{
	//////////////////////////////////////////////////////////////////////////
	///对全图建立网格索引
	//////////////////////////////////////////////////////////////////////////
	//initialization
	gridHeight = int((maxLat - minLat) / (maxLon - minLon) * double(gridWidth)) + 1;
	gridSizeDeg = (maxLon - minLon) / double(gridWidth);
	grid = new list<Edge*>* *[gridHeight];
	for (int i = 0; i < gridHeight; ++i){
		grid[i] = new list<Edge*>*[gridWidth];
	}
	for (int i = 0; i < gridHeight; ++i)
	{
		for (int j = 0; j < gridWidth; ++j)
		{
			grid[i][j] = new list<Edge*>();
		}
	}
	printf("Map index gridWidth = %d, gridHeight = %d\n", gridWidth, gridHeight);
	cout << "gridSize = " << gridSizeDeg * GeoPoint::geoScale << "m" << endl;
	for (vector<Edge*>::iterator edgeIter = edges.begin(); edgeIter != edges.end(); ++edgeIter)
	{
		createGridIndexForEdge((*edgeIter));
	}
}

void Map::insertEdgeIntoGrid(Edge* edge, int row, int col)
{
	//////////////////////////////////////////////////////////////////////////
	///将路段edge加入grid[row][col]中索引，如果已经加入过则不添加
	//////////////////////////////////////////////////////////////////////////
	if (grid[row][col]->size() > 0 && grid[row][col]->back() == edge){
		return;
	}
	else{
		grid[row][col]->push_back(edge);
	}
}

void Map::createGridIndexForEdge(Edge *edge)
{
	//////////////////////////////////////////////////////////////////////////
	///对edge路段插入网格索引，经过的网格都加入其指针，如果与网格相交长度过小则不加入网格
	//////////////////////////////////////////////////////////////////////////
	if (edge == NULL){
		return;
	}
	bool crossRow;
	GeoPoint* pt1 = edge->figure->front();
	GeoPoint* pt2 = edge->figure->back();
	double x1 = pt1->lon - minLon;
	double y1 = pt1->lat - minLat;
	double x2 = pt2->lon - minLon;
	double y2 = pt2->lat - minLat;
	int row1 = y1 / gridSizeDeg;
	int row2 = y2 / gridSizeDeg;
	int col1 = x1 / gridSizeDeg;
	int col2 = x2 / gridSizeDeg;
	double A = y2 - y1;
	double B = -(x2 - x1);
	double C = -B * y1 - A * x1;
	int i, j;
	//pt1,pt2都在一个cell中
	if (row1 == row2 && col1 == col2)
	{
		insertEdgeIntoGrid(edge, row1, col1);
		return;
	}
	//只穿越横向格子
	if (row1 == row2)
	{
		//头
		double headDist = ((min(col1, col2) + 1) * gridSizeDeg - min(x1, x2)) / gridSizeDeg;
		if (headDist / gridSizeDeg > strictThreshold){
			insertEdgeIntoGrid(edge, row1, min(col1, col2));
		}
		//中间
		for (i = min(col1, col2) + 1; i < max(col1, col2); ++i)
		{
			insertEdgeIntoGrid(edge, row1, i);
		}
		//尾
		double tailDist = (max(x1, x2) - max(col1, col2) * gridSizeDeg) / gridSizeDeg;
		if (tailDist / gridSizeDeg > strictThreshold){
			insertEdgeIntoGrid(edge, row1, max(col1, col2));
		}
		return;
	}
	//只穿越纵向格子
	if (col1 == col2)
	{
		//头
		double headDist = ((min(row1, row2) + 1) * gridSizeDeg - min(y1, y2)) / gridSizeDeg;
		if (headDist / gridSizeDeg > strictThreshold){
			insertEdgeIntoGrid(edge, min(row1, row2), col1);
		}
		//中间
		for (i = min(row1, row2) + 1; i < max(row1, row2); ++i)
		{
			insertEdgeIntoGrid(edge, i, col1);
		}
		//尾
		double tailDist = (max(y1, y2) - max(row1, row2) * gridSizeDeg) / gridSizeDeg;
		if (tailDist / gridSizeDeg > strictThreshold){
			insertEdgeIntoGrid(edge, max(row1, row2), col1);
		}
		return;
	}
	simplePoint pts[1000];
	int n_pts = 0;
	for (i = min(row1, row2) + 1; i <= max(row1, row2); ++i)
	{
		pts[n_pts++] = std::make_pair((-C - B*i*gridSizeDeg) / A, i*gridSizeDeg);
	}
	for (i = min(col1, col2) + 1; i <= max(col1, col2); ++i)
	{
		pts[n_pts++] = std::make_pair(i*gridSizeDeg, (-C - A*i*gridSizeDeg) / B);
	}
	std::sort(pts, pts + n_pts, smallerInX);

	GeoPoint* leftPt, *rightPt;
	if (x1 < x2)
	{
		leftPt = pt1;
		rightPt = pt2;
	}
	else
	{
		leftPt = pt2;
		rightPt = pt1;
	}
	double xL = leftPt->lon - minLon;
	double xR = rightPt->lon - minLon;
	double yL = leftPt->lat - minLat;
	double yR = rightPt->lat - minLat;

	//头
	double headDist = sqrt((xL - pts[0].first)*(xL - pts[0].first) + (yL - pts[0].second)*(yL - pts[0].second)) / gridSizeDeg;
	if (headDist / gridSizeDeg > strictThreshold){
		insertEdgeIntoGrid(edge, (int)(yL / gridSizeDeg), (int)(xL / gridSizeDeg));
	}
	//中间
	for (i = 0; i < n_pts - 1; ++i)
	{
		double dist = sqrt((pts[i].first - pts[i + 1].first)*(pts[i].first - pts[i + 1].first) + (pts[i].second - pts[i + 1].second)*(pts[i].second - pts[i + 1].second)) / gridSizeDeg;
		if (dist / gridSizeDeg > strictThreshold)
			//insertEdgeIntoGrid(edge, getRowId(pts[i], pts[i + 1]), getColId(pts[i], pts[i + 1]));
		{
			int row = (int)(min(pts[i].second, pts[i + 1].second) / gridSizeDeg);
			int col = (int)(min(pts[i].first, pts[i + 1].first) / gridSizeDeg);
			insertEdgeIntoGrid(edge, row, col);
		}
	}
	//尾
	double tailDist = sqrt((xR - pts[n_pts - 1].first)*(xR - pts[n_pts - 1].first) + (yR - pts[n_pts - 1].second)*(yR - pts[n_pts - 1].second)) / gridSizeDeg;
	if (tailDist / gridSizeDeg > strictThreshold){
		insertEdgeIntoGrid(edge, (int)(yR / gridSizeDeg), (int)(xR / gridSizeDeg));
	}
	return;
}

void Map::insertEdge(int edgeId, int startNodeId, int endNodeId)
{
	//////////////////////////////////////////////////////////////////////////
	///向邻接表adjList中插入一条边的连通关系，初次构建图时使用，私有版本，不允许外部调用
	//////////////////////////////////////////////////////////////////////////

	AdjNode* current = adjList[startNodeId];
	while (current->next != NULL)
	{
		current = current->next;
	}
	AdjNode* tmpAdjNode = new AdjNode();
	tmpAdjNode->endPointId = endNodeId;
	tmpAdjNode->edgeId = edgeId;
	tmpAdjNode->next = NULL;
	current->next = tmpAdjNode;
	edges[edgeId]->startNodeId = startNodeId;
	edges[edgeId]->endNodeId = endNodeId;
}

int Map::getRowId(double lat) const
{
	return (int)((lat - minLat) / gridSizeDeg);
}

int Map::getColId(double lon) const
{
	return (int)((lon - minLon) / gridSizeDeg);
}

double Map::cosAngle(GeoPoint* pt1, GeoPoint* pt2, GeoPoint* pt3) const
{
	double v1x = pt2->lon - pt1->lon;
	double v1y = pt2->lat - pt1->lat;
	double v2x = pt3->lon - pt2->lon;
	double v2y = pt3->lat - pt2->lat;
	return (v1x * v2x + v1y * v2y) / sqrt((v1x * v1x + v1y * v1y)*(v2x * v2x + v2y * v2y));
}

void Map::split(const string& src, const string& separator, vector<string>& dest)
{
	std::string str = src;
	std::string substring;
	std::string::size_type start = 0, index;
	do
	{
		index = str.find_first_of(separator, start);
		if (index != std::string::npos)
		{
			substring = str.substr(start, index - start);
			dest.push_back(substring);
			start = str.find_first_not_of(separator, index);
			if (start == std::string::npos) return;
		}
	} while (index != std::string::npos);
	//the last token
	substring = str.substr(start);
	dest.push_back(substring);
}

bool smallerInX(simplePoint pt1, simplePoint pt2)
{
	//////////////////////////////////////////////////////////////////////////
	///将直线与网格交点统一按照x轴递增方向排列
	//////////////////////////////////////////////////////////////////////////
	return pt1.first < pt2.first;
}

