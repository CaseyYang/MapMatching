#include <iostream>
#include <map>
#include <set>
#include "FileIO.h"
#include "Map.h"
#include "MapMatching.h"
#include "MapMatchingUsingBiasStatistic.h"
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\�¼�������\\";
string inputDirectory = "day7\\day7_unsplit";//����Ĺ켣�ļ���Ҫ���ԡ�input_����ͷ
string outputDirectory = "day7\\day7_output";//�����ƥ�����ļ������ԡ�output_����ͷ
string gridCellBiasFileName = "biasStatistic.txt";
Map routeNetwork = Map(rootFilePath, 2000);

vector<string> outputFileNames;//ƥ�����ļ�������
list<Traj*> trajList;//�켣����
map<pair<int, int>, map<Edge*, int>> biasSet;

void biasStatistic(Traj* traj, list<Edge*> result){
	int trajPointIndex = 0;
	Traj::iterator trajIter = traj->begin();
	set<pair<int, int>> countedGridCellSet = set<pair<int, int>>();
	for each (Edge* edge in result)
	{
		GeoPoint* trajPoint = *trajIter;
		trajIter++;
		if (edge != NULL){
			pair<int, int> gridCellIndex = routeNetwork.findGridCellIndex(trajPoint->lat, trajPoint->lon);
			if (countedGridCellSet.find(gridCellIndex) == countedGridCellSet.end()){
				if (biasSet.find(gridCellIndex) == biasSet.end()){
					biasSet[gridCellIndex] = map<Edge*, int>();
				}
				if (biasSet[gridCellIndex].find(edge) == biasSet[gridCellIndex].end()){
					biasSet[gridCellIndex][edge] = 0;
				}
				biasSet[gridCellIndex][edge]++;
				countedGridCellSet.insert(gridCellIndex);
			}
			else{
				continue;
			}
		}
		else{
			continue;
		}
	}
}

void main(){
	scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
	readGridCellBias(gridCellBiasFileName, biasSet, routeNetwork);
	int trajIndex = 0;
	cout << "��ʼ��ͼƥ�䣡" << endl;
	for (list<Traj*>::iterator trajIter = trajList.begin(); trajIter != trajList.end(); trajIter++){
		/*ƥ��·����Ϣͳ��*/
		list<Edge*> resultList = MapMatching(*(*trajIter));
		biasStatistic(*trajIter, resultList);
		/*����ƥ��·��ͳ����Ϣ���е�ͼƥ��*/
		//list<Edge*> resultList = MapMatchingUsingBiasStatistic(*(*trajIter));
		//list<Edge*> resultList = MapMatchingUsingBiasStatisticAsPriorProb(*(*trajIter));
		//outputMatchedEdges(rootFilePath + outputDirectory + "\\" + outputFileNames[trajIndex], *trajIter, resultList);
		//cout << "��" << trajIndex << "���켣ƥ����ϣ�" << endl;		
		trajIndex++;
	}
	cout << "��ͼƥ����ɣ�" << endl;
	outputGridCellBias(gridCellBiasFileName, biasSet);
	system("pause");
}