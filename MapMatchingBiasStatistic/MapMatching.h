#pragma once
#include <iostream>
#include <fstream>
#include <direct.h>
#include <io.h>
#include <vector>
#include "GeoPoint.h"
#include "Map.h"
using namespace std;


//地图匹配所用参数
#define SIGMAZ 4.591689
#define N2_SIGMAZ2 -0.0237151062104234
#define SQR_2PI_SIGMAZ 0.0868835586212075
//地图匹配通用参数
#define RANGEOFCANADIDATEEDGES 50.0 //候选路段选取范围
#define MINPROB 1e-150 //整体概率的下限

extern Map routeNetwork;
extern list<Edge*> MapMatching(list<GeoPoint*> &trajectory);