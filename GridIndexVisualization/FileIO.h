#pragma once
#include <direct.h>
#include <io.h>
#include "GeoPoint.h"
#include "Map.h"
typedef vector<int> MatchedTraj;//地图匹配结果类型

//读入给定路径的轨迹文件中的一条轨迹
extern Traj* readOneTrajectory(string &filePath);
//读入文件夹中所有轨迹文件，保存轨迹并生成对应的输出文件名
extern void scanTrajFolder(string folderDir,string inputDirestory, list<Traj*> &trajList, vector<string> &outputFileNames);
//读入指定路径和文件名集合中所有轨迹匹配结果文件，保存在resultList中
extern void inputMatchedEdges(string fileName, MatchedTraj &result);
//输出一条轨迹的地图匹配结果
extern void outputMatchedEdges(string fileName, Traj* traj, list<Edge*> &resultList);
//输出网格中的轨迹点匹配路段频数统计
extern void outputGridCellBias(string &fileName, map<pair<int, int>, map<Edge*, int>> &biasSet);
//读入网格中的轨迹点匹配路段频数统计
extern void readGridCellBias(string &fileName, map<pair<int, int>, map<Edge*, int>> &biasSet,Map &routeNetwork);
//读入给定路径的一个地图匹配结果文件
extern list<int> ReadOneResultFile(string filePath);
//读入所有匹配结果文件和相应的答案文件
extern void scanTrajFolderAndAnswerFolder(string folderDir, string outputDirectory, string answerDirectory, vector<string> &outputFileNames, list<list<int>> &resultList, list<list<int>> &answerList);