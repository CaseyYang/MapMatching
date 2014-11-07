#pragma once
#include <iostream>
#include <fstream>
#include <direct.h>
#include <io.h>
#include <vector>
#include "GeoPoint.h"
#include "Map.h"
using namespace std;

#define SIGMAZ 4.591689
#define N2_SIGMAZ2 -0.0237151062104234
#define SQR_2PI_SIGMAZ 0.0868835586212075
#define RANGEOFCANADIDATEEDGES 50.0
#define MINPROB 1e-150

extern Map map;
extern list<Edge*> MapMatching(list<GeoPoint*> &trajectory);