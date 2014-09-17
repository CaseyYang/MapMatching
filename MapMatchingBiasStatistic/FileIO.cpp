#include "FileIO.h"


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
			string outputFileName = inputFileName.substr(6, inputFileName.size() - 10);
			outputFileName = "output_" + outputFileName + ".txt";
			outputFileNames.push_back(outputFileName);
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		cout << "所有轨迹文件读入完毕！" << endl;
		return;
	}
}

//读入指定路径和文件名集合中所有轨迹匹配结果文件，保存在resultList中
void inputMatchedEdges(string fileName, MatchedTraj &result){
	ifstream matchedEdgeInput(fileName);
	MatchedTraj traj = MatchedTraj();
	int time, edgeId;
	double confidence;
	char useless;
	while (matchedEdgeInput >> time){
		matchedEdgeInput >> useless >> edgeId >> useless >> confidence;
		traj.push_back(edgeId);
	}
	matchedEdgeInput.close();
	return;
}

//输出一条轨迹的地图匹配结果
void outputMatchedEdges(string fileName, Traj* traj, list<Edge*> &resultList){
	ofstream matchedEdgeOutput(fileName);
	Traj::iterator trajPointIter = traj->begin();
	for (list<Edge*>::iterator edgeIter = resultList.begin(); edgeIter != resultList.end(); edgeIter++, trajPointIter++){
		if (*edgeIter != NULL){
			int currentIndex = (*edgeIter)->id;
			matchedEdgeOutput << (*trajPointIter)->time << "," << currentIndex << ",1.0" << endl;
		}
		else{
			matchedEdgeOutput << (*trajPointIter)->time << "," << -1 << ",1.0" << endl;
		}
	}
	matchedEdgeOutput.close();
	return;
}

//输出网格中的轨迹点匹配路段频数统计
void outputGridCellBias(string &fileName, map<pair<int, int>, map<Edge*, int>> &biasSet){
	ofstream biasOutput(fileName);
	for each (auto gridCellBiasPair in biasSet){
		biasOutput << gridCellBiasPair.first.first << " " << gridCellBiasPair.first.second << " " << gridCellBiasPair.second.size() << " ";
		for each(auto edgeCountPair in gridCellBiasPair.second){
			biasOutput << edgeCountPair.first->id << " " << edgeCountPair.second << " ";
		}
		biasOutput << endl;
	}
	biasOutput.close();
}

//读入网格中的轨迹点匹配路段频数统计
void readGridCellBias(string &fileName, map<pair<int, int>, map<Edge*, int>> &biasSet, Map &routeNetwork){
	ifstream biasInput(fileName);
	string str;
	int indexX, indexY, pairCount, edgeId, edgeCount;
	while (biasInput >> indexX >> indexY >> pairCount){
		pair<int, int> gridCellIndex = make_pair(indexX, indexY);
		biasSet[gridCellIndex] = map<Edge*, int>();
		for (int i = 0; i < pairCount; i++){
			biasInput >> edgeId >> edgeCount;
			biasSet[gridCellIndex][routeNetwork.edges[edgeId]] = edgeCount;
		}
	}
	biasInput.close();
	cout << "统计文件读入完毕！" << endl;
	return;
}