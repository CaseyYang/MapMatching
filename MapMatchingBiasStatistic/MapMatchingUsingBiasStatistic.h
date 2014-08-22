#pragma once
#include <iostream>
#include <fstream>
#include "Map.h"
using namespace std;

extern Map routeNetwork;
extern map<pair<int, int>, map<Edge*, int>> biasSet;
extern list<Edge*> MapMatchingUsingBiasStatistic(list<GeoPoint*> &trajectory);