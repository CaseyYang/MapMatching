#pragma once
#include <direct.h>
#include <io.h>
#include "GeoPoint.h"
#include "Map.h"

//�������·���Ĺ켣�ļ��е�һ���켣
extern Traj* readOneTrajectory(string &filePath);
//�����ļ��������й켣�ļ�������켣�����ɶ�Ӧ������ļ���
extern void scanTrajFolder(string folderDir, list<Traj*> &trajList, vector<string> &outputFileNames);