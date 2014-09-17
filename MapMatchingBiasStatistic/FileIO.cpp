#include "FileIO.h"


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
void scanTrajFolder(string folderDir, string inputDirestory, list<Traj*> &trajList, vector<string> &outputFileNames)
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
			string outputFileName = inputFileName.substr(6, inputFileName.size() - 10);
			outputFileName = "output_" + outputFileName + ".txt";
			outputFileNames.push_back(outputFileName);
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		cout << "���й켣�ļ�������ϣ�" << endl;
		return;
	}
}

//����ָ��·�����ļ������������й켣ƥ�����ļ���������resultList��
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

//���һ���켣�ĵ�ͼƥ����
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

//��������еĹ켣��ƥ��·��Ƶ��ͳ��
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

//���������еĹ켣��ƥ��·��Ƶ��ͳ��
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
	cout << "ͳ���ļ�������ϣ�" << endl;
	return;
}