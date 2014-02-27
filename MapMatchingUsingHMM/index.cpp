#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <process.h>
#include <time.h>
#include <math.h>
#include "define.h"
#include "geometry.h"
#include "index.h"

int LATNUM = 1976;														//γ�ȷֳɵ�����
int LONGNUM = 2876;														//���ȷֳɵ�����
double GRIDLAT = 0.00179815;												//��������γ�ȷ�������cellSize����ã�
double GRIDLONG = 0.002797408;											//�����������ȷ���
double CHANGELAT;
double CHANGELONG;
int cellSize = 200;//��ʼʱ��һ����Ԫ��̶�Ϊ200m*200m
int divSize;

//���ݵ�ͼ��С���������С
void resetCellSize(){
	//datamember�켣����
	int newCellSize = 200 - 20 * dataNumber;
	if (newCellSize < 75) newCellSize = 75;//��λ����

	GRIDLAT = GRIDLAT*newCellSize / cellSize;//ͨ��newCellSize���µ�������
	GRIDLONG = GRIDLONG*newCellSize / cellSize;

	LATNUM = (int)((maxLat - minLat) / GRIDLAT) + 1;
	LONGNUM = (int)((maxLong - minLong) / GRIDLONG) + 1;

	CHANGELAT = 1 / GRIDLAT;//�����ǵ�λ��γ�ȱ仯������
	CHANGELONG = 1 / GRIDLONG;

	cellSize = newCellSize;
	divSize = cellSize - 10;//��֤�ͱ߽��ཻʱ�����ڵ�Ԫ����
}

int cmp(const void *a, const void *b) {
	return *(int *)a - *(int *)b;
}

/**
 *description:�ҵ�һ��ָ����������ƥ���·��(���񣬻����һ����С��200M�뾶��԰)
 *param:double double �����꣬int* �������,int* ����
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
 *description:��ȡһ������ı߽�
 *param:int ����Id��double double ���Ͻǵ㣬double double ���½ǵ�
 *return:
 */
void gridGetBound(int gridId, double* x1, double* y1, double* x2, double* y2){
	int xN = gridId / LONGNUM, yN = gridId % LONGNUM;
	*x1 = xN*GRIDLAT + minLat; *x2 = (xN + 1)*GRIDLAT + minLat;
	*y1 = yN*GRIDLONG + minLong; *y2 = (yN + 1)*GRIDLONG + minLong;
}


static void insertLineToIndex(int edgeId, double lat1, double long1, double lat2, double long2)
{
	//div��div2�ǿ��������·���м��ݣ����еĽ϶�ķݼ�
	int div = (int)fabs((lat2 - lat1)*CHANGELAT);
	int div2 = (int)fabs((long2 - long1)*CHANGELONG);
	if (div2 > div) div = div2;
	++div;
	double x = lat1, y = long1, dx = (lat2 - lat1) / div, dy = (long2 - long1) / div;
	int centerGrid, edgeLast;
	int i;
	for (i = 0; i <= div; ++i) {
		centerGrid = inWhichGrid(x, y);//grid����Ϊһά����ṹ
		//ÿ����Ԫ�����һ�������ڽӱ�Ľṹ����¼�õ�Ԫ���������е�·��
		//ÿ��·�λ���ָ��ǰһ��·�ε�����
		//indexLast��¼���ǣ���ÿ����Ԫ����ڽӱ�����һ��·�Σ���indexOther��������ģ�����
		//indexPre��¼���ǣ�����������������indexOther������ʾ��·�Σ�����ĳ����Ԫ���е��ڽӱ��е�ǰһ��·�Σ���indexOther�еģ�����
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

