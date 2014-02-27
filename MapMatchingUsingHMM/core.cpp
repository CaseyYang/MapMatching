#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "define.h"
#include "geometry.h"
#include "index.h"

#define SIGMAZ 4.591689
//N2_SIGMAZ2 = -2 * SIGMAZ * SIGMAZ
#define N2_SIGMAZ2 -0.0237151062104234
//SQR_2PI_SIGMAZ = sqrt(2 * PI) * SIGMAZ
#define SQR_2PI_SIGMAZ 0.0868835586212075


double prob1(double dist){
	//搞死（高斯）概率；除法变乘法，看不懂悠着办
	return exp(dist * dist * N2_SIGMAZ2) * SQR_2PI_SIGMAZ;
}


//dp数组每行代表一个样本点；每个样本点最多500个候选匹配路段；每个元素记录匹配概率
//dpSize具体记录每个样本点有几个候选路段
//deEdge记录每个样本点对应的匹配路段的id号
//deLeft记录（（每个样本点在匹配路段上的）投影点）里路段起点的距离
//deLen记录候选边的长度
//dePre记录从每个元素的转移路径
void coreDP(long double dp[][CANADIDATEEDGES], int dpSize[], int dpEdge[][CANADIDATEEDGES], double dpLeft[][CANADIDATEEDGES], double dpLen[][CANADIDATEEDGES], int dpPre[][CANADIDATEEDGES]){
	double leftDist, dist, roadLen, theDP[1000];
	int i, j, k, result[1000], resNum;

	int *IndexPointer = DijkstraIndex;

	int oriGrid = inWhichGrid(nodeSequence[0], nodeSequence[1]);
	//返回周围八个格子里的所有路段，都可能是候选路段；路段id存于result数组中；resNum存结果数目
	getRelateEdgeSet(oriGrid, result, &resNum);
	//printf("res:%d\n",resNum);
	dpSize[0] = 0;

	long double midPro = -1e10;//当前最大概率
	for (i = 0; i<resNum; ++i){
		//计算点到边的最小距离；若不存在投影则得到的是点到边起点/终点的最小距离
		//所有距离都用米做单位
		dist = nodeToEdgeDistanceAndNodeSide(nodeSequence[0], nodeSequence[1], result[i], &leftDist, &roadLen);
		if (dist > 50.0){
			continue;//50m之外不是候选边，否则DP
		}
		else {
			dpEdge[0][dpSize[0]] = result[i];
			dpLeft[0][dpSize[0]] = leftDist;
			dpLen[0][dpSize[0]] = roadLen;
			//初始放射概率
			dp[0][dpSize[0]] = prob1(dist);
			if (dp[0][dpSize[0]] > midPro){
				midPro = dp[0][dpSize[0]];
			}
			++dpSize[0];
		}
	}
	int tmpCount = 0;
	for (i = 0; i < dpSize[0]; ++i){
		//如果初始放射概率过小，直接忽视
		if (dp[0][i] > 1e-150){
			//把初始放射概率过小的覆盖掉
			if (tmpCount < i){
				dpEdge[0][tmpCount] = dpEdge[0][i];
				dpLeft[0][tmpCount] = dpLeft[0][i];
				dpLen[0][tmpCount] = dpLen[0][i];
				dp[0][tmpCount] = dp[0][i];
			}
			++tmpCount;
		}
	}
	dpSize[0] = tmpCount;
	for (i = 0; i < dpSize[0]; ++i) dp[0][i] /= midPro;

	int st;
	long double maxPr = -1e10;


	int cutFlag = 0;//记录断头的位置
	long double BT = (long double)BETA_ARR[rate];
	int tTime = nowTime;//当前处理轨迹的样本点数，注意：后面循环从1开始
	for (i = 1; i < tTime; ++i) {
		dpSize[i] = 0;
		int tpN = 0, n2 = 0;
		int tmpGrid = inWhichGrid(nodeSequence[i << 1], nodeSequence[(i << 1) + 1]);

		if (tmpGrid != oriGrid){
			getRelateEdgeSet(tmpGrid, result, &resNum);
			oriGrid = tmpGrid;
		}

		for (j = 0; j < resNum; ++j) {
			dist = nodeToEdgeDistanceAndNodeSide(nodeSequence[i << 1], nodeSequence[(i << 1) + 1], result[j], &leftDist, &roadLen);
			if (dist > 50.0) {
				continue;
			}
			else{
				dpEdge[i][dpSize[i]] = result[j];
				dpLeft[i][dpSize[i]] = leftDist;
				dpLen[i][dpSize[i]] = roadLen;
				//隐马尔科夫模型中放射概率
				dp[i][dpSize[i]] = prob1(dist);
				++dpSize[i];
			}
		}

		for (j = 0; j < dpSize[i]; ++j) {
			midPro = -1.0;
			double pathDist;
			long double tmp;

			for (k = 0; k < dpSize[i - 1]; ++k) {
				//当前一个点i-1的第k个候选点和当前i的第j个候选点来自同一个路段时，两者到起点距离相减就是路网距离，减少调用distance函数的次数
				if (dpEdge[i - 1][k] == dpEdge[i][j]) {
					pathDist = fabs(dpLeft[i][j] - dpLeft[i - 1][k]);
				}
				else {
					//用dijkstra计算前后两点路网距离
					double srctt = H_GetShortestPathLength(other[dpEdge[i - 1][k]], thisSide[dpEdge[i][j]], (double)(50 * (timeSequence[i] - timeSequence[i - 1])), IndexPointer);
					//srctt计算的是前一条边终点到后一条边起点的最短路网距离，所以还要算上后一条边起点到投影点以及前一条边终点到投影点的距离
					pathDist = dpLeft[i][j] + dpLen[i - 1][k] - dpLeft[i - 1][k] + srctt;
				}
				//两样本点间的球面距离
				double driveLength = circleDistance(nodeSequence[(i << 1) - 2], nodeSequence[(i << 1) - 1], nodeSequence[(i << 1)], nodeSequence[(i << 1) + 1]);
				tmp = exp(-fabs((long double)driveLength - (long double)pathDist) / BT) / BT;

				//GIS2012CUP的优化：len是路段长度；cost是最短通过时间
				tmp *= len[dpEdge[i][j]] / cost[dpEdge[i][j]];

				if (dp[i - 1][k] * tmp > midPro){
					midPro = dp[i - 1][k] * tmp;
					dpPre[i][j] = k;
				}
			}
			dp[i][j] *= midPro;
		}

		//在一个隐马尔科夫过程后去掉那些概率小的，为的是在后一轮隐马尔科夫过程中减少候选
		if (dpSize[i] > 10) {
			tmpCount = 0;
			for (j = 0; j < dpSize[i]; ++j) {
				if (dp[i][j] > 1e-50) {
					if (tmpCount < j) {
						dpEdge[i][tmpCount] = dpEdge[i][j];
						dpLeft[i][tmpCount] = dpLeft[i][j];
						dpLen[i][tmpCount] = dpLen[i][j];
						dpPre[i][tmpCount] = dpPre[i][j];
						dp[i][tmpCount] = dp[i][j];
					}
					++tmpCount;
				}
			}
			dpSize[i] = tmpCount;
		}
		midPro = -1e10;
		for (j = 0; j<dpSize[i]; ++j) {
			if (dp[i][j]>midPro) {
				midPro = dp[i][j];
			}
		}

		//Case 2 概率断掉（断头路，路径的特殊情况） 重新开始一段
		//if (midPro < 1e-100){

		//	int ii;
		//	printf("%d--\n", dpSize[i - 1]);
		//	for (ii = 0; ii < dpSize[i - 1]; ++ii)
		//	if (maxPr < dp[i - 1][ii]){
		//		maxPr = dp[i - 1][ii];
		//		st = ii;
		//	}


		//	for (ii = i - 1; ii >= cutFlag; --ii){
		//		printf("%d %d\t", ii, st);
		//		ans[ii] = dpEdge[ii][st];
		//		st = dpPre[ii][st];
		//	}
		//	cutFlag = i;
		//	dpSize[i] = 0;


		//	tmpGrid = inWhichGrid(nodeSequence[i * 2], nodeSequence[i * 2 + 1]);
		//	if (tmpGrid != oriGrid){
		//		getRelateEdgeSet(tmpGrid, result, &resNum);
		//		oriGrid = tmpGrid;
		//	}


		//	for (j = 0; j<resNum; ++j){
		//		dist = nodeToEdgeDistanceAndNodeSide(nodeSequence[i << 1], nodeSequence[(i << 1) + 1], result[j], &leftDist, &roadLen);
		//		if (dist>50) continue;
		//		dpEdge[i][dpSize[i]] = result[j];
		//		dpLeft[i][dpSize[i]] = leftDist;
		//		dpLen[i][dpSize[i]] = roadLen;
		//		dp[i][dpSize[i]] = prob1(dist);
		//		++dpSize[i];
		//	}


		//}
		//else
		//归一化
		for (j = 0; j < dpSize[i]; ++j) {
			dp[i][j] /= midPro;
		}
	}


	//输出全局匹配路径保存至ans数组
	maxPr = -1e10;
	for (i = 0; i < dpSize[nowTime - 1]; ++i){
		if (maxPr < dp[nowTime - 1][i]){
			maxPr = dp[nowTime - 1][i];
			st = i;
		}
	}
	for (i = nowTime - 1; i >= cutFlag; --i){
		ans[i] = dpEdge[i][st];
		st = dpPre[i][st];
	}
}
