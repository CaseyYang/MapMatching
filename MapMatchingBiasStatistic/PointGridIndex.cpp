#include "PointGridIndex.h"

//bool smallerThan(GeoPoint* pt1, GeoPoint* pt2)
//{
//	return pt1->dist < pt2->dist;
//}
//////////////////////////////////////////////////////////////////////////
///public
//////////////////////////////////////////////////////////////////////////
void PointGridIndex::createIndex(list<GeoPoint*>& pts, Area* area, int gridWidth)
{
	this->area = area;
	this->gridWidth = gridWidth;
	initialization();
	for (list<GeoPoint*>::iterator ptIter = pts.begin(); ptIter != pts.end(); ptIter++)
	{
		insertOnePt(*ptIter);
	}
}

void PointGridIndex::setGridIndexParameters(Area* area, int gridWidth){
	this->area = area;
	this->gridWidth = gridWidth;
	if (gridWidth <= 0){ return; }
	gridHeight = int((area->maxLat - area->minLat) / (area->maxLon - area->minLon) * double(gridWidth)) + 1;
	gridSizeDeg = (area->maxLon - area->minLon) / double(gridWidth);
	printf("Point index gridWidth = %d, gridHeight = %d\n", gridWidth, gridHeight);
	cout << "gridSize = " << gridSizeDeg * GeoPoint::geoScale << "m" << endl;
}

pair<int, int> PointGridIndex::getRowCol(GeoPoint* pt)
{
	//////////////////////////////////////////////////////////////////////////
	///first row, second col
	//////////////////////////////////////////////////////////////////////////
	return make_pair((int)((pt->lat - area->minLat) / gridSizeDeg), (int)((pt->lon - area->minLon) / gridSizeDeg));
}

//void PointGridIndex::drawGridLine(Gdiplus::Color color, MapDrawer& md)
//{
//	//////////////////////////////////////////////////////////////////////////
//	///在图片上画出网格线 
//	//////////////////////////////////////////////////////////////////////////
//	Gdiplus::ARGB argb = Gdiplus::Color::MakeARGB(90, color.GetR(), color.GetG(), color.GetB());
//	color.SetValue(argb);
//	double delta = 0.0000001;
//	for (int i = 0; i < gridHeight; i++)
//	{
//		double lat = area->minLat + gridSizeDeg * i;
//		md.drawLine(color, lat, area->minLon + delta, lat, area->maxLon - delta);
//	}
//	for (int i = 0; i < gridWidth; i++)
//	{
//		double lon = area->minLon + gridSizeDeg * i;
//		md.drawLine(color, area->minLat + delta, lon, area->maxLat - delta, lon);
//	}
//}

void PointGridIndex::getNearPts(GeoPoint* pt, double thresholdM, vector<GeoPoint*>& dest)
{
	dest.clear();
	//calculate search range
	int gridSearchRange = int(thresholdM / (gridSizeDeg * GeoPoint::geoScale)) + 1;
	pair<int, int> rowCol = getRowCol(pt);
	int rowPt = rowCol.first;
	int colPt = rowCol.second;
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
			for (list<GeoPoint*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
			{
				double dist = GeoPoint::distM((*iter), pt);
				if (dist < thresholdM && (*iter) != pt)
					dest.push_back((*iter));
			}
		}
	}
}

void PointGridIndex::getNearPts(GeoPoint* pt, int gridRange, vector<GeoPoint*>& dest)
{
	dest.clear();
	pair<int, int> rolCol = getRowCol(pt);
	int rowPt = rolCol.first;
	int colPt = rolCol.second;
	int row1 = rowPt - gridRange;
	int col1 = colPt - gridRange;
	int row2 = rowPt + gridRange;
	int col2 = colPt + gridRange;
	if (row1 < 0) row1 = 0;
	if (row2 >= gridHeight) row2 = gridHeight - 1;
	if (col1 < 0) col1 = 0;
	if (col2 >= gridWidth) col2 = gridWidth - 1;

	for (list<GeoPoint*>::iterator iter = grid[rowPt][colPt]->begin(); iter != grid[rowPt][colPt]->end(); iter++)
	{
		if ((*iter) != pt)
			dest.push_back((*iter));
	}
	for (int row = row1; row <= row2; row++)
	{
		for (int col = col1; col <= col2; col++)
		{
			if (row == rowPt && col == colPt)
				continue;
			for (list<GeoPoint*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
			{
				if ((*iter) != pt)
					dest.push_back((*iter));
			}
		}
	}
}

//void PointGridIndex::kNN(GeoPoint* pt, int k, double thresholdM, vector<GeoPoint*>& dest)
//{
//	//////////////////////////////////////////////////////////////////////////
//	///[ATTENTION]:只查找pt所在网格周围8个网格范围，超过的不检查
//	///如果有k个点都满足距离pt小雨thresholdM米，则随便找满足这个的k个点返回
//	///[TODO]:偷懒，要重写的
//	//////////////////////////////////////////////////////////////////////////
//	dest.clear();
//	vector<GeoPoint*> candidatePts;
//	int range = 1;
//	while (candidatePts.size() < k)
//	{
//		getNearPts(pt, range, candidatePts);
//		range++;
//	}
//	for (int i = 0; i < candidatePts.size(); i++)
//	{
//		double dist = GeoPoint::distM(pt, candidatePts[i]);
//		candidatePts[i]->dist = dist;
//		if (dist < thresholdM && dest.size() < k)
//		{
//			dest.push_back(candidatePts[i]);
//		}
//		if (dest.size() == k)
//			return;
//	}
//	dest.clear();
//	sort(candidatePts.begin(), candidatePts.end(), smallerThan);
//	for (int i = 0; i < k; i++)
//	{
//		dest.push_back(candidatePts[i]);
//	}
//}

void PointGridIndex::initialization()
{
	//////////////////////////////////////////////////////////////////////////
	///初始化中完成以下操作
	///1.根据area计算网格行数
	///2.在堆上开空间
	///[ATTENTION]：[MEM_LEAK]建立索引时不会判断grid是否是NULL，需自行delete前一个索引空间
	//////////////////////////////////////////////////////////////////////////	
	if (gridWidth <= 0){ return; }
	gridHeight = int((area->maxLat - area->minLat) / (area->maxLon - area->minLon) * double(gridWidth)) + 1;
	gridSizeDeg = (area->maxLon - area->minLon) / double(gridWidth);
	grid = new list<GeoPoint*>* *[gridHeight];
	for (int i = 0; i < gridHeight; i++)
		grid[i] = new list<GeoPoint*>*[gridWidth];
	for (int i = 0; i < gridHeight; i++)
	{
		for (int j = 0; j < gridWidth; j++)
		{
			grid[i][j] = new list<GeoPoint*>();
		}
	}
	printf("Point index gridWidth = %d, gridHeight = %d\n", gridWidth, gridHeight);
	cout << "gridSize = " << gridSizeDeg * GeoPoint::geoScale << "m" << endl;
}

void PointGridIndex::insertOnePt(GeoPoint* pt)
{
	if ((pt->lat >= area->minLat && pt->lat <= area->maxLat && pt->lon >= area->minLon && pt->lon <= area->maxLon))
	{
		pair<int, int> rolCol = getRowCol(pt);
		grid[rolCol.first][rolCol.second]->push_back(pt);
	}
}