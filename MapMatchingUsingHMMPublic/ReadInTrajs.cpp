#include "ReadInTrajs.h"

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
		if (flag == -1)
			break;
		GeoPoint* pt = new GeoPoint(lat, lon, time);
		traj->push_back(pt);
	}
	fclose(fpIn);
	return traj;
}
void scanTrajFolder(string folderDir, string inputDirestory, list<Traj*> &trajList, vector<string> &outputFileNames)
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
	_finddata_t fileInfo;
	long lf;
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		return;
	}
	else {
		do {
			string inputFileName = fileInfo.name;
			trajList.push_back(readOneTrajectory(folderDir + inputDirestory + "\\" + inputFileName));
			string outputFileName = inputFileName.substr(6, inputFileName.size() - 10);
			outputFileName = "output_" + outputFileName + ".txt";
			outputFileNames.push_back(outputFileName);
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		return;
	}
}
void readResultFiles(string folderDir, vector<string> &outputFileNames, list<MatchedTraj> &resultList){
	string outputDirectory = "output";
	for each (string outputFileName in outputFileNames)
	{
		ifstream fin(folderDir + outputDirectory + "\\" + outputFileName);
		MatchedTraj traj = MatchedTraj();
		int time, edgeId;
		double confidence;
		char useless;
		while (fin >> time){
			fin >> useless >> edgeId >> useless >> confidence;
			traj.push_back(edgeId);
		}
		fin.close();
		resultList.push_back(traj);
	}
}
