#include <iostream>
#include <map>
#include <set>
#include "FileIO.h"
#include "Map.h"
#include "MapMatching.h"
#include "MapMatchingUsingBiasStatistic.h"
#include "TrajReader.h"
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\�¼�������\\";
string inputDirectory = "day3\\day3_unsplit";//����Ĺ켣�ļ���Ҫ���ԡ�input_����ͷ
string outputDirectory = "15days\\15days_output2";//�����ƥ�����ļ������ԡ�output_����ͷ
string gridCellBiasFileName = "biasStatistic.txt";
string mergedTrajFilePath = "D:\\MapMatchingProject\\Data\\�¼�������\\15days\\wy_MMTrajs.txt";
Map routeNetwork = Map(rootFilePath, 3100);

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
	/*�����ļ������켣��ȡ����*/
	scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
	/*�����ļ������켣��ȡ����*/
	//TrajReader trajReader(mergedTrajFilePath);
	//trajReader.readTrajs(trajList);
	//trajReader.outputMatchedEdges(trajList, rootFilePath + "15days\\15days_answer");
	//trajReader.makeOutputFileNames(outputFileNames);
	readGridCellBias(gridCellBiasFileName, biasSet, routeNetwork);
	int trajIndex = 0;
	cout << "��ʼ��ͼƥ�䣡" << endl;
	for (list<Traj*>::iterator trajIter = trajList.begin(); trajIter != trajList.end(); trajIter++){
		//if (trajIndex == 1365){
			//cout << "�켣���ȣ�" << (*trajIter)->size() << endl;
			/*ƥ��·����Ϣͳ��*/
			list<Edge*> resultList = MapMatching(*(*trajIter));
			biasStatistic(*trajIter, resultList);
			/*����ƥ��·��ͳ����Ϣ���е�ͼƥ��*/
			//list<Edge*> resultList = MapMatchingUsingBiasStatistic(*(*trajIter));
			/*����ƥ��·��ͳ����Ϣ��Ϊ������ʽ��е�ͼƥ��*/
			//list<Edge*> resultList = MapMatchingUsingBiasStatisticAsPriorProb(*(*trajIter));
			//cout << "��" << tra2000jIndex << "���켣ƥ����ϣ�" << endl;
			//outputMatchedEdges(rootFilePath + outputDirectory + "\\" + outputFileNames[trajIndex], *trajIter, resultList);
			cout << "��" << trajIndex << "���켣ƥ��·�������ϣ�" << endl;
		//}
		trajIndex++;
	}
	cout << "��ͼƥ����ɣ�" << endl;
	outputGridCellBias(gridCellBiasFileName, biasSet);
	system("pause");
}