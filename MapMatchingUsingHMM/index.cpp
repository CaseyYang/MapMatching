#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <process.h>
#include <time.h>
#include <math.h>
#include "define.h"
#include "geometry.h"
#include "index.h"

int LATNUM = 1976;														//纬度分成的数量
int LONGNUM = 2876;														//经度分成的数量
double GRIDLAT = 0.00179815;												//网格索引纬度分量（按cellSize计算得）
double GRIDLONG = 0.002797408;											//网格索引经度分量
double CHANGELAT;
double CHANGELONG;
int cellSize = 200;//初始时，一个单元格固定为200m*200m
int divSize;

//根据地图大小设置网格大小
void resetCellSize(){
	//datamember轨迹条数
	int newCellSize = 200 - 20 * dataNumber;
	if (newCellSize < 75) newCellSize = 75;//单位是米

	GRIDLAT = GRIDLAT*newCellSize / cellSize;//通过newCellSize重新调整粒度
	GRIDLONG = GRIDLONG*newCellSize / cellSize;

	LATNUM = (int)((maxLat - minLat) / GRIDLAT) + 1;
	LONGNUM = (int)((maxLong - minLong) / GRIDLONG) + 1;

	CHANGELAT = 1 / GRIDLAT;//可能是单位经纬度变化的米数
	CHANGELONG = 1 / GRIDLONG;

	cellSize = newCellSize;
	divSize = cellSize - 10;//保证和边界相交时总落在单元格内
}

int cmp(const void *a, const void *b) {
	return *(int *)a - *(int *)b;
}

/**
 *description:找到一个指定点所可以匹配的路径(网格，还需进一步缩小到200M半径的园)
 *param:double double 点坐标，int* 结果数组,int* 个数
 *return:
 */
void getRelateEdgeSet(int centerGrid, int result[], int* resNum){
	int resNumber = 0;
	int p, q, laIndex;
	for (p = -1; p<2; ++p)
	for (q = -1; q<2; ++q){
		//int nbGrid=getNeighbor(centerGrid,p,q);
		int nbGrid = centerGrid + p * LONGNUM + q;
		if (nbGrid>-1){
			laIndex = indexLast[nbGrid];
			while (laIndex>0){
				result[resNumber++] = indexOther[laIndex];
				laIndex = indexPre[laIndex];
			}
		}
	}
	qsort(result, resNumber, 4, cmp);
	int tmpNum = 0;
	for (p = 1; p<resNumber; ++p)
	if (result[p]>result[tmpNum])
		result[++tmpNum] = result[p];
	*resNum = tmpNum + 1;
}

int inWhichGrid(double nodeLat, double nodeLong){
	return (int)((nodeLat - minLat)*CHANGELAT)*LONGNUM + (int)((nodeLong - minLong)*CHANGELONG) + 1;
}

/**
 *description:获取一个网格的边界
 *param:int 网格Id，double double 左上角点，double double 右下角点
 *return:
 */
void gridGetBound(int gridId, double* x1, double* y1, double* x2, double* y2){
	int xN = gridId / LONGNUM, yN = gridId % LONGNUM;
	*x1 = xN*GRIDLAT + minLat; *x2 = (xN + 1)*GRIDLAT + minLat;
	*y1 = yN*GRIDLONG + minLong; *y2 = (yN + 1)*GRIDLONG + minLong;
}


static void insertLineToIndex(int edgeId, double lat1, double long1, double lat2, double long2)
{
	//div和div2是看把输入的路段切几份；按切的较多的份计
	int div = (int)fabs((lat2 - lat1)*CHANGELAT);
	int div2 = (int)fabs((long2 - long1)*CHANGELONG);
	if (div2 > div) div = div2;
	++div;
	double x = lat1, y = long1, dx = (lat2 - lat1) / div, dy = (long2 - long1) / div;
	int centerGrid, edgeLast;
	int i;
	for (i = 0; i <= div; ++i) {
		centerGrid = inWhichGrid(x, y);//grid索引为一维数组结构
		//每个单元格包含一个类似邻接表的结构，记录该单元格里面所有的路段
		//每个路段还有指向前一个路段的索引
		//indexLast记录的是（（每个单元格的邻接表的最后一个路段）在indexOther数组里面的）索引
		//indexPre记录的是（（（给定索引（在indexOther里所表示的路段））在某个单元格中的邻接表中的前一个路段）在indexOther中的）索引
		edgeLast = indexLast[centerGrid];
		if (!edgeLast || indexOther[edgeLast] != edgeId) {
			++crossSize;
			indexPre[crossSize] = edgeLast;
			indexLast[centerGrid] = crossSize;
			indexOther[crossSize] = edgeId;
		}
		x += dx; y += dy;
	}
}

void createGirdIndex() {
	//_beginthread( createGirdIndex2, 0, NULL );
	int i = 0, j, dt = 1;
	//int halfEdge=edgeNumber/2;
	for (i = 0; i < edgeNumber; ++i) {
		/*if (i==247266){
				for (j = edgeStart[i]; j < edgeStart[i + 1] - 1; ++j) {
				int q= j << 1;
				}
				}*/
		for (j = edgeStart[i]; j < edgeStart[i + 1] - 1; ++j) {
			int q = j << 1;
			insertLineToIndex(i, coordNet[q], coordNet[q + 1], coordNet[q + 2], coordNet[q + 3]);
		}
	}
}

