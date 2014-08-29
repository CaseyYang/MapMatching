/*
 * Last Updated at [2014/4/21 10:01] by wuhao
 */
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include "GeoPoint.h"
using namespace std;
#define INF 999999999
#define INVALID_ROADID -1
typedef list<GeoPoint*> Traj;

class TrajReader
{
public:
	TrajReader();
	TrajReader(string filePath); //构造函数时就打开轨迹文件 
	void open(string filePath); //打开轨迹文件，轨迹文件路径为filePath
	void readTrajs(vector<Traj*>& dest, int count = INF); //读入count条轨迹放入dest(dest会先清空)，默认为全部读入，并关闭文件
	void readTrajs(list<Traj*>& dest, int count = INF); //读入count条轨迹放入dest(dest会先清空)，默认为全部读入。并关闭文件
	static void outputTrajs(list<Traj*>& trajs, string filePath, int count = INF); //将trajs内count条轨迹按照标准格式输出至filePath

private:
	//设定区域，在区域外的轨迹不读
	//double minLat;
	//double maxLat;
	//double minLon;
	//double maxLon;
	ifstream trajIfs;
};


//////////////////////////////////////////////////////////////////////////
///public part
//////////////////////////////////////////////////////////////////////////
TrajReader::TrajReader(string filePath)
{
	open(filePath);
}

void TrajReader::open(string filePath)
{
	trajIfs.open(filePath);
	if (!trajIfs)
	{
		cout << "open " + filePath + " error!\n";
		system("pause");
		exit(0);
	}
}

void TrajReader::readTrajs(vector<Traj*>& dest, int count /* = INF */)
{
	//////////////////////////////////////////////////////////////////////////
	///格式(每一行):time lat lon mmRoadId
	///轨迹结束：-1 单独占一行
	///轨迹长度为1的会丢弃
	//////////////////////////////////////////////////////////////////////////
	dest.clear();
	cout << ">> start reading trajs" << endl;
	bool isStart = true;
	int time, mmRoadId;
	double lat, lon;
	Traj* tmpTraj = NULL;
	int currentCount = 0;
	while (trajIfs)
	{
		if (currentCount == count)
		{
			break;
		}
		int time;
		trajIfs >> time;
		if (trajIfs.fail())
		{
			break;
		}
		if (time == -1)
		{
			isStart = true;
			if (tmpTraj != NULL && tmpTraj->size() > 1)
			{
				dest.push_back(tmpTraj);
				currentCount++;
			}
			continue;
		}
		else
		{
			trajIfs >> lat >> lon >> mmRoadId;
			GeoPoint* pt = new GeoPoint(lat, lon, time);
			if (isStart)
			{
				tmpTraj = new Traj();
				tmpTraj->push_back(pt);
				isStart = false;
			}
			else
			{
				tmpTraj->push_back(pt);
			}
		}
	}
	cout << ">> reading trajs finished" << endl;
	cout << dest.size() << "trajs in all" << endl;
	trajIfs.close();
}

void TrajReader::readTrajs(list<Traj*>& dest, int count /* = INF */)
{
	//////////////////////////////////////////////////////////////////////////
	///格式(每一行):time lat lon mmRoadId
	///轨迹结束：-1 单独占一行
	///轨迹长度为1的会丢弃
	//////////////////////////////////////////////////////////////////////////
	dest.clear();
	cout << ">> start reading trajs" << endl;
	bool isStart = true;
	int time, mmRoadId;
	double lat, lon;
	Traj* tmpTraj = NULL;
	int currentCount = 0;
	while (trajIfs)
	{
		if (currentCount == count)
		{
			break;
		}
		if (currentCount % 1000 == 0 && currentCount > 0)
		{
			printf("read %d trajs\n", currentCount++);
		}
		int time;
		trajIfs >> time;
		if (trajIfs.fail())
		{
			break;
		}
		if (time == -1)
		{
			isStart = true;
			if (tmpTraj != NULL && tmpTraj->size() > 1)
			{
				dest.push_back(tmpTraj);
				currentCount++;
			}
			continue;
		}
		else
		{
			trajIfs >> lat >> lon >> mmRoadId;
			GeoPoint* pt = new GeoPoint(lat, lon, time);
			if (isStart)
			{
				tmpTraj = new Traj();
				tmpTraj->push_back(pt);
				isStart = false;
			}
			else
			{
				tmpTraj->push_back(pt);
			}
		}
	}
	cout << ">> reading trajs finished" << endl;
	cout << dest.size() << " trajs in all" << endl;
	trajIfs.close();
}

//void TrajReader::outputTrajs(list<Traj*>& trajs, string filePath, int count /* = INF */)
//{
//	ofstream ofs(filePath);
//	if (!ofs)
//	{
//		cout << "open " << filePath << " error!" << endl;
//		system("pause");
//	}
//	ofs << fixed << showpoint << setprecision(8);
//	int currentCount = 0;
//	for (list<Traj*>::iterator trajIter = trajs.begin(); trajIter != trajs.end(); trajIter++)
//	{
//		if (currentCount == count)
//		{
//			break;
//		}
//		for (Traj::iterator ptIter = (*trajIter)->begin(); ptIter != (*trajIter)->end(); ptIter++)
//		{
//			ofs << (*ptIter)->time << " " << (*ptIter)->lat << " " << (*ptIter)->lon << " " << (*ptIter)->mmRoadId << endl;
//		}
//		ofs << -1 << endl;
//		currentCount++;
//	}
//	cout << ">> " << currentCount << " trajs have been output to " << filePath << endl;
//	ofs.close();
//}