#pragma once
#include <iostream>
#include <fstream>
#include "Map.h"
#include "PointGridIndex.h"
using namespace std;

#define N2_SIGMAZ2 -0.0237151062104234
#define SQR_2PI_SIGMAZ 0.0868835586212075
#define RANGEOFCANADIDATEEDGES 50.0 //��ѡ·��ѡȡ��Χ

extern Map routeNetwork;
extern PointGridIndex litePointGridIndex;
extern map<pair<int, int>, map<Edge*, int>> biasSet;
//ʹ�������·��ƫ�ý��е�ͼƥ��
extern list<Edge*> MapMatchingUsingBiasStatistic(list<GeoPoint*> &trajectory);
//ʹ�������·��ƫ����Ϊ������ʽ��е�ͼƥ��
extern list<Edge*> MapMatchingUsingBiasStatisticAsPriorProb(list<GeoPoint*> &trajectory);