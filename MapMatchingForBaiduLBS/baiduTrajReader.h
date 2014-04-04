/* 
 * Last Updated at [2014/4/3 11:33] by wuhao
 */
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include "GeoPoint.h"
using namespace std;
#define MAX_TRAJS 999999999
typedef list<GeoPoint*> Traj;

class BaiduTrajReader
{
public:
	BaiduTrajReader();
	BaiduTrajReader(string filePath); //���캯��ʱ�ʹ򿪹켣�ļ� 
	void open(string filePath); //�򿪹켣�ļ����켣�ļ�·��ΪfilePath
	void readTrajs(vector<Traj*>& dest, int count = MAX_TRAJS); //����count���켣����dest(dest�������)��Ĭ��Ϊȫ�����룬���ر��ļ�
	void readTrajs(list<Traj*>& dest, int count = MAX_TRAJS); //����count���켣����dest(dest�������)��Ĭ��Ϊȫ�����롣���ر��ļ�

private:
	ifstream trajIfs;
	void split(const string& src, const char& separator, vector<string>& dest);
};


//////////////////////////////////////////////////////////////////////////
///public part
//////////////////////////////////////////////////////////////////////////
BaiduTrajReader::BaiduTrajReader(string filePath)
{
	open(filePath);
}

void BaiduTrajReader::open(string filePath)
{
	trajIfs.open(filePath);
	if (!trajIfs)
	{
		cout << "open " + filePath + " error!\n";
		system("pause");
		exit(0);
	}
}

void BaiduTrajReader::readTrajs(vector<Traj*>& dest, int count /* = INF */)
{
	//////////////////////////////////////////////////////////////////////////
	///��ʽ(ÿһ��):userId time0,lon0,lat0|time1,lon1,lat1|...
	//////////////////////////////////////////////////////////////////////////
	dest.clear();
	int currentCount = 0;
	bool continueFlag = false;
	while (trajIfs)
	{
		if (currentCount >= count)
			break;
		int userId;
		string strLine;
		Traj* traj = new Traj();
		trajIfs >> userId >> strLine;
		if (trajIfs.fail())
			break;
		vector<string> substrs;
		split(strLine, '|', substrs);
		for (size_t i = 0; i < substrs.size(); i++)
		{
			vector<string> subsubstrs;
			split(substrs[i], ',', subsubstrs);
			GeoPoint* pt = new GeoPoint(atof(subsubstrs[2].c_str()), atof(subsubstrs[1].c_str()),atoi(subsubstrs[0].c_str()));
			traj->push_back(pt);
		}
		dest.push_back(traj);
		currentCount++;
	}
	trajIfs.close();
}

void BaiduTrajReader::readTrajs(list<Traj*>& dest, int count /* = INF */)
{
	//////////////////////////////////////////////////////////////////////////
	///��ʽ(ÿһ��):userId time0,lon0,lat0|time1,lon1,lat1|...
	//////////////////////////////////////////////////////////////////////////
	dest.clear();
	int currentCount = 0;
	bool continueFlag = false;
	while (trajIfs)
	{
		if (currentCount >= count)
			break;
		int userId;
		string strLine;
		Traj* traj = new Traj();
		trajIfs >> userId >> strLine;
		if (trajIfs.fail())
			break;
		vector<string> substrs;
		split(strLine, '|', substrs);
		for (size_t i = 0; i < substrs.size(); i++)
		{
			vector<string> subsubstrs;
			split(substrs[i], ',', subsubstrs);
			GeoPoint* pt = new GeoPoint(atof(subsubstrs[2].c_str()), atof(subsubstrs[1].c_str()), atoi(subsubstrs[0].c_str()));
			traj->push_back(pt);
		}
		dest.push_back(traj);
		currentCount++;
	}
	trajIfs.close();
}

//////////////////////////////////////////////////////////////////////////
///private part
//////////////////////////////////////////////////////////////////////////
void BaiduTrajReader::split(const string& src, const char& separator, vector<string>& dest)
{
	int index = 0, start = 0;
	while (index != src.size())
	{
		if (src[index] == separator)
		{
			string substring = src.substr(start, index - start);
			dest.push_back(substring);
			while (src[index + 1] == separator)
			{
				dest.push_back("");
				index++;
			}
			index++;
			start = index;
		}
		else
			index++;
	}
	//the last token
	string substring = src.substr(start);
	dest.push_back(substring);
}
