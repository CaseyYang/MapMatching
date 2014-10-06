#include<iostream>
#include <direct.h>
#include <io.h>
#include "TrajReader.h"
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\新加坡数据\\";
string mergedTrajFilePath = "D:\\MapMatchingProject\\Data\\新加坡数据\\15days\\wy_MMTrajs.txt";//合并起来的轨迹文件路径
string sepereatedFilesDirectory = "15days\\15days_separated_high_quality_input";//轨迹文件集合所在文件夹路径
list<Traj*> trajList = list<Traj*>();//轨迹集合

string ToString(int i){
	stringstream ss;
	ss << i;
	return ss.str();
}

//读入给定路径的轨迹文件中的一条轨迹
Traj* readOneTrajectory(string &filePath)
{
	FILE *fpIn;
	fopen_s(&fpIn, filePath.c_str(), "r");
	double lat, lon;
	int time;
	Traj* traj = new Traj();
	while (!feof(fpIn))
	{
		int flag = fscanf_s(fpIn, "%d,%lf,%lf", &time, &lat, &lon);
		//防止末行读入两遍
		if (flag == -1)
			break;
		GeoPoint* pt = new GeoPoint(lat, lon, time);
		traj->push_back(pt);
	}
	fclose(fpIn);
	return traj;
}

//读入文件夹中所有轨迹文件，保存轨迹并生成对应的输出文件名
void scanTrajFolder(string folderDir, string inputDirestory, list<Traj*> &trajList)
{
	/*文件目录结构为
	* folderDir
	* |-input
	*   |-input_000011.txt ...
	* |-output
	*   |-output_000011.txt ...
	*/
	string completeInputFilesPath = folderDir + inputDirestory + "\\" + "*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//文件信息
	long lf;//文件句柄
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		cout << "文件夹" << completeInputFilesPath << "下未找到指定文件！" << endl;
		return;
	}
	else {
		do {
			string inputFileName = fileInfo.name;
			trajList.push_back(readOneTrajectory(folderDir + inputDirestory + "\\" + inputFileName));
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		cout << "所有轨迹文件读入完毕！" << endl;
		return;
	}
}

//输出一条轨迹到一个文件
void OutputOneTrajectory(string fileName, Traj* traj){
	ofstream fout(fileName);
	fout.precision(8);
	for each (auto var in *traj)
	{
		fout << var->time << "," << var->lat << "," << var->lon << endl;
	}
	fout.close();
}

//把合并轨迹文件拆为每条轨迹一个文件
//filesDirectory：轨迹文件集所在文件夹路径
//minimumLength：要保留的轨迹的最少采样点数
void ConvertMergedFileToSeparateFiles(string filesDirectory,int minimumLength){
	TrajReader trajReader(mergedTrajFilePath);
	trajReader.readTrajs(trajList);
	int trajIndex = 0;
	for each (auto var in trajList)
	{
		if (var->size() > minimumLength){
			OutputOneTrajectory(filesDirectory + "\\" + "input_" + ToString(trajIndex) + ".txt", var);
			++trajIndex;
		}
	}
}

//把每条轨迹合并到一个文件
//filesDirectory：轨迹文件集所在文件夹路径
//minimumLength：要保留的轨迹的最少采样点数
void ConvertSeparateFilesToMergedFile(string filesDirectory,int minimumLength){
	scanTrajFolder(rootFilePath, sepereatedFilesDirectory, trajList);
	ofstream fout(mergedTrajFilePath);
	for each (auto traj in trajList)
	{
		if (traj->size() > minimumLength){
			for each (auto var in *traj)
			{
				fout << var->time << " " << var->lat << " " << var->lon << " " << -1 << endl;
			}
			fout << -1 << endl;
		}
	}
	fout.close();
}

int main(){
	ConvertMergedFileToSeparateFiles(rootFilePath + sepereatedFilesDirectory,15);
	system("pause");
}