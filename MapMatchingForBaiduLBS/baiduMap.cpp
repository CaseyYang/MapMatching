/*
 * Last Updated at [2014/4/3 11:42] by wuhao
 */
#include "BaiduMap.h"

bool smallerInX(simplePoint& pt1, simplePoint& pt2);
bool smallerInDist(pair<Edge*, double>& c1, pair<Edge*, double>& c2);

//////////////////////////////////////////////////////////////////////////
///public part
//////////////////////////////////////////////////////////////////////////
BaiduMap::BaiduMap()
{

}

BaiduMap::BaiduMap(string folderDir, int gridWidth)
{
	this->open(folderDir, gridWidth);
}

void BaiduMap::open(string folderDir, int gridWidth)
{
	/*�ļ�Ŀ¼�ṹΪ
	* folderDir
	* |-link_hefei.txt
	* |-ref_points_hefei.txt
	* |-connectivity_hefei.txt
	*/
	this->gridWidth = gridWidth;
	//int count = 0;

	//
	/**********************************************************/
	/*test code starts from here*/
	double minLat = 999, maxLat = -1, minLon = 999, maxLon = -1;
	/*test code ends*/
	/**********************************************************/


	//////////////////////////////////////////////////////////////////////////
	//��ȡlink_hefei.txt
	//��ʽ��edgeId ��·���� rank length(KM) 4λ��·���� ��ʼ��lon ��ʼ��lat �յ�lon �յ�lat    
	//////////////////////////////////////////////////////////////////////////
	//count = 0;
	int edgeCount = 0;
	int nodeCount = 0;
	std::ifstream edgeIfs(folderDir + "link_hefei.txt");
	if (!edgeIfs)
	{
		cout << "open " + folderDir + "link_hefei.txt" + " error!\n";
		system("pause");
		exit(0);
	}
	while (edgeIfs)
	{
		long long id;
		string name;
		int rank;
		double lengthKM;
		string edgeInfo;
		double startLon, startLat, endLon, endLat;

		edgeIfs >> id >> name >> rank >> lengthKM >> edgeInfo
			>> startLon >> startLat >> endLon >> endLat;
		if (edgeIfs.fail())
		{
			break;
		}
		Edge* tmpEdge = new Edge();

		//tmpEdge->id = id;
		tmpEdge->id = edgeCount;
		tmpEdge->rank = rank;
		tmpEdge->lengthM = lengthKM / 1000;
		//TODO: edgeInfo
		NodeMap::iterator iter = node_map.find(make_pair(startLat, startLon));
		if (iter == node_map.end())
		{
			node_map.insert(make_pair(make_pair(startLat, startLon), nodeCount));
			nodes.push_back(new GeoPoint(startLat, startLon));
			AdjNode* adjNode = new AdjNode();
			adjNode->next = NULL;
			adjList.push_back(adjNode);
			tmpEdge->startNodeId = nodeCount;
		}
		else
		{
			tmpEdge->startNodeId = (*iter).second;
		}

		iter = node_map.find(make_pair(endLat, endLon));
		if (iter == node_map.end())
		{
			node_map.insert(make_pair(make_pair(endLat, endLon), nodeCount));
			nodes.push_back(new GeoPoint(endLat, endLon));
			AdjNode* adjNode = new AdjNode();
			adjNode->next = NULL;
			adjList.push_back(adjNode);
			nodeCount++;
			tmpEdge->endNodeId = nodeCount;
		}
		else
		{
			tmpEdge->endNodeId = (*iter).second;
		}
		edges.push_back(tmpEdge);
		insertEdge(edgeCount, tmpEdge->startNodeId, tmpEdge->endNodeId);
		edgeCount++;
		//TODO: AdjList
	}

	printf("edges count = %d\n", edges.size());
	printf("nodes count = %d\n", nodeCount); //4607
	//printf("not in area edges count = %d\n", count);
	edgeIfs.close();

	//////////////////////////////////////////////////////////////////////////
	//��ȡref_points_hefei.txt
	//��ʽ����id(����) edgeId ·�ε�id lon lat
	//////////////////////////////////////////////////////////////////////////
	ifstream figureIfs(folderDir + "ref_points_hefei.txt");
	if (!figureIfs)
	{
		cout << "open " + folderDir + "ref_points_hefei.txt" + " error!\n";
		system("pause");
		exit(0);
	}
	bool startFlag = true;
	double lat, lon;
	int figureId;
	long long edgeId;
	int currentEdgeIndex = 0;
	Figure* figure = NULL;
	while (figureIfs)
	{
		int dummyId;

		figureIfs >> dummyId >> edgeId >> figureId >> lon >> lat;
		if (figureIfs.fail())
		{
			edges[currentEdgeIndex]->figure = figure;
			break;
		}
		if (figureId == 0)
		{
			if (figure)
			{
				edges[currentEdgeIndex]->figure = figure;
				currentEdgeIndex++;
			}
			figure = new Figure();
		}
		GeoPoint* figurePt = new GeoPoint(lat, lon);
		figure->push_back(figurePt);

		/**********************************************************/
		/*test code starts from here*/
		/*if (edges[currentEdgeIndex]->id != edgeId)
		{
		cout << "currentEdgeIndex = " << currentEdgeIndex << endl;
		cout << "edgeId is no equal! " << edges[currentEdgeIndex]->id << " " << edgeId << endl;
		system("pause");
		}*/
		minLon = lon < minLon ? lon : minLon;
		maxLon = lon > maxLon ? lon : maxLon;
		minLat = lat < minLat ? lat : minLat;
		maxLat = lat > maxLat ? lat : maxLat;
		/*test code ends*/
		/**********************************************************/

	}
	figureIfs.close();

	printf(">> reading map finished\n");
	printf("lat(%lf ~ %lf), lon(%lf ~ %lf)\n", minLat, maxLat, minLon, maxLon);
	createGridIndex();
	printf(">> creating grid index finished\n");
}

