#include "Map.h"

bool smallerInX(simplePoint& pt1, simplePoint& pt2);
bool smallerInDist(pair<Edge*, double>& c1, pair<Edge*, double>& c2);

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
	bool continueFlag = false;
	while (getline(geometryIfs, strLine))
	{
		if (geometryIfs.fail())
			break;
		std::vector<std::string> substrs;
		//split(strLine, "^", substrs);
		/*singapore ver*/
		/*split(strLine, '^', substrs);
		int edgeId = atoi(substrs[0].c_str());
		double startLat = atof(substrs[3].c_str());
		double startLon = atof(substrs[4].c_str());
		double endLat = atof(substrs[substrs.size() - 2].c_str());
		double endLon = atof(substrs[substrs.size() - 1].c_str());
		if (!inArea(startLat, startLon) || !inArea(endLat, endLon))
		{
		printf("start(%lf,%lf), end(%lf,%lf)\n", startLat, startLon, endLat, endLon);
		system("pause");
		edges.push_back(NULL);
		count++;
		continue;
		}
		Figure* figure = new Figure();
		for (int i = 3; i < substrs.size() - 1; i += 2)
		{
		double lat, lon;
		lat = atof(substrs[i].c_str());
		lon = atof(substrs[i + 1].c_str());
		if (inArea(lat, lon))
		{
		GeoPoint* pt = new GeoPoint(lat, lon);
		figure->push_back(pt);
		}
		else
		{
		continueFlag = true;
		edges.push_back(NULL);
		count++;
		break;
		}

		}*/
		/*washington ver*/
		split(strLine, '^', substrs);
		int edgeId = atoi(substrs[0].c_str());
		double startLat = atof(substrs[4].c_str());
		double startLon = atof(substrs[5].c_str());
		double endLat = atof(substrs[substrs.size() - 2].c_str());
		double endLon = atof(substrs[substrs.size() - 1].c_str());
		if (!inArea(startLat, startLon) || !inArea(endLat, endLon))
		{
			//printf("start(%lf,%lf), end(%lf,%lf)\n", startLat, startLon, endLat, endLon);
			//system("pause");
			edges.push_back(NULL);
			count++;
			continue;
		}
		Figure* figure = new Figure();
		for (size_t i = 4; i < substrs.size() - 1; i += 2)
		{
			double lat, lon;
			lat = atof(substrs[i].c_str());
			lon = atof(substrs[i + 1].c_str());
			if (inArea(lat, lon))
			{
				GeoPoint* pt = new GeoPoint(lat, lon);
				figure->push_back(pt);
			}
			else
			{
				continueFlag = true;
				edges.push_back(NULL);
				count++;
				break;
			}

		}
		if (continueFlag)
		{
			delete figure;
			continueFlag = false;
			continue;
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
	for (size_t i = 0; i < edges.size(); i++)
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
		if (edgeIfs.fail())
			break;
		if (inArea(startNodeId) && inArea(endNodeId) && edges[edgeId] != NULL)
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

//返回所有距离(lat, lon)点严格小于threshold米的所有Edge*
vector<Edge*> Map::getNearEdges(double lat, double lon, double threshold) const
{
	//////////////////////////////////////////////////////////////////////////
	///返回(lat, lon)周围距离小于threshold米的所有路段
	///[注意]会产生内存泄露
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
	for (int row = row1; row <= row2; row++)
	{
		for (int col = col1; col <= col2; col++)
		{
			for (list<Edge*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
			{
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

void Map::getNearEdges(double lat, double lon, size_t k, vector<Edge*>& dest)
{
	//////////////////////////////////////////////////////////////////////////
	///找出离(lat, lon)距离最近的k个边，按照从近到远的距离存入dest中
	///搜索方法： 先以查询点所在的格子为中心，gridSearchRange为搜索半径进行搜索。
	///然后以每次搜索半径增加gridExtendStep的幅度向外扩散搜索，直到candidates集合中元素个数
	///大于等于k停止搜索。
	//////////////////////////////////////////////////////////////////////////
	dest.clear();
	vector<pair<Edge*, double>> candidates;
	//初步搜索
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

	int gridExtendStep = 1; //扩展步进，每次向外扩1格	
	int newRow1, newRow2, newCol1, newCol2; //记录新的搜索范围
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
				if (row >= row1 && row <= row2 && col >= col1 && col <= col2) //已经搜索过的就不用搜了
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
		if (newRow1 == 0 && newRow2 == gridHeight - 1 && newCol1 == 0 && newCol2 == gridWidth - 1)//如果搜索全图还没满足，就中断搜索
			break;
	}
	sort(candidates.begin(), candidates.end(), smallerInDist);
	for (size_t i = 0; i < k; i++)
	{
		dest.push_back(candidates[i].first);
	}

	//还原所有边的visitFlag
	for (size_t i = 0; i < candidates.size(); i++)
	{
		candidates[i].first->visited = false;
	}
	return;

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
	double currentShortestDist = INF;
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

//找出距离(lat, lon)点最近的k条路段
vector<Edge*> Map::getKNearEdges(double lat, double lon, size_t k){
	const int rowPt = getRowId(lat);
	const int colPt = getColId(lon);
	int range = 0;
	int maxSearchRange = max(gridHeight, gridWidth);
	vector<pair<Edge*, double>> canadidateEdges = vector<pair<Edge*, double>>();
	set<int> visitedEdgeIdSet = set<int>();
	bool foundKEdges = false;
	bool noNearerEdge = false;
	while ((!foundKEdges || !noNearerEdge) && range <= max(gridHeight, gridWidth)){
		bool hasEdgeAdded = false;
		for (int row = rowPt - range; row <= rowPt + range; row++){
			if (row >= 0 && row < gridHeight){
				if (row == rowPt - range || row == rowPt + range){
					for (int col = colPt - range; col <= colPt + range; col++){
						if (col >= 0 && col < gridWidth){
							hasEdgeAdded = getEdgesInAGridCell(lat, lon, row, col, canadidateEdges, visitedEdgeIdSet) || hasEdgeAdded;
						}
						else{
							continue;
						}
					}
				}
				else{
					int col = colPt - range;
					if (col >= 0 && col < gridWidth){
						hasEdgeAdded = getEdgesInAGridCell(lat, lon, row, col, canadidateEdges, visitedEdgeIdSet) || hasEdgeAdded;
					}
					col = colPt + range;
					if (col >= 0 && col < gridWidth){
						hasEdgeAdded = getEdgesInAGridCell(lat, lon, row, col, canadidateEdges, visitedEdgeIdSet) || hasEdgeAdded;
					}
				}
			}
			else
			{
				continue;
			}
		}
		if (visitedEdgeIdSet.size() >= k&&hasEdgeAdded){
			foundKEdges = true;
			sort(canadidateEdges.begin(), canadidateEdges.end(), [&](pair<Edge*, double> &e1, pair<Edge*, double> &e2){
				return e1.second < e2.second;
			});
			canadidateEdges.resize(k);
			visitedEdgeIdSet.clear();
			for each (pair<Edge*, double> var in canadidateEdges)
			{
				visitedEdgeIdSet.insert(var.first->id);
			}
			double currentLongestDist = canadidateEdges.back().second;
			maxSearchRange = int(currentLongestDist / (gridSizeDeg * GeoPoint::geoScale)) + 1;
		}
		if (range >= maxSearchRange) {
			noNearerEdge = true;
			break;
		}
		range++;
	}
	vector<Edge*> resultEdges = vector<Edge*>();
	for each (pair<Edge*, double> var in canadidateEdges)
	{
		resultEdges.push_back(var.first);
	}
	return resultEdges;
}

//返回(lat,lon)点到edge的距离，单位为米
double Map::distM(double lat, double lon, Edge* edge) const
{
	//////////////////////////////////////////////////////////////////////////
	///返回点(lat, lon)到边edge的精确距离
	///距离定义为：min(点到可投影边的投影距离，点到所有形状点的欧氏距离)
	//////////////////////////////////////////////////////////////////////////
	double minDist = INF;
	//遍历端点距离
	for (Figure::iterator iter = edge->figure->begin(); iter != edge->figure->end(); iter++)
	{
		double tmpDist = GeoPoint::distM(lat, lon, (*iter)->lat, (*iter)->lon);
		if (tmpDist < minDist)
			minDist = tmpDist;
	}
	//遍历投影距离
	Figure::iterator iter = edge->figure->begin();
	Figure::iterator nextIter = edge->figure->begin();
	nextIter++;
	GeoPoint* pt = new GeoPoint(lat, lon);
	while (nextIter != edge->figure->end())
	{
		//有投影
		if (cosAngle(pt, (*iter), (*nextIter)) <= 0 && cosAngle(pt, (*nextIter), (*iter)) <= 0)
		{
			double A = ((*nextIter)->lat - (*iter)->lat);
			double B = -((*nextIter)->lon - (*iter)->lon);
			double C = (*iter)->lat * ((*nextIter)->lon - (*iter)->lon) - (*iter)->lon * ((*nextIter)->lat - (*iter)->lat);
			double tmpDist = abs(A * pt->lon + B * pt->lat + C) / sqrt(A * A + B * B);
			tmpDist *= GeoPoint::geoScale;
			if (minDist > tmpDist){
				minDist = tmpDist;
			}
		}
		iter++;
		nextIter++;
	}
	delete pt;
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
	double minDist = INF;
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
	GeoPoint* pt = new GeoPoint(lat, lon);
	while (nextIter != edge->figure->end())
	{
		//有投影		
		if (cosAngle(pt, (*iter), (*nextIter)) <= 0 && cosAngle(pt, (*nextIter), (*iter)) <= 0)
		{
			double A = ((*nextIter)->lat - (*iter)->lat);
			double B = -((*nextIter)->lon - (*iter)->lon);
			double C = (*iter)->lat * ((*nextIter)->lon - (*iter)->lon) - (*iter)->lon * ((*nextIter)->lat - (*iter)->lat);
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
	delete pt;
	prjDist = tempTotalPrjDist;
	return minDist;
}

//移植SRC版本：返回(lat,lon)点到edge的距离，单位为米；同时记录投影点到edge起点的距离存入prjDist
double Map::distMFromTransplantFromSRC(double lat, double lon, Edge* edge, double& prjDist){
	double tmpSideLen = 0;
	double result = 1e80, tmp = 0;
	double x = -1, y = -1;
	for (Figure::iterator figIter = edge->figure->begin(); figIter != edge->figure->end(); figIter++){
		if (x != -1 && y != -1){
			double x2 = (*figIter)->lat;
			double y2 = (*figIter)->lon;
			double dist = GeoPoint::distM(x, y, lat, lon);//circleDistance(x, y, nodeX, nodeY);
			if (dist < result){
				result = dist;
				tmpSideLen = tmp;
			}
			double vecX1 = x2 - x;
			double vecY1 = y2 - y;
			double vecX2 = lat - x;
			double vecY2 = lon - y;
			double vecX3 = lat - x2;
			double vecY3 = lon - y2;
			if (vecX1*vecX2 + vecY1*vecY2 > 0 && -vecX1*vecX3 - vecY1*vecY3 > 0 && (vecX1 != 0 || vecY1 != 0)){
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

//返回(lat,lon)点所在的网格索引号
pair<int, int> Map::findGridCellIndex(double lat, double lon){

	return make_pair(getRowId(lat), getColId(lon));
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
		else
			current = current->next;
	}
	return -1;
}

//在当前图中插入点
int Map::insertNode(double lat, double lon)
{
	//////////////////////////////////////////////////////////////////////////
	///插入一个新结点(lat, lon),并同时在邻接表中也对应插入一个邻接表结点,返回新结点的id
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

//在当前图中插入边
int Map::insertEdge(Figure* figure, int startNodeId, int endNodeId)
{
	//////////////////////////////////////////////////////////////////////////
	///以figure为路形构造一条新边插入地图,并插入网格索引
	///[注意]不可在没有建立网格索引时调用!
	//////////////////////////////////////////////////////////////////////////
	Edge* newEdge = new Edge();
	newEdge->figure = figure;
	newEdge->startNodeId = startNodeId;
	newEdge->endNodeId = endNodeId;
	newEdge->lengthM = calEdgeLength(figure);
	newEdge->id = edges.size();
	edges.push_back(newEdge);
	AdjNode* current = adjList[startNodeId];
	insertEdge(newEdge->id, startNodeId, endNodeId); //加入连通关系
	createGridIndexForEdge(newEdge); //加入网格索引
	return newEdge->id;
}

int Map::splitEdge(int edgeId, double lat, double lon)
{
	//////////////////////////////////////////////////////////////////////////
	///将edgeId号路在(lat, lon)点切割成两段路,(lat, lon)作为intersection
	///切割保证是安全的,无副作用的
	//////////////////////////////////////////////////////////////////////////
	Edge* edge = edges[edgeId];
	Figure* figure = edge->figure;
	pair<int, int> result;
	bool bidirection = false;
	Edge* edgeR = NULL; //记录双向道时的反向对应路段
	//找到切割点
	Figure* subFigure1 = new Figure(); //记录切割后的前半段路段
	Figure* subFigure2 = new Figure(); //记录切割后的后半段路段
	Figure::iterator iter = figure->begin();
	Figure::iterator nextIter = figure->begin();
	nextIter++;
	int newNodeId;
	while (nextIter != edge->figure->end())
	{
		GeoPoint* pt = (*iter);
		GeoPoint* nextPt = (*nextIter);
		subFigure1->push_back(pt);
		//有投影
		double A = (nextPt->lat - pt->lat);
		double B = -(nextPt->lon - pt->lon);
		double C = pt->lat * (nextPt->lon - pt->lon)
			- pt->lon * (nextPt->lat - pt->lat);
		if (abs(A * lon + B * lat + C) < eps)
		{
			newNodeId = insertNode(lat, lon);
			subFigure1->push_back(nodes[newNodeId]);
			subFigure2->push_back(nodes[newNodeId]);
			//分析该路是否为双向路
			//变量名带R的代表reverse
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
	if (nextIter == figure->end()) //切割点不在路上则报错退出
	{
		cout << "error: split point is not on the edge" << endl;
		system("pause");
		exit(0);
	}
	//将后半段压入subFigure2
	while (iter != figure->end())
	{
		subFigure2->push_back(*iter);
		iter++;
	}
	//将新边加入,修改连接关系
	//将subFigure2作为新边加入地图
	Edge* edge2 = edges[insertEdge(subFigure2, newNodeId, edge->endNodeId)];
	//将subFigure1替代原来的edge
	delete edge->figure;
	edge->figure = subFigure1;
	edge->lengthM = calEdgeLength(subFigure1);
	edge->endNodeId = newNodeId;
	//修改前半段的连通关系
	AdjNode* current = adjList[edge->startNodeId]->next;
	while (current->edgeId != edge->id)
	{
		current = current->next;
	}
	current->endPointId = newNodeId;
	//处理双向道情况
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
		//将subFigure2R替代edgeR
		delete edgeR->figure;
		edgeR->figure = subFigure2R;
		edgeR->lengthM = calEdgeLength(subFigure2R);
		//重新创建edge2R
		insertEdge(subFigure2R, edge2->endNodeId, edge2->startNodeId);
	}
	return newNodeId;
}

void Map::delEdge(int edgeId)
{
	//【注意】会发生内存泄露
	edges[edgeId] = NULL;
	//TODO 邻接表忘记删了._.
	//这个不能用
}

void Map::getMinMaxLatLon(string nodeFilePath)
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

/*
A路段起点到B路段起点的最小路网距离
参数：
ID1：A路段起点
ID2：B路段起点
dist1：基于隐马尔科夫模型地图匹配算法中轨迹点到B路段起点的距离，默认值为0
dist2：基于隐马尔科夫模型地图匹配算法中轨迹点到A路段起点的距离，默认值为0
deltaT：基于隐马尔科夫模型地图匹配算法中两轨迹点的时间差，默认为INF
shortestPath：保存组成最短路径的路段的Edge*
*/
double Map::shortestPathLength(int ID1, int ID2, list<Edge*> &shortestPath, double dist1, double dist2, double deltaT){
	int maxNodeNum = nodes.size();
	vector<double> dist = vector<double>(maxNodeNum);
	vector<bool> flag = vector<bool>(maxNodeNum);
	for (int i = 0; i < maxNodeNum; i++) {
		dist[i] = INF;
		flag[i] = false;
	}
	dist[ID1] = 0;
	priority_queue<NODE_DIJKSTRA> Q;
	map<int, Edge*> preEdges;//键为路段终点，值为路段id
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

//给定行号row和列号row，返回相应单元格中所有路段保存在resultEdges中，另外把相应的EdgeId保存在visitedEdgeIdSet中；如果resultEdges集合有改变，则返回true，反之返回false
bool Map::getEdgesInAGridCell(double lat, double lon, int row, int col, vector<pair<Edge*, double>> &resultEdges, set<int> &visitedEdgeIdSet){
	bool result = false;
	for (list<Edge*>::iterator edgeIter = grid[row][col]->begin(); edgeIter != grid[row][col]->end(); edgeIter++)
	{
		if (visitedEdgeIdSet.count((*edgeIter)->id) == 0){
			double dist = this->distM(lat, lon, (*edgeIter));
			resultEdges.push_back(make_pair((*edgeIter), dist));
			visitedEdgeIdSet.insert((*edgeIter)->id);
			result = true;
		}
	}
	return result;
}

double Map::distM_withThres(double lat, double lon, Edge* edge, double threshold) const
{
	//////////////////////////////////////////////////////////////////////////
	///返回点(lat, lon)到边edge的距离上界 【注意】不可用于计算精确距离！
	///距离定义为：min(点到可投影边的投影距离，点到所有形状点的欧氏距离)
	///如果更新上界时发现已经低于threshold(单位米)则直接返回
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
	//遍历端点距离
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
	//遍历投影距离
	Figure::iterator iter = edge->figure->begin();
	Figure::iterator nextIter = edge->figure->begin();
	nextIter++;
	GeoPoint* pt = new GeoPoint(lat, lon);
	while (nextIter != edge->figure->end())
	{
		//有投影
		if (cosAngle(pt, (*iter), (*nextIter)) <= 0 && cosAngle(pt, (*nextIter), (*iter)) <= 0)
		{
			double A = ((*nextIter)->lat - (*iter)->lat);
			double B = -((*nextIter)->lon - (*iter)->lon);
			double C = (*iter)->lat * ((*nextIter)->lon - (*iter)->lon) - (*iter)->lon * ((*nextIter)->lat - (*iter)->lat);
			double tmpDist = abs(A * pt->lon + B * pt->lat + C) / sqrt(A * A + B * B);
			tmpDist *= GeoPoint::geoScale;
			if (tmpDist < threshold){
				return tmpDist;
			}
			if (minDist > tmpDist){
				minDist = tmpDist;
			}
		}
		iter++;
		nextIter++;
	}
	delete pt;
	return minDist;
}

//计算路段的长度，单位为m
double Map::calEdgeLength(Figure* figure) const
{
	double lengthM = 0;
	Figure::iterator ptIter = figure->begin(), nextPtIter = ptIter;
	nextPtIter++;
	while (1)
	{
		if (nextPtIter == figure->end()){ break; }
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

//对全图建立网格索引
void Map::createGridIndex()
{
	//initialization
	gridHeight = int((maxLat - minLat) / (maxLon - minLon) * double(gridWidth)) + 1;
	gridSizeDeg = (maxLon - minLon) / double(gridWidth);
	grid = new list<Edge*>* *[gridHeight];
	for (int i = 0; i < gridHeight; i++){
		grid[i] = new list<Edge*>*[gridWidth];
	}
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

//将路段edge加入grid[row][col]中索引，如果已经加入过则不添加
void Map::insertEdgeIntoGrid(Edge* edge, int row, int col)
{
	//////////////////////////////////////////////////////////////////////////
	///该函数一定在对某条edge建立索引时调用,所以加入过的grid中最后一个一定是edge
	//////////////////////////////////////////////////////////////////////////
	if (row >= gridHeight || row < 0 || col >= gridWidth || col < 0)
		return;
	if (grid[row][col]->size() > 0 && grid[row][col]->back() == edge)
		return;
	else
		grid[row][col]->push_back(edge);
}

//对路段edge中的fromPt->toPt段插入网格索引，经过的网格都加入其指针，如果与网格相交长度过小则不加入网格
void Map::createGridIndexForSegment(Edge *edge, GeoPoint* fromPT, GeoPoint* toPt)
{
	if (edge == NULL){ return; }
	GeoPoint* pt1 = fromPT;
	GeoPoint* pt2 = toPt;
	double x1 = pt1->lon - minLon;
	double y1 = pt1->lat - minLat;
	double x2 = pt2->lon - minLon;
	double y2 = pt2->lat - minLat;
	int row1 = static_cast<int>(y1 / gridSizeDeg);
	int row2 = static_cast<int>(y2 / gridSizeDeg);
	int col1 = static_cast<int>(x1 / gridSizeDeg);
	int col2 = static_cast<int>(x2 / gridSizeDeg);
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
		//TODO：这一坨没有仔细想过能不能这么写
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
		if (headDist / gridSizeDeg > strictThreshold)
			insertEdgeIntoGrid(edge, row1, min(col1, col2));
		//中间
		for (i = min(col1, col2) + 1; i < max(col1, col2); i++)
		{
			insertEdgeIntoGrid(edge, row1, i);
		}
		//尾
		double tailDist = (max(x1, x2) - max(col1, col2) * gridSizeDeg) / gridSizeDeg;
		if (tailDist / gridSizeDeg > strictThreshold)
			insertEdgeIntoGrid(edge, row1, max(col1, col2));
		return;
	}
	//只穿越纵向格子
	if (col1 == col2)
	{
		//头
		double headDist = ((min(row1, row2) + 1) * gridSizeDeg - min(y1, y2)) / gridSizeDeg;
		if (headDist / gridSizeDeg > strictThreshold)
			insertEdgeIntoGrid(edge, min(row1, row2), col1);
		//中间
		for (i = min(row1, row2) + 1; i < max(row1, row2); i++)
		{
			insertEdgeIntoGrid(edge, i, col1);
		}
		//尾
		double tailDist = (max(y1, y2) - max(row1, row2) * gridSizeDeg) / gridSizeDeg;
		if (tailDist / gridSizeDeg > strictThreshold)
			insertEdgeIntoGrid(edge, max(row1, row2), col1);
		return;
	}
	//斜向穿越
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

	//头
	double headDist = sqrt((xL - pts[0].first)*(xL - pts[0].first) + (yL - pts[0].second)*(yL - pts[0].second));
	if (headDist / gridSizeDeg > strictThreshold)
	{
		insertEdgeIntoGrid(edge, (int)(yL / gridSizeDeg), (int)(xL / gridSizeDeg));
	}
	//中间
	for (i = 0; i < n_pts - 1; i++)
	{
		double dist = sqrt((pts[i].first - pts[i + 1].first)*(pts[i].first - pts[i + 1].first) + (pts[i].second - pts[i + 1].second)*(pts[i].second - pts[i + 1].second));
		if (dist / gridSizeDeg > strictThreshold)
			//insertEdgeIntoGrid(edge, getRowId(pts[i], pts[i + 1]), getColId(pts[i], pts[i + 1]));
		{
			//加1e-9是为了解决double的精度误差,比如原本row应该是13的,因为精度误差而算成12.99999999,取整后变成12
			int pts_i_row = (int)(pts[i].second / gridSizeDeg + 1e-9);
			int pts_i_col = (int)(pts[i].first / gridSizeDeg + 1e-9);
			int pts_i_plus_1_row = (int)(pts[i + 1].second / gridSizeDeg + 1e-9);
			int pts_i_plus_1_col = (int)(pts[i + 1].first / gridSizeDeg + 1e-9);
			int row = min(pts_i_row, pts_i_plus_1_row);
			int col = min(pts_i_col, pts_i_plus_1_col);
			insertEdgeIntoGrid(edge, row, col);
		}
	}
	//尾
	double tailDist = sqrt((xR - pts[n_pts - 1].first)*(xR - pts[n_pts - 1].first) + (yR - pts[n_pts - 1].second)*(yR - pts[n_pts - 1].second));
	if (tailDist / gridSizeDeg > strictThreshold)
	{
		insertEdgeIntoGrid(edge, (int)(yR / gridSizeDeg), (int)(xR / gridSizeDeg));
	}
	return;
}

void Map::createGridIndexForEdge(Edge *edge)
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

//向邻接表adjList中插入一条边的连通关系，初次构建图时使用，私有版本，不允许外部调用
void Map::insertEdge(int edgeId, int startNodeId, int endNodeId)
{
	//////////////////////////////////////////////////////////////////////////
	///TODO: 可能会有问题@while
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

void Map::split(const string& src, const char& separator, vector<string>& dest)
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

//将直线与网格交点统一按照x轴递增方向排列
bool smallerInX(simplePoint& pt1, simplePoint& pt2)
{
	return pt1.first < pt2.first;
}

//函数void getNearEdges(double lat, double lon, int k, vector<Edge*>& dest)中使用到的比较函数
bool smallerInDist(pair<Edge*, double>& c1, pair<Edge*, double>& c2)
{
	return c1.second < c2.second;
}