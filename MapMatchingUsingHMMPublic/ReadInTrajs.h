#pragma once
#include <direct.h>
#include <io.h>
#include "GeoPoint.h"
#include "Map.h"
typedef vector<int> MatchedTraj;

extern Traj* readOneTrajectory(string &filePath);
extern void scanTrajFolder(string folderDir,string inputDirestory, list<Traj*> &trajList, vector<string> &outputFileNames);
extern void readResultFiles(string folderDir, vector<string> &outputFileNames, list<MatchedTraj> &resultList);