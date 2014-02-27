#pragma once
#include <direct.h>
#include <io.h>
#include "GeoPoint.h"
#include "Map.h"

//读入给定路径的轨迹文件中的一条轨迹
extern Traj* readOneTrajectory(string &filePath);
//读入文件夹中所有轨迹文件，保存轨迹并生成对应的输出文件名
extern void scanTrajFolder(string folderDir, list<Traj*> &trajList, vector<string> &outputFileNames);