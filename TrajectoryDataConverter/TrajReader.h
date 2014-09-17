/*
 * Last Updated at [2014/4/21 10:01] by wuhao
 */
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
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
	TrajReader(string filePath); //���캯��ʱ�ʹ򿪹켣�ļ� 
	void open(string filePath); //�򿪹켣�ļ����켣�ļ�·��ΪfilePath
	void readTrajs(vector<Traj*>& dest, int count = INF); //����count���켣����dest(dest�������)��Ĭ��Ϊȫ�����룬���ر��ļ�
	void readTrajs(list<Traj*>& dest, int count = INF); //����count���켣����dest(dest�������)��Ĭ��Ϊȫ�����롣���ر��ļ�
	void makeOutputFileNames(vector<string> &outputFileNames);//�������й켣��������һ���ļ��У������Ҫ����������켣������ļ���
	void TrajReader::outputMatchedEdges(list<Traj*> &trajs, string directoryPath);//���ÿ���켣��Ӧ��ƥ��·�Σ�����directoryPath��󲻱��Դ���//��
	static void outputTrajs(list<Traj*>& trajs, string filePath, int count = INF); //��trajs��count���켣���ձ�׼��ʽ�����filePath

private:
	//�趨������������Ĺ켣����
	//double minLat;
	//double maxLat;
	//double minLon;
	//double maxLon;
	ifstream trajIfs;//�켣�ļ�������
	int trajectoriesNum;//����켣����
};


//���캯��ʱ�ʹ򿪹켣�ļ� 
TrajReader::TrajReader(string filePath)
{
	open(filePath);
}

//�򿪹켣�ļ����켣�ļ�·��ΪfilePath
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

//����count���켣����dest(dest�������)��Ĭ��Ϊȫ�����룬���ر��ļ�
void TrajReader::readTrajs(vector<Traj*>& dest, int count /* = INF */)
{
	//////////////////////////////////////////////////////////////////////////
	///��ʽ(ÿһ��):time lat lon mmRoadId
	///�켣������-1 ����ռһ��
	///�켣����Ϊ1�Ļᶪ��
	//////////////////////////////////////////////////////////////////////////
	dest.clear();
	cout << ">> start reading trajs" << endl;
	bool isStart = true;
	int mmRoadId, currentCount = 0;
	double lat, lon;
	Traj* tmpTraj = NULL;
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
			GeoPoint* pt = new GeoPoint(lat, lon, time, mmRoadId);
			if (isStart)
			{
				tmpTraj = new Traj();
				isStart = false;
			}
			tmpTraj->push_back(pt);
		}
	}
	cout << ">> reading trajs finished" << endl;
	cout << dest.size() << "trajs in all" << endl;
	trajectoriesNum = static_cast<int>(dest.size());
	trajIfs.close();
}

//����count���켣����dest(dest�������)��Ĭ��Ϊȫ�����롣���ر��ļ�
void TrajReader::readTrajs(list<Traj*>& dest, int count /* = INF */)
{
	//////////////////////////////////////////////////////////////////////////
	///��ʽ(ÿһ��):time lat lon mmRoadId
	///�켣������-1 ����ռһ��
	///�켣����Ϊ1�Ļᶪ��
	//////////////////////////////////////////////////////////////////////////
	dest.clear();
	cout << ">> start reading trajs" << endl;
	bool isStart = true;
	int mmRoadId, currentCount = 0;
	double lat, lon;
	Traj* tmpTraj = NULL;
	while (trajIfs)
	{
		if (currentCount == count)
		{
			break;
		}
		//if (currentCount % 1000 == 0 && currentCount > 0)
		//{
		//	printf("read %d trajs\n", currentCount++);
		//}
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
			GeoPoint* pt = new GeoPoint(lat, lon, time, mmRoadId);
			if (isStart)
			{
				tmpTraj = new Traj();
				isStart = false;
			}
			tmpTraj->push_back(pt);
		}
	}
	cout << ">> reading trajs finished" << endl;
	cout << dest.size() << " trajs in all" << endl;
	trajectoriesNum = static_cast<int>(dest.size());
	trajIfs.close();
}

//�������й켣��������һ���ļ��У������Ҫ����������켣������ļ���
void TrajReader::makeOutputFileNames(vector<string> &outputFileNames){
	string baseStr = "output_";
	for (int i = 0; i < trajectoriesNum; i++){
		stringstream ss;
		ss << baseStr << i << ".txt";
		outputFileNames.push_back(ss.str());
	}
}

//���ÿ���켣��Ӧ��ƥ��·�Σ�����directoryPath��󲻱��Դ���//��
void TrajReader::outputMatchedEdges(list<Traj*> &trajs, string directoryPath){
	int index = 0;
	for each (auto traj in trajs)
	{
		stringstream ss;
		ss << directoryPath + "\\output_" << index << ".txt";
		index++;
		ofstream outputAnswerFile = ofstream(ss.str());
		for each (auto trajPoint in *traj)
		{
			outputAnswerFile << trajPoint->time << "," << trajPoint->matchedEdgeId << ",1.0" << endl;
		}
		outputAnswerFile.close();
	}
}

//��trajs��count���켣���ձ�׼��ʽ�����filePath
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