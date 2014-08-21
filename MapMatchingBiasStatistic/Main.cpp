#include <iostream>
#include <map>
#include "FileIO.h"
#include "Map.h"
#include "MapMatching.h"
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\GISCUP2012_Data\\";
string inputDirectory = "input_60";//����Ĺ켣�ļ���Ҫ���ԡ�input_����ͷ
string outputDirectory = "output_60";//�����ƥ�����ļ������ԡ�output_����ͷ
string gridCellBiasFileName = "biasStatistic.txt";
Map routeNetwork = Map(rootFilePath, 1000);

vector<string> outputFileNames;//ƥ�����ļ�������
list<Traj*> trajList;//�켣����
map<pair<int, int>, map<Edge*, int>> biasSet;

void biasStatistic(Traj* traj, list<Edge*> result){
	int trajPointIndex = 0;
	Traj::iterator trajIter = traj->begin();
	for each (Edge* edge in result)
	{
		GeoPoint* trajPoint = *trajIter;
		trajIter++;
		if (edge != NULL){
			pair<int, int> gridCellIndex = routeNetwork.findGridCellIndex(trajPoint->lat, trajPoint->lon);
			if (biasSet.find(gridCellIndex) == biasSet.end()){
				biasSet[gridCellIndex] = map<Edge*, int>();
			}
			if (biasSet[gridCellIndex].find(edge) == biasSet[gridCellIndex].end()){
				biasSet[gridCellIndex][edge] = 0;
			}
			biasSet[gridCellIndex][edge]++;
		}
		else{
			continue;
		}
	}
}

void main(){
	scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
	int trajIndex = 0;
	cout << "��ʼ��ͼƥ�䣡" << endl;
	for (list<Traj*>::iterator trajIter = trajList.begin(); trajIter != trajList.end(); trajIter++){
		list<Edge*> resultList = MapMatching(*(*trajIter));
		//outputMatchedEdges(rootFilePath + outputDirectory + "\\" + outputFileNames[trajIndex], *trajIter, resultList);
		//cout << "��" << trajIndex << "���켣ƥ����ϣ�" << endl;
		biasStatistic(*trajIter, resultList);
		trajIndex++;
	}
	outputGridCellBias(gridCellBiasFileName, biasSet);
	system("pause");
}