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
	//��������˹�����ʣ�������˷������������Ű�
	return exp(dist * dist * N2_SIGMAZ2) * SQR_2PI_SIGMAZ;
}


//dp����ÿ�д���һ�������㣻ÿ�����������500����ѡƥ��·�Σ�ÿ��Ԫ�ؼ�¼ƥ�����
//dpSize�����¼ÿ���������м�����ѡ·��
//deEdge��¼ÿ���������Ӧ��ƥ��·�ε�id��
//deLeft��¼����ÿ����������ƥ��·���ϵģ�ͶӰ�㣩��·�����ľ���
//deLen��¼��ѡ�ߵĳ���
//dePre��¼��ÿ��Ԫ�ص�ת��·��
void coreDP(long double dp[][CANADIDATEEDGES], int dpSize[], int dpEdge[][CANADIDATEEDGES], double dpLeft[][CANADIDATEEDGES], double dpLen[][CANADIDATEEDGES], int dpPre[][CANADIDATEEDGES]){
	double leftDist, dist, roadLen, theDP[1000];
	int i, j, k, result[1000], resNum;

	int *IndexPointer = DijkstraIndex;

	int oriGrid = inWhichGrid(nodeSequence[0], nodeSequence[1]);
	//������Χ�˸������������·�Σ��������Ǻ�ѡ·�Σ�·��id����result�����У�resNum������Ŀ
	getRelateEdgeSet(oriGrid, result, &resNum);
	//printf("res:%d\n",resNum);
	dpSize[0] = 0;

	long double midPro = -1e10;//��ǰ������
	for (i = 0; i<resNum; ++i){
		//����㵽�ߵ���С���룻��������ͶӰ��õ����ǵ㵽�����/�յ����С����
		//���о��붼��������λ
		dist = nodeToEdgeDistanceAndNodeSide(nodeSequence[0], nodeSequence[1], result[i], &leftDist, &roadLen);
		if (dist > 50.0){
			continue;//50m֮�ⲻ�Ǻ�ѡ�ߣ�����DP
		}
		else {
			dpEdge[0][dpSize[0]] = result[i];
			dpLeft[0][dpSize[0]] = leftDist;
			dpLen[0][dpSize[0]] = roadLen;
			//��ʼ�������
			dp[0][dpSize[0]] = prob1(dist);
			if (dp[0][dpSize[0]] > midPro){
				midPro = dp[0][dpSize[0]];
			}
			++dpSize[0];
		}
	}
	int tmpCount = 0;
	for (i = 0; i < dpSize[0]; ++i){
		//�����ʼ������ʹ�С��ֱ�Ӻ���
		if (dp[0][i] > 1e-150){
			//�ѳ�ʼ������ʹ�С�ĸ��ǵ�
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


	int cutFlag = 0;//��¼��ͷ��λ��
	long double BT = (long double)BETA_ARR[rate];
	int tTime = nowTime;//��ǰ����켣������������ע�⣺����ѭ����1��ʼ
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
				//������Ʒ�ģ���з������
				dp[i][dpSize[i]] = prob1(dist);
				++dpSize[i];
			}
		}

		for (j = 0; j < dpSize[i]; ++j) {
			midPro = -1.0;
			double pathDist;
			long double tmp;

			for (k = 0; k < dpSize[i - 1]; ++k) {
				//��ǰһ����i-1�ĵ�k����ѡ��͵�ǰi�ĵ�j����ѡ������ͬһ��·��ʱ�����ߵ��������������·�����룬���ٵ���distance�����Ĵ���
				if (dpEdge[i - 1][k] == dpEdge[i][j]) {
					pathDist = fabs(dpLeft[i][j] - dpLeft[i - 1][k]);
				}
				else {
					//��dijkstra����ǰ������·������
					double srctt = H_GetShortestPathLength(other[dpEdge[i - 1][k]], thisSide[dpEdge[i][j]], (double)(50 * (timeSequence[i] - timeSequence[i - 1])), IndexPointer);
					//srctt�������ǰһ�����յ㵽��һ�����������·�����룬���Ի�Ҫ���Ϻ�һ������㵽ͶӰ���Լ�ǰһ�����յ㵽ͶӰ��ľ���
					pathDist = dpLeft[i][j] + dpLen[i - 1][k] - dpLeft[i - 1][k] + srctt;
				}
				//�����������������
				double driveLength = circleDistance(nodeSequence[(i << 1) - 2], nodeSequence[(i << 1) - 1], nodeSequence[(i << 1)], nodeSequence[(i << 1) + 1]);
				tmp = exp(-fabs((long double)driveLength - (long double)pathDist) / BT) / BT;

				//GIS2012CUP���Ż���len��·�γ��ȣ�cost�����ͨ��ʱ��
				tmp *= len[dpEdge[i][j]] / cost[dpEdge[i][j]];

				if (dp[i - 1][k] * tmp > midPro){
					midPro = dp[i - 1][k] * tmp;
					dpPre[i][j] = k;
				}
			}
			dp[i][j] *= midPro;
		}

		//��һ��������Ʒ���̺�ȥ����Щ����С�ģ�Ϊ�����ں�һ��������Ʒ�����м��ٺ�ѡ
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

		//Case 2 ���ʶϵ�����ͷ·��·������������� ���¿�ʼһ��
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
		//��һ��
		for (j = 0; j < dpSize[i]; ++j) {
			dp[i][j] /= midPro;
		}
	}


	//���ȫ��ƥ��·��������ans����
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