void BaiduMap::setArea(MapDrawer& md)
{
	this->minLat = md.minLat;
	this->maxLat = md.maxLat;
	this->minLon = md.minLon;
	this->maxLon = md.maxLon;
}

vector<Edge*> BaiduMap::getNearEdges(double lat, double lon, double threshold) const
{
	//////////////////////////////////////////////////////////////////////////
	///����(lat, lon)��Χ����С��threshold�׵�����·��
	///[ע��]������ڴ�й¶
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
	if (row1 < 0) row1 = 0;
	if (row2 >= gridHeight) row2 = gridHeight - 1;
	if (col1 < 0) col1 = 0;
	if (col2 >= gridWidth) col2 = gridWidth - 1;
	//cout << "gridrange = " << gridSearchRange << endl;
	for (int row = row1; row <= row2; row++)
	{
		for (int col = col1; col <= col2; col++)
		{
			for (list<Edge*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
			{
				//if (grid[row][col]->size() != 0)
				//	cout << "grid count = " << grid[row][col]->size() << endl;
				if (!((*iter)->visited))
				{
					(*iter)->visited = true;
					double dist = distM_withThres(lat, lon, (*iter), threshold);
					if (dist < threshold)
						result.push_back((*iter));
					else
						fail.push_back((*iter));
				}
			}
		}
	}
	for (size_t i = 0; i < result.size(); i++)
	{
		result[i]->visited = false;
	}
	for (size_t i = 0; i < fail.size(); i++)
	{
		fail[i]->visited = false;
	}
	return result;
}

void BaiduMap::getNearEdges(double lat, double lon, double threshold, vector<Edge*>& dest)
{
	//////////////////////////////////////////////////////////////////////////
	///����(lat, lon)��Χ����С��threshold�׵�����·��
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
	if (row1 < 0) row1 = 0;
	if (row2 >= gridHeight) row2 = gridHeight - 1;
	if (col1 < 0) col1 = 0;
	if (col2 >= gridWidth) col2 = gridWidth - 1;
	//cout << "gridrange = " << gridSearchRange << endl;
	for (int row = row1; row <= row2; row++)
	{
		for (int col = col1; col <= col2; col++)
		{
			for (list<Edge*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
			{
				//if (grid[row][col]->size() != 0)
				//	cout << "grid count = " << grid[row][col]->size() << endl;
				if (!((*iter)->visited))
				{
					(*iter)->visited = true;
					double dist = distM_withThres(lat, lon, (*iter), threshold);
					if (dist < threshold)
						dest.push_back((*iter));
					else
						fail.push_back((*iter));
				}
			}
		}
	}
	for (size_t i = 0; i < dest.size(); i++)
	{
		dest[i]->visited = false;
	}
	for (size_t i = 0; i < fail.size(); i++)
	{
		fail[i]->visited = false;
	}
}

void BaiduMap::getNearEdges(double lat, double lon, size_t k, vector<Edge*>& dest)
{
	//////////////////////////////////////////////////////////////////////////
	///�ҳ���(lat, lon)���������k���ߣ����մӽ���Զ�ľ������dest��
	///���������� ���Բ�ѯ�����ڵĸ���Ϊ���ģ�gridSearchRangeΪ�����뾶����������
	///Ȼ����ÿ�������뾶����gridExtendStep�ķ���������ɢ������ֱ��candidates������Ԫ�ظ���
	///���ڵ���kֹͣ������
	//////////////////////////////////////////////////////////////////////////
	dest.clear();
	vector<pair<Edge*, double>> candidates;
	//��������
	int gridSearchRange = 3;
	int rowPt = getRowId(lat);
	int colPt = getColId(lon);
	int row1 = rowPt - gridSearchRange;
	int col1 = colPt - gridSearchRange;
	int row2 = rowPt + gridSearchRange;
	int col2 = colPt + gridSearchRange;
	if (row1 < 0) row1 = 0;
	if (row2 >= gridHeight) row2 = gridHeight - 1;
	if (col1 < 0) col1 = 0;
	if (col2 >= gridWidth) col2 = gridWidth - 1;
	for (int row = row1; row <= row2; row++)
	{
		for (int col = col1; col <= col2; col++)
		{
			for (list<Edge*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
			{
				if (!((*iter)->visited))
				{
					(*iter)->visited = true;
					double dist = distM(lat, lon, (*iter));
					candidates.push_back(make_pair((*iter), dist));
				}
			}
		}
	}

	int gridExtendStep = 1; //��չ������ÿ��������1��	
	int newRow1, newRow2, newCol1, newCol2; //��¼�µ�������Χ
	while (candidates.size() < k)
	{
		newRow1 = row1 - gridExtendStep;
		newRow2 = row2 + gridExtendStep;
		newCol1 = col1 - gridExtendStep;
		newCol2 = col2 + gridExtendStep;
		if (newRow1 < 0) newRow1 = 0;
		if (newRow2 >= gridHeight) newRow2 = gridHeight - 1;
		if (newCol1 < 0) newCol1 = 0;
		if (newCol2 >= gridWidth) newCol2 = gridWidth - 1;
		for (int row = newRow1; row <= newCol2; row++)
		{
			for (int col = newCol1; col <= newCol2; col++)
			{
				if (row >= row1 && row <= row2 && col >= col1 && col <= col2) //�Ѿ��������ľͲ�������
					continue;
				for (list<Edge*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
				{
					if (!((*iter)->visited))
					{
						(*iter)->visited = true;
						double dist = distM(lat, lon, (*iter));
						candidates.push_back(make_pair((*iter), dist));
					}
				}

			}
		}
		if (newRow1 == 0 && newRow2 == gridHeight - 1 && newCol1 == 0 && newCol2 == gridWidth - 1)//�������ȫͼ��û���㣬���ж�����
			break;
	}
	sort(candidates.begin(), candidates.end(), smallerInDist);
	for (size_t i = 0; i < k; i++)
	{
		dest.push_back(candidates[i].first);
	}

	//��ԭ���бߵ�visitFlag
	for (size_t i = 0; i < candidates.size(); i++)
	{
		candidates[i].first->visited = false;
	}
	return;

}

double BaiduMap::shortestPathLength(int ID1, int ID2, list<Edge*> &shortestPath, double dist1, double dist2, double deltaT){
	int maxNodeNum = nodes.size();
	vector<double> dist = vector<double>(maxNodeNum);
	vector<bool> flag = vector<bool>(maxNodeNum);
	for (int i = 0; i < maxNodeNum; i++) {
		dist[i] = INF;
		flag[i] = false;
	}
	dist[ID1] = 0;
	priority_queue<NODE_DIJKSTRA> Q;
	map<int, Edge*> preEdges;//��Ϊ·���յ㣬ֵΪ·��id
	NODE_DIJKSTRA tmp(ID1, 0);
	preEdges[ID1] = NULL;
	Q.push(tmp);
	while (!Q.empty()) {
		NODE_DIJKSTRA x = Q.top();
		Q.pop();
		int u = x.t;
		if (x.dist > deltaT*MAXSPEED){
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
				preEdges[i->endPointId] = edges[i->edgeId];
				Q.push(tmp);
			}
		}
	}
	double resultLength = dist[ID2];
	for (Edge* edge = preEdges[ID2]; edge != NULL; edge = preEdges[edge->startNodeId]){
		shortestPath.push_front(edge);
	}
	return resultLength;
}

double BaiduMap::distM(double lat, double lon, Edge* edge) const
{
	//////////////////////////////////////////////////////////////////////////
	///���ص�(lat, lon)����edge�ľ�ȷ����
	///���붨��Ϊ��min(�㵽��ͶӰ�ߵ�ͶӰ���룬�㵽������״���ŷ�Ͼ���)
	//////////////////////////////////////////////////////////////////////////
	double minDist = INF;
	//�����˵����
	for (Figure::iterator iter = edge->figure->begin(); iter != edge->figure->end(); iter++)
	{
		double tmpDist = GeoPoint::distM(lat, lon, (*iter)->lat, (*iter)->lon);
		if (tmpDist < minDist)
			minDist = tmpDist;
	}
	//����ͶӰ����
	Figure::iterator iter = edge->figure->begin();
	Figure::iterator nextIter = edge->figure->begin();
	nextIter++;
	while (nextIter != edge->figure->end())
	{
		//��ͶӰ
		GeoPoint* pt = new GeoPoint(lat, lon);
		if (cosAngle(pt, (*iter), (*nextIter)) <= 0 && cosAngle(pt, (*nextIter), (*iter)) <= 0)
		{
			double A = ((*nextIter)->lat - (*iter)->lat);
			double B = -((*nextIter)->lon - (*iter)->lon);
			double C = (*iter)->lat * ((*nextIter)->lon - (*iter)->lon)
				- (*iter)->lon * ((*nextIter)->lat - (*iter)->lat);
			double tmpDist = abs(A * pt->lon + B * pt->lat + C) / sqrt(A * A + B * B);
			tmpDist *= GeoPoint::geoScale;
			if (minDist > tmpDist)
				minDist = tmpDist;
		}
		iter++;
		nextIter++;
	}
	return minDist;
}

double BaiduMap::distM(double lat, double lon, Edge* edge, double& prjDist) const
{
	//////////////////////////////////////////////////////////////////////////
	///���ص�(lat, lon)����edge�ľ�ȷ����
	///���붨��Ϊ��min(�㵽��ͶӰ�ߵ�ͶӰ���룬�㵽������״���ŷ�Ͼ���)
	///�����ͶӰ�Ļ���prjDist��¼ͶӰ�㵽�켣���ľ��룬û�еĻ�prjDistΪ0
	//////////////////////////////////////////////////////////////////////////
	Figure::iterator iter = edge->figure->begin();
	Figure::iterator nextIter = edge->figure->begin();
	nextIter++;
	prjDist = 0;
	double frontSegmentDist = 0;
	double tempTotalPrjDist = 0;
	double minDist = INF;
	//�����˵����
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
	//�����һ����
	double tmpDist = GeoPoint::distM(lat, lon, (*iter)->lat, (*iter)->lon);
	if (tmpDist < minDist)
	{
		minDist = tmpDist;
		tempTotalPrjDist = frontSegmentDist;
	}
	//����ͶӰ����
	frontSegmentDist = 0;
	iter = edge->figure->begin();
	nextIter = edge->figure->begin();
	nextIter++;
	while (nextIter != edge->figure->end())
	{
		//��ͶӰ
		GeoPoint* pt = new GeoPoint(lat, lon);
		if (cosAngle(pt, (*iter), (*nextIter)) <= 0 && cosAngle(pt, (*nextIter), (*iter)) <= 0)
		{
			double A = ((*nextIter)->lat - (*iter)->lat);
			double B = -((*nextIter)->lon - (*iter)->lon);
			double C = (*iter)->lat * ((*nextIter)->lon - (*iter)->lon)
				- (*iter)->lon * ((*nextIter)->lat - (*iter)->lat);
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
		iter++;
		nextIter++;
	}
	prjDist = tempTotalPrjDist;
	return minDist;
}

//��ֲSRC�汾������(lat,lon)�㵽edge�ľ��룬��λΪ�ף�ͬʱ��¼ͶӰ�㵽edge���ľ������prjDist
double BaiduMap::distMFromTransplantFromSRC(double lat, double lon, Edge* edge, double& prjDist){
	double tmpSideLen = 0;
	double result = 1e80, tmp = 0;
	double x = -1, y = -1;
	for (Figure::iterator figIter = edge->figure->begin(); figIter != edge->figure->end(); figIter++){
		if (x != -1 && y != -1){
			double x2 = (*figIter)->lat;
			double y2 = (*figIter)->lon;
			double dist = GeoPoint::distM(x, y, lat, lon);//circleDistance(x, y, nodeX, nodeY);
			if (dist<result){
				result = dist;
				tmpSideLen = tmp;
			}
			double vecX1 = x2 - x;
			double vecY1 = y2 - y;
			double vecX2 = lat - x;
			double vecY2 = lon - y;
			double vecX3 = lat - x2;
			double vecY3 = lon - y2;
			if (vecX1*vecX2 + vecY1*vecY2>0 && -vecX1*vecX3 - vecY1*vecY3 > 0 && (vecX1 != 0 || vecY1 != 0)){
				double rate = ((lat - x2)*vecX1 + (lon - y2)*vecY1) / (-vecX1*vecX1 - vecY1*vecY1);
				double nearX = rate*x + (1 - rate)*x2, nearY = rate*y + (1 - rate)*y2;
				double dist = GeoPoint::distM(nearX, nearY, lat, lon);
				if (dist < result){
					result = dist;
					tmpSideLen = tmp + GeoPoint::distM(x, y, nearX, nearY);
				}
			}
			tmp += GeoPoint::distM(x, y, x2, y2);
		}
		x = (*figIter)->lat;
		y = (*figIter)->lon;
	}
	prjDist = tmpSideLen;
	return result;
}

int BaiduMap::hasEdge(int startNodeId, int endNodeId) const
{
	AdjNode* current = adjList[startNodeId]->next;
	while (current != NULL)
	{
		if (current->endPointId == endNodeId)
		{
			return current->edgeId;
		}
		else
			current = current->next;
	}
	return -1;
}

int BaiduMap::insertNode(double lat, double lon)
{
	//////////////////////////////////////////////////////////////////////////
	///����һ���½��(lat, lon),��ͬʱ���ڽӱ���Ҳ��Ӧ����һ���ڽӱ���,�����½���id
	//////////////////////////////////////////////////////////////////////////
	//if (!inArea(lat, lon))
	//	return -1;
	GeoPoint* pt = new GeoPoint(lat, lon);
	nodes.push_back(pt);
	AdjNode* adjNode = new AdjNode();
	adjNode->endPointId = adjList.size();
	adjNode->next = NULL;
	adjList.push_back(adjNode);
	return nodes.size() - 1;
}

int BaiduMap::insertEdge(Figure* figure, int startNodeId, int endNodeId)
{
	//////////////////////////////////////////////////////////////////////////
	///��figureΪ·�ι���һ���±߲����ͼ,��������������
	///[ע��]������û�н�����������ʱ����!
	//////////////////////////////////////////////////////////////////////////
	Edge* newEdge = new Edge();
	newEdge->figure = figure;
	newEdge->startNodeId = startNodeId;
	newEdge->endNodeId = endNodeId;
	newEdge->lengthM = calEdgeLength(figure);
	newEdge->id = edges.size();
	edges.push_back(newEdge);
	AdjNode* current = adjList[startNodeId];
	insertEdge(newEdge->id, startNodeId, endNodeId); //������ͨ��ϵ
	createGridIndexForEdge(newEdge); //������������
	return newEdge->id;
}

int BaiduMap::splitEdge(int edgeId, double lat, double lon)
{
	//////////////////////////////////////////////////////////////////////////
	///��edgeId��·��(lat, lon)���и������·,(lat, lon)��Ϊintersection
	///�и֤�ǰ�ȫ��,�޸����õ�
	//////////////////////////////////////////////////////////////////////////
	Edge* edge = edges[edgeId];
	Figure* figure = edge->figure;
	pair<int, int> result;
	bool bidirection = false;
	Edge* edgeR = NULL; //��¼˫���ʱ�ķ����Ӧ·��
	//�ҵ��и��
	Figure* subFigure1 = new Figure(); //��¼�и���ǰ���·��
	Figure* subFigure2 = new Figure(); //��¼�и��ĺ���·��
	Figure::iterator iter = figure->begin();
	Figure::iterator nextIter = figure->begin();
	nextIter++;
	int newNodeId;
	while (nextIter != edge->figure->end())
	{
		GeoPoint* pt = (*iter);
		GeoPoint* nextPt = (*nextIter);
		subFigure1->push_back(pt);
		//��ͶӰ
		double A = (nextPt->lat - pt->lat);
		double B = -(nextPt->lon - pt->lon);
		double C = pt->lat * (nextPt->lon - pt->lon)
			- pt->lon * (nextPt->lat - pt->lat);
		if (abs(A * lon + B * lat + C) < eps)
		{
			newNodeId = insertNode(lat, lon);
			subFigure1->push_back(nodes[newNodeId]);
			subFigure2->push_back(nodes[newNodeId]);
			//������·�Ƿ�Ϊ˫��·
			//��������R�Ĵ���reverse
			AdjNode* currentAdjNode = adjList[edge->endNodeId]->next;
			bidirection = false;
			while (currentAdjNode != NULL && bidirection == false)
			{
				if (currentAdjNode->endPointId == edge->startNodeId)
				{
					edgeR = edges[currentAdjNode->edgeId];
					Figure::iterator iterR = edgeR->figure->begin();
					Figure::iterator nextIterR = edgeR->figure->begin();
					nextIterR++;
					while (nextIterR != edgeR->figure->end())
					{
						GeoPoint* ptR = (*iterR);
						GeoPoint* nextPtR = (*nextIterR);
						if (abs(ptR->lat - nextPt->lat) < eps && abs(ptR->lon - nextPt->lon) < eps
							&& abs(nextPtR->lat - pt->lat) < eps && abs(nextPtR->lon - pt->lon) < eps)
						{
							bidirection = true;
							break;
						}
						iterR++;
						nextIterR++;
					}
				}
				currentAdjNode = currentAdjNode->next;
			}
			iter++;
			break;
		}
		iter++;
		nextIter++;
	}
	if (nextIter == figure->end()) //�и�㲻��·���򱨴��˳�
	{
		cout << "error: split point is not on the edge" << endl;
		system("pause");
		exit(0);
	}
	//������ѹ��subFigure2
	while (iter != figure->end())
	{
		subFigure2->push_back(*iter);
		iter++;
	}
	//���±߼���,�޸����ӹ�ϵ
	//��subFigure2��Ϊ�±߼����ͼ
	Edge* edge2 = edges[insertEdge(subFigure2, newNodeId, edge->endNodeId)];
	//��subFigure1���ԭ����edge
	delete edge->figure;
	edge->figure = subFigure1;
	edge->lengthM = calEdgeLength(subFigure1);
	edge->endNodeId = newNodeId;
	//�޸�ǰ��ε���ͨ��ϵ
	AdjNode* current = adjList[edge->startNodeId]->next;
	while (current->edgeId != edge->id)
	{
		current = current->next;
	}
	current->endPointId = newNodeId;
	//����˫������
	if (bidirection)
	{
		Figure* subFigure1R = new Figure();
		Figure* subFigure2R = new Figure();
		for (Figure::iterator iter = subFigure1->begin(); iter != subFigure1->end(); iter++)
		{
			subFigure1R->push_front(*iter);
		}
		for (Figure::iterator iter = subFigure2->begin(); iter != subFigure2->end(); iter++)
		{
			subFigure2R->push_front(*iter);
		}
		//��subFigure2R���edgeR
		delete edgeR->figure;
		edgeR->figure = subFigure2R;
		edgeR->lengthM = calEdgeLength(subFigure2R);
		//���´���edge2R
		insertEdge(subFigure2R, edge2->endNodeId, edge2->startNodeId);
	}
	return newNodeId;
}

void BaiduMap::delEdge(int edgeId)
{
	//��ע�⡿�ᷢ���ڴ�й¶
	edges[edgeId] = NULL;
	//TODO �ڽӱ�����ɾ��._.
	//���������
}

Color randomColor();

void BaiduMap::getMinMaxLatLon(string nodeFilePath)
{
	ifstream nodeIfs(nodeFilePath);
	if (!nodeIfs)
	{
		cout << "open " + nodeFilePath + " error!\n";
		system("pause");
		exit(0);
	}
	minLon = 999;
	maxLon = -999;
	minLat = 999;
	maxLat = -999;
	while (nodeIfs)
	{
		double lat, lon;
		int nodeId;
		nodeIfs >> nodeId >> lat >> lon;
		if (nodeIfs.fail())
			break;
		if (lon < minLon) minLon = lon;
		if (lon > maxLon) maxLon = lon;
		if (lat < minLat) minLat = lat;
		if (lat > maxLat) maxLat = lat;
	}
	printf("minLat:%lf, maxLat:%lf, minLon:%lf, maxLon:%lf\n", minLat, maxLat, minLon, maxLon);
	nodeIfs.close();
}

void BaiduMap::drawMap(Color color, MapDrawer& md)
{
	for (size_t i = 0; i < edges.size(); i++)
	{
		if (edges[i] == NULL)
			continue;

		Figure::iterator ptIter = edges[i]->figure->begin(), nextPtIter = ptIter;
		nextPtIter++;
		Color rndColor = randomColor();
		while (1)
		{
			if (nextPtIter == edges[i]->figure->end())
				break;
			md.drawLine(color, (*ptIter)->lat, (*ptIter)->lon, (*nextPtIter)->lat, (*nextPtIter)->lon);
			//printf("(%lf, %lf) -> (%lf, %lf)\n", (*ptIter)->lat, (*ptIter)->lon, (*nextPtIter)->lat, (*nextPtIter)->lon);
			//	system("pause");
			md.drawBigPoint(Gdiplus::Color::Black, (*ptIter)->lat, (*ptIter)->lon);
			//md.drawBigPoint(Gdiplus::Color::Black, (*nextPtIter)->lat, (*nextPtIter)->lon);
			ptIter++;
			nextPtIter++;
		}
	}
}

void BaiduMap::drawGridLine(Gdiplus::Color color, MapDrawer& md)
{
	//////////////////////////////////////////////////////////////////////////
	///��ͼƬ�ϻ��������� 
	//////////////////////////////////////////////////////////////////////////
	//����͸����
	ARGB argb = Color::MakeARGB(90, color.GetR(), color.GetG(), color.GetB());
	color.SetValue(argb);
	double delta = 0.0000001;
	for (int i = 0; i < gridHeight; i++)
	{
		double lat = minLat + gridSizeDeg * i;
		md.drawLine(color, lat, minLon + delta, lat, maxLon - delta);
	}
	for (int i = 0; i < gridWidth; i++)
	{
		double lon = minLon + gridSizeDeg * i;
		md.drawLine(color, minLat + delta, lon, maxLat - delta, lon);
	}
}
//////////////////////////////////////////////////////////////////////////
///private part
//////////////////////////////////////////////////////////////////////////
double BaiduMap::distM_withThres(double lat, double lon, Edge* edge, double threshold) const
{
	//////////////////////////////////////////////////////////////////////////
	///���ص�(lat, lon)����edge�ľ����Ͻ� ��ע�⡿�������ڼ��㾫ȷ���룡
	///���붨��Ϊ��min(�㵽��ͶӰ�ߵ�ͶӰ���룬�㵽������״���ŷ�Ͼ���)
	///��������Ͻ�ʱ�����Ѿ�����threshold(��λ��)��ֱ�ӷ���
	//////////////////////////////////////////////////////////////////////////
	double minDist = INF;
	if (edge == NULL)
	{
		cout << "edge = NULL";
		system("pause");
	}
	if (edge->figure == NULL)
	{
		cout << "edge->figure = NULL";
		system("pause");
	}
	//�����˵����
	for (Figure::iterator iter = edge->figure->begin(); iter != edge->figure->end(); iter++)
	{
		if (*iter == NULL)
		{
			cout << "*iter = NULL";
			system("pause");
		}
		if (!inArea(lat, lon) || !inArea((*iter)->lat, (*iter)->lon))
		{
			cout << "not in area";
			printf("(lat,lon) = (%lf,%lf), iter = (%lf, %lf)\n", lat, lon, (*iter)->lat, (*iter)->lon);
		}
		double tmpDist = GeoPoint::distM(lat, lon, (*iter)->lat, (*iter)->lon);
		if (tmpDist < threshold)
			return tmpDist;
		if (tmpDist < minDist)
			minDist = tmpDist;
	}
	//����ͶӰ����
	Figure::iterator iter = edge->figure->begin();
	Figure::iterator nextIter = edge->figure->begin();
	nextIter++;
	while (nextIter != edge->figure->end())
	{
		//��ͶӰ
		GeoPoint* pt = new GeoPoint(lat, lon);
		if (cosAngle(pt, (*iter), (*nextIter)) <= 0 && cosAngle(pt, (*nextIter), (*iter)) <= 0)
		{
			double A = ((*nextIter)->lat - (*iter)->lat);
			double B = -((*nextIter)->lon - (*iter)->lon);
			double C = (*iter)->lat * ((*nextIter)->lon - (*iter)->lon)
				- (*iter)->lon * ((*nextIter)->lat - (*iter)->lat);
			double tmpDist = abs(A * pt->lon + B * pt->lat + C) / sqrt(A * A + B * B);
			tmpDist *= GeoPoint::geoScale;
			if (tmpDist < threshold)
				return tmpDist;
			if (minDist > tmpDist)
				minDist = tmpDist;
		}
		iter++;
		nextIter++;
	}
	return minDist;
}

double BaiduMap::calEdgeLength(Figure* figure) const
{
	//////////////////////////////////////////////////////////////////////////
	///����·�εĳ��ȣ���λΪm
	//////////////////////////////////////////////////////////////////////////
	double lengthM = 0;
	Figure::iterator ptIter = figure->begin(), nextPtIter = ptIter;
	nextPtIter++;
	while (1)
	{
		if (nextPtIter == figure->end())
			break;
		lengthM += GeoPoint::distM((*ptIter)->lat, (*ptIter)->lon, (*nextPtIter)->lat, (*nextPtIter)->lon);
		ptIter++;
		nextPtIter++;
	}
	return lengthM;
}

bool BaiduMap::inArea(double lat, double lon) const
{
	return (lat > minLat && lat < maxLat && lon > minLon && lon < maxLon);
}

bool BaiduMap::inArea(int nodeId) const
{
	return (nodes[nodeId] != NULL);
}

void BaiduMap::test()
{
	int* flag = new int[nodes.size()];
	for (size_t i = 0; i < nodes.size(); i++)
	{
		flag[i] = -1;
	}
	for (size_t i = 0; i < adjList.size(); i++)
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

void BaiduMap::createGridIndex()
{
	//////////////////////////////////////////////////////////////////////////
	///��ȫͼ������������
	//////////////////////////////////////////////////////////////////////////
	//initialization
	gridHeight = int((maxLat - minLat) / (maxLon - minLon) * double(gridWidth)) + 1;
	gridSizeDeg = (maxLon - minLon) / double(gridWidth);
	grid = new list<Edge*>* *[gridHeight];
	for (int i = 0; i < gridHeight; i++)
		grid[i] = new list<Edge*>*[gridWidth];
	for (int i = 0; i < gridHeight; i++)
	{
		for (int j = 0; j < gridWidth; j++)
		{
			grid[i][j] = new list<Edge*>();
		}
	}
	printf("Map index gridWidth = %d, gridHeight = %d\n", gridWidth, gridHeight);
	cout << "gridSize = " << gridSizeDeg * GeoPoint::geoScale << "m" << endl;
	for (vector<Edge*>::iterator edgeIter = edges.begin(); edgeIter != edges.end(); edgeIter++)
	{
		createGridIndexForEdge((*edgeIter));
	}
}

void BaiduMap::insertEdgeIntoGrid(Edge* edge, int row, int col)
{
	//////////////////////////////////////////////////////////////////////////
	///��·��edge����grid[row][col]������������Ѿ�����������
	///�ĺ���һ���ڶ�ĳ��edge��������ʱ����,���Լ������grid�����һ��һ����edge
	//////////////////////////////////////////////////////////////////////////
	if (row >= gridHeight || row < 0 || col >= gridWidth || col < 0)
		return;
	if (grid[row][col]->size() > 0 && grid[row][col]->back() == edge)
		return;
	else
		grid[row][col]->push_back(edge);
}

void BaiduMap::createGridIndexForSegment(Edge *edge, GeoPoint* fromPT, GeoPoint* toPt)
{
	//////////////////////////////////////////////////////////////////////////
	///��edge·�е�fromPt->toPt�β����������������������񶼼�����ָ�룬����������ཻ���ȹ�С�򲻼�������
	//////////////////////////////////////////////////////////////////////////
	if (edge == NULL){ return; }
	GeoPoint* pt1 = fromPT;
	GeoPoint* pt2 = toPt;
	double x1 = pt1->lon - minLon;
	double y1 = pt1->lat - minLat;
	double x2 = pt2->lon - minLon;
	double y2 = pt2->lat - minLat;
	int row1 = y1 / gridSizeDeg;
	int row2 = y2 / gridSizeDeg;
	int col1 = x1 / gridSizeDeg;
	int col2 = x2 / gridSizeDeg;
	if (row1 >= gridHeight || row1 < 0 || col1 >= gridWidth || col1 < 0 ||
		row2 >= gridHeight || row2 < 0 || col2 >= gridWidth || col2 < 0)
	{
		cout << "************test**************" << endl;
		cout << "row1 = " << row1 << " col1 = " << col1 << endl;
		cout << "row2 = " << row2 << " col2 = " << col2 << endl;
		printf("pt1(%.8lf,%.8lf)\n", pt1->lat, pt1->lon);
		printf("pt2(%.8lf,%.8lf)\n", pt2->lat, pt2->lon);
		printf("min(%.8lf,%.8lf), max(%.8lf,%.8lf)\n", minLat, minLon, maxLat, maxLon);
		cout << "edgeId = " << edge->id << endl;
		cout << "from node " << edge->startNodeId << " to node " << edge->endNodeId << endl;
		cout << "inarea = " << inArea(pt2->lat, pt2->lon) << endl;
		cout << "maxRow = " << gridHeight - 1 << " maxCol = " << gridWidth - 1 << endl;
		//system("pause");
		//TODO����һ��û����ϸ����ܲ�����ôд
		/*if (row1 >= gridHeight)	row1 = gridHeight;
		if (row2 >= gridHeight)	row2 = gridHeight;
		if (col1 >= gridWidth)	col1 = gridWidth;
		if (col2 >= gridWidth)	col2 = gridWidth;
		if (row1 < 0)	row1 = 0;
		if (row2 < 0)	row2 = 0;
		if (col1 < 0)	col1 = 0;
		if (col2 < 0)	col2 = 0;*/
	}
	double A = y2 - y1;
	double B = -(x2 - x1);
	double C = -B * y1 - A * x1;
	int i;
	//pt1,pt2����һ��cell��
	if (row1 == row2 && col1 == col2)
	{
		insertEdgeIntoGrid(edge, row1, col1);
		return;
	}
	//ֻ��Խ�������
	if (row1 == row2)
	{
		//ͷ
		double headDist = ((min(col1, col2) + 1) * gridSizeDeg - min(x1, x2)) / gridSizeDeg;
		if (headDist / gridSizeDeg > strictThreshold)
			insertEdgeIntoGrid(edge, row1, min(col1, col2));
		//�м�
		for (i = min(col1, col2) + 1; i < max(col1, col2); i++)
		{
			insertEdgeIntoGrid(edge, row1, i);
		}
		//β
		double tailDist = (max(x1, x2) - max(col1, col2) * gridSizeDeg) / gridSizeDeg;
		if (tailDist / gridSizeDeg > strictThreshold)
			insertEdgeIntoGrid(edge, row1, max(col1, col2));
		return;
	}
	//ֻ��Խ�������
	if (col1 == col2)
	{
		//ͷ
		double headDist = ((min(row1, row2) + 1) * gridSizeDeg - min(y1, y2)) / gridSizeDeg;
		if (headDist / gridSizeDeg > strictThreshold)
			insertEdgeIntoGrid(edge, min(row1, row2), col1);
		//�м�
		for (i = min(row1, row2) + 1; i < max(row1, row2); i++)
		{
			insertEdgeIntoGrid(edge, i, col1);
		}
		//β
		double tailDist = (max(y1, y2) - max(row1, row2) * gridSizeDeg) / gridSizeDeg;
		if (tailDist / gridSizeDeg > strictThreshold)
			insertEdgeIntoGrid(edge, max(row1, row2), col1);
		return;
	}
	//б��Խ
	simplePoint pts[1000];
	int n_pts = 0;
	for (i = min(row1, row2) + 1; i <= max(row1, row2); i++)
	{
		pts[n_pts++] = std::make_pair((-C - B*i*gridSizeDeg) / A, i*gridSizeDeg);
	}
	for (i = min(col1, col2) + 1; i <= max(col1, col2); i++)
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

	//ͷ
	double headDist = sqrt((xL - pts[0].first)*(xL - pts[0].first) + (yL - pts[0].second)*(yL - pts[0].second));
	if (headDist / gridSizeDeg > strictThreshold)
	{
		insertEdgeIntoGrid(edge, (int)(yL / gridSizeDeg), (int)(xL / gridSizeDeg));
	}
	//�м�
	for (i = 0; i < n_pts - 1; i++)
	{
		double dist = sqrt((pts[i].first - pts[i + 1].first)*(pts[i].first - pts[i + 1].first) + (pts[i].second - pts[i + 1].second)*(pts[i].second - pts[i + 1].second));
		if (dist / gridSizeDeg > strictThreshold)
			//insertEdgeIntoGrid(edge, getRowId(pts[i], pts[i + 1]), getColId(pts[i], pts[i + 1]));
		{
			//��1e-9��Ϊ�˽��double�ľ������,����ԭ��rowӦ����13��,��Ϊ�����������12.99999999,ȡ������12
			int pts_i_row = (int)(pts[i].second / gridSizeDeg + 1e-9);
			int pts_i_col = (int)(pts[i].first / gridSizeDeg + 1e-9);
			int pts_i_plus_1_row = (int)(pts[i + 1].second / gridSizeDeg + 1e-9);
			int pts_i_plus_1_col = (int)(pts[i + 1].first / gridSizeDeg + 1e-9);
			int row = min(pts_i_row, pts_i_plus_1_row);
			int col = min(pts_i_col, pts_i_plus_1_col);
			insertEdgeIntoGrid(edge, row, col);
		}
	}
	//β
	double tailDist = sqrt((xR - pts[n_pts - 1].first)*(xR - pts[n_pts - 1].first) + (yR - pts[n_pts - 1].second)*(yR - pts[n_pts - 1].second));
	if (tailDist / gridSizeDeg > strictThreshold)
	{
		insertEdgeIntoGrid(edge, (int)(yR / gridSizeDeg), (int)(xR / gridSizeDeg));
	}
	return;
}

void BaiduMap::createGridIndexForEdge(Edge *edge)
{
	if (edge == NULL)
		return;
	Figure::iterator ptIter = edge->figure->begin();
	Figure::iterator nextPtIter = edge->figure->begin(); nextPtIter++;
	while (nextPtIter != edge->figure->end())
	{
		createGridIndexForSegment(edge, *ptIter, *nextPtIter);
		ptIter++;
		nextPtIter++;
	}
}

void BaiduMap::insertEdge(int edgeId, int startNodeId, int endNodeId)
{
	//////////////////////////////////////////////////////////////////////////
	///���ڽӱ�adjList�в���һ���ߵ���ͨ��ϵ�����ι���ͼʱʹ�ã�˽�а汾���������ⲿ����
	///TODO: ���ܻ�������@Line1047��while
	//////////////////////////////////////////////////////////////////////////
	/*if (startNodeId == -1)
	{
	system("pause");
	}
	if (startNodeId >= adjList.size())
	{
	cout << "startNodeId: " << startNodeId;
	cout << " adjList: " << adjList.size() << endl;
	}*/
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

int BaiduMap::getRowId(double lat) const
{
	return (int)((lat - minLat) / gridSizeDeg);
}

int BaiduMap::getColId(double lon) const
{
	return (int)((lon - minLon) / gridSizeDeg);
}

double BaiduMap::cosAngle(GeoPoint* pt1, GeoPoint* pt2, GeoPoint* pt3) const
{
	double v1x = pt2->lon - pt1->lon;
	double v1y = pt2->lat - pt1->lat;
	double v2x = pt3->lon - pt2->lon;
	double v2y = pt3->lat - pt2->lat;
	return (v1x * v2x + v1y * v2y) / sqrt((v1x * v1x + v1y * v1y)*(v2x * v2x + v2y * v2y));
}

void BaiduMap::split(const string& src, const string& separator, vector<string>& dest)
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

void BaiduMap::split(const string& src, const char& separator, vector<string>& dest)
{
	int index = 0, start = 0;
	while (index != src.size())
	{
		if (src[index] == separator)
		{
			string substring = src.substr(start, index - start);
			dest.push_back(substring);
			while (src[index + 1] == separator)
			{
				dest.push_back("");
				index++;
			}
			index++;
			start = index;
		}
		else
			index++;
	}
	//the last token
	string substring = src.substr(start);
	dest.push_back(substring);

}

bool smallerInX(simplePoint& pt1, simplePoint& pt2)
{
	//////////////////////////////////////////////////////////////////////////
	///��ֱ�������񽻵�ͳһ����x�������������
	//////////////////////////////////////////////////////////////////////////
	return pt1.first < pt2.first;
}

bool smallerInDist(pair<Edge*, double>& c1, pair<Edge*, double>& c2)
{
	//////////////////////////////////////////////////////////////////////////
	///void getNearEdges(double lat, double lon, int k, vector<Edge*>& dest)������ʹ�õ��ıȽϺ���
	//////////////////////////////////////////////////////////////////////////
	return c1.second < c2.second;
}