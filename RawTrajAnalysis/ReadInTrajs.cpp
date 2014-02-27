#include "ReadInTrajs.h"


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
void scanTrajFolder(string folderDir, list<Traj*> &trajList, vector<string> &outputFileNames)
{
	/*文件目录结构为
	* folderDir
	* |-input
	*   |-input_000011.txt ...
	* |-output
	*   |-output_000011.txt ...
	*/
	string completeInputFilesPath = folderDir + "test_input\\" + "*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//文件信息
	long lf;//文件句柄
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		return;
	}
	else {
		do {
			string inputFileName = fileInfo.name;
			trajList.push_back(readOneTrajectory(folderDir + "test_input\\" + inputFileName));
			string outputFileName = inputFileName.substr(6, inputFileName.size() - 10);
			outputFileName = "output_" + outputFileName + ".txt";
			outputFileNames.push_back(outputFileName);
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		return;
	}
}