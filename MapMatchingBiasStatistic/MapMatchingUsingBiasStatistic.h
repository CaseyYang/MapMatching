#pragma once
#include <iostream>
#include <fstream>
#include "Map.h"
#include "PointGridIndex.h"
using namespace std;

#define N2_SIGMAZ2 -0.0237151062104234
#define SQR_2PI_SIGMAZ 0.0868835586212075
#define RANGEOFCANADIDATEEDGES 50.0 //候选路段选取范围

extern Map routeNetwork;
extern PointGridIndex litePointGridIndex;
extern map<pair<int, int>, map<Edge*, int>> biasSet;
//使用网格的路段偏好进行地图匹配
extern list<Edge*> MapMatchingUsingBiasStatistic(list<GeoPoint*> &trajectory);
//使用网格的路段偏好作为先验概率进行地图匹配
extern list<Edge*> MapMatchingUsingBiasStatisticAsPriorProb(list<GeoPoint*> &trajectory);