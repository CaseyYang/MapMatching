#include<iostream>
#include <direct.h>
#include <io.h>
#include "TrajReader.h"
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\�¼�������\\";
string mergedTrajFilePath = "D:\\MapMatchingProject\\Data\\�¼�������\\15days\\wy_MMTrajs.txt";//�ϲ������Ĺ켣�ļ�·��
string sepereatedFilesDirectory = "15days\\15days_separated_high_quality_input";//�켣�ļ����������ļ���·��
list<Traj*> trajList = list<Traj*>();//�켣����

string ToString(int i){
	stringstream ss;
	ss << i;
	return ss.str();
}

//�������·���Ĺ켣�ļ��е�һ���켣
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
		//��ֹĩ�ж�������
		if (flag == -1)
			break;
		GeoPoint* pt = new GeoPoint(lat, lon, time);
		traj->push_back(pt);
	}
	fclose(fpIn);
	return traj;
}

//�����ļ��������й켣�ļ�������켣�����ɶ�Ӧ������ļ���
void scanTrajFolder(string folderDir, string inputDirestory, list<Traj*> &trajList)
{
	/*�ļ�Ŀ¼�ṹΪ
	* folderDir
	* |-input
	*   |-input_000011.txt ...
	* |-output
	*   |-output_000011.txt ...
	*/
	string completeInputFilesPath = folderDir + inputDirestory + "\\" + "*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//�ļ���Ϣ
	long lf;//�ļ����
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		cout << "�ļ���" << completeInputFilesPath << "��δ�ҵ�ָ���ļ���" << endl;
		return;
	}
	else {
		do {
			string inputFileName = fileInfo.name;
			trajList.push_back(readOneTrajectory(folderDir + inputDirestory + "\\" + inputFileName));
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		cout << "���й켣�ļ�������ϣ�" << endl;
		return;
	}
}

//���һ���켣��һ���ļ�
void OutputOneTrajectory(string fileName, Traj* traj){
	ofstream fout(fileName);
	fout.precision(8);
	for each (auto var in *traj)
	{
		fout << var->time << "," << var->lat << "," << var->lon << endl;
	}
	fout.close();
}

//�Ѻϲ��켣�ļ���Ϊÿ���켣һ���ļ�
//filesDirectory���켣�ļ��������ļ���·��
//minimumLength��Ҫ�����Ĺ켣�����ٲ�������
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

//��ÿ���켣�ϲ���һ���ļ�
//filesDirectory���켣�ļ��������ļ���·��
//minimumLength��Ҫ�����Ĺ켣�����ٲ�������
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