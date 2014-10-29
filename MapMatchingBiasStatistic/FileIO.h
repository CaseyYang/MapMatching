#pragma once
#include <direct.h>
#include <io.h>
#include "GeoPoint.h"
#include "Map.h"
typedef vector<int> MatchedTraj;//��ͼƥ��������

//�������·���Ĺ켣�ļ��е�һ���켣
extern Traj* readOneTrajectory(string &filePath);
//�����ļ��������й켣�ļ�������켣�����ɶ�Ӧ������ļ���
extern void scanTrajFolder(string folderDir, string inputDirectory, list<Traj*> &trajList, vector<string> &outputFileNames);
//����ָ��·�����ļ������������й켣ƥ�����ļ���������resultList��
extern void inputMatchedEdges(string fileName, MatchedTraj &result);
//���һ���켣�ĵ�ͼƥ����
extern void outputMatchedEdges(string fileName, Traj* traj, list<Edge*> &resultList);
//��������еĹ켣��ƥ��·��Ƶ��ͳ��
extern void outputGridCellBias(string &fileName, map<pair<int, int>, map<Edge*, int>> &biasSet);
//���������еĹ켣��ƥ��·��Ƶ��ͳ��
extern void readGridCellBias(string &fileName, map<pair<int, int>, map<Edge*, int>> &biasSet,Map &routeNetwork);