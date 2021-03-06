#pragma once
#include <direct.h>
#include <io.h>
#include "GeoPoint.h"
#include "Map.h"
typedef vector<int> MatchedTraj;//地图匹配结果类型

//读入给定路径的轨迹文件中的一条轨迹
extern Traj* readOneTrajectory(string &filePath);
//读入文件夹中所有轨迹文件，保存轨迹并生成对应的输出文件名
void scanTrajFolder(string folderDir, string inputDirestory, list<Traj*> &trajList, vector<string> &outputFileNames);
//读入指定路径和文件名集合中所有轨迹匹配结果文件，保存在resultList中
extern void readResultFiles(string folderDir, vector<string> &outputFileNames, list<MatchedTraj> &resultList);
//输出轨迹集合至指定路径的文件中
void outputTrajsToFiles(Traj &traj, string filePath);
//输出轨迹匹配答案集合至指定路径的文件中
void outputAnswersToFiles(Traj &traj, string filePath);