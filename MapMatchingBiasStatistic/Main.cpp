#include <iostream>
#include <map>
#include <set>
#include "FileIO.h"
#include "Map.h"
#include "MapMatching.h"
#include "MapMatchingUsingBiasStatistic.h"
#include "TrajReader.h"
#include "PointGridIndex.h"
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\�¼�������\\";
string inputDirectory = "day1\\day1_splited_input";//����Ĺ켣�ļ���Ҫ���ԡ�input_����ͷ
string outputDirectory = "day1\\day1_splited_answer";//�����ƥ�����ļ������ԡ�output_����ͷ
string gridCellBiasFileName = "biasStatistic_15000.txt";
string mergedTrajFilePath = "D:\\MapMatchingProject\\Data\\�¼�������\\15days\\wy_MMTrajs.txt";
int pointIndexGranularity = 15000;

Map routeNetwork = Map(rootFilePath, 1000);
PointGridIndex pointGridIndex;//������й켣�㽨������������
vector<string> outputFileNames;//ƥ�����ļ�������
list<Traj*> trajList;//�켣����

map<pair<int, int>, map<Edge*, int>> biasSet;//��ͼ�л�������ĵ�ͼƥ��ͳ���������

//�ѹ켣����trajList�����й켣��ϲ�������trajPointList��
void makeTrajPointGridIndex(int gridWidth){
	list<GeoPoint*> trajPointList = list<GeoPoint*>();//���й켣�㼯��
	for each (Traj* traj in trajList)
	{
		for each (GeoPoint* trajPoint in *traj)
		{
			trajPointList.push_back(trajPoint);
		}
	}
	cout << "inside the function" << endl;
	pointGridIndex.createIndex(trajPointList, routeNetwork.getMapRange(), gridWidth);
	trajPointList.clear();
	
}

//ͳ�ƹ켣traj�����й켣��ĵ�ͼƥ�����
void biasStatistic(Traj* traj, list<Edge*> result){
	int trajPointIndex = 0;
	Traj::iterator trajIter = traj->begin();
	//����һ������ͬһ���켣���ֻ����һ���켣��Ͷ�ڸ������У���ֹͬһ���켣ƥ�䵽��·����ͬһ�����ڶ�μ���
	set<pair<int, int>> countedGridCellSet = set<pair<int, int>>();
	for each (Edge* edge in result)
	{
		GeoPoint* trajPoint = *trajIter;
		trajIter++;
		if (edge != NULL){
			pair<int, int> gridCellIndex = pointGridIndex.getRowCol(trajPoint);//ʹ�õ�������Թ켣�㽨��������������ͳ��ƥ����Ϣ
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
	return;
}

//��ÿ���켣��Ӧ��ƥ�����ļ��ж�ȡƥ��·�Σ������˽���ƥ����Ϣͳ��
void biasStatisticFromResults(){
	int trajIndex = 0;
	for each (Traj* traj in trajList)
	{
		MatchedTraj result = MatchedTraj();
		inputMatchedEdges(rootFilePath + outputDirectory + "\\" + outputFileNames[trajIndex], result);
		list<Edge*> edges = list<Edge*>();
		for each (auto var in result)
		{
			if (var != -1){
				edges.push_back(routeNetwork.edges[var]);
			}
			else{
				edges.push_back(NULL);
			}
		}
		biasStatistic(traj, edges);
		trajIndex++;
	}
	return;
}

void main(int argc, char* argv[]){
	if (argc != 1 && argc != 5){
		cout << "����Ӧ�����ĸ���������һ������Ϊ�켣�ļ������ļ��У��ڶ�������Ϊƥ���������ļ��У�����������Ϊ�����������������ȣ����ĸ�����Ϊƥ��·����Ϣͳ���ļ�����" << endl;
	}
	else{
		if (argc == 5){
			inputDirectory = argv[1];
			outputDirectory = argv[2];
			pointIndexGranularity = atoi(argv[3]);
			gridCellBiasFileName = argv[4];
		}
		cout << "�켣�ļ������ļ��У�" << inputDirectory << endl;
		cout << "ƥ���������ļ��У�" << outputDirectory << endl;
		cout << "�����������������ȣ�" << pointIndexGranularity << endl;
		cout << "ƥ��·����Ϣͳ���ļ�����" << gridCellBiasFileName << endl;
		/*�����ļ������켣��ȡ����*/
		scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
		/*�����ļ������켣��ȡ����*/
		//TrajReader trajReader(mergedTrajFilePath);
		//trajReader.readTrajs(trajList);
		//trajReader.makeOutputFileNames(outputFileNames);
		//trajReader.outputMatchedEdges(trajList, rootFilePath + "15days\\15days_answer");//���15��켣�ļ�����ƥ�����һ���������ļ���
		readGridCellBias(gridCellBiasFileName, biasSet, routeNetwork);//�����ѱ���ĵ�����
		pointGridIndex.setGridIndexParameters(routeNetwork.getMapRange(), pointIndexGranularity);
		/*�Թ켣�㼯�ϵ�����������������ʹ����ʷ���ݽ���ƥ��·��ͳ����Ϣʱʹ��*/
		/*
		TODO����ʵ�ϣ�ֻ��ͳ�ƻ��������ƥ����Ϣ������Ҫ��������������ֻ��Ҫά��biasSet��һ���ݽṹ��
		��ˣ��ڱ�Ҫ����¿���ȥ������makeTrajPointGridIndex����������pointGridIndex��
		���д��������е���pointGridIndex.getRowCol�����ĵط������ĳ�������ڵ�����row��col�����ɡ�
		*/
		cout << "zaizheli0" << endl;
		makeTrajPointGridIndex(pointIndexGranularity);
		cout << "zaizheli1" << endl;
		biasStatisticFromResults();
		cout << "zaizheli2" << endl;
		//int trajIndex = 0;
		//cout << "��ʼ��ͼƥ�䣡" << endl;
		//for (list<Traj*>::iterator trajIter = trajList.begin(); trajIter != trajList.end(); trajIter++){
		//	//if (trajIndex == 1365){
		//	//cout << "�켣���ȣ�" << (*trajIter)->size() << endl;
		//	/*ƥ��·����Ϣͳ��*/
		//	//list<Edge*> resultList = MapMatching(*(*trajIter));
		//	//biasStatistic(*trajIter, resultList);
		//	/*����ƥ��·��ͳ����Ϣ���е�ͼƥ��*/
		//	//list<Edge*> resultList = MapMatchingUsingBiasStatistic(*(*trajIter));
		//	/*����ƥ��·��ͳ����Ϣ��Ϊ������ʽ��е�ͼƥ��*/
		//	list<Edge*> resultList = MapMatchingUsingBiasStatisticAsPriorProb(*(*trajIter));
		//	//cout << "��" << trajIndex << "���켣ƥ����ϣ�" << endl;
		//	outputMatchedEdges(rootFilePath + outputDirectory + "\\" + outputFileNames[trajIndex], *trajIter, resultList);
		//	//cout << "��" << trajIndex << "���켣ƥ��·�������ϣ�" << endl;
		//	//}
		//	trajIndex++;
		//}
		//cout << "��ͼƥ����ɣ�" << endl;
		/*���ƥ��·��ͳ����Ϣ���ļ���ʹ����ʷ���ݽ���ƥ��·��ͳ����Ϣʱʹ��*/
		outputGridCellBias(gridCellBiasFileName, biasSet);
	}
	system("pause");
}