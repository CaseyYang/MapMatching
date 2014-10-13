#include <iostream>
#include <map>
#include <set>
#include "FileIO.h"
#include "Map.h"
#include "PointGridIndex.h"
#include "GridCenter.h"
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\�¼�������\\";
string inputDirectory = "test_input";//����Ĺ켣�ļ���Ҫ���ԡ�input_����ͷ
string outputDirectory = "test_answer";//�����ƥ�����ļ������ԡ�output_����ͷ
string answerDirectory = "test_answer";//�����ƥ�����ļ������ԡ�output_����ͷ
string gridCellBiasFileName = "biasStatistic_100.txt";
int pointIndexGranularity = 100;

Map routeNetwork(rootFilePath, 1000);
PointGridIndex litePointGridIndex(routeNetwork.getMapRange(), pointIndexGranularity);
vector<string> outputFileNames;//ƥ�����ļ�������
list<Traj*> trajList;//�켣����
list<list<int>> resultList;//����ƥ��������
list<list<int>> answerList;//����𰸼���

map<pair<int, int>, map<Edge*, int>> historyMatchedDataSet;//��ͼ�л�������ĵ�ͼƥ��ͳ���������
map<pair<int, int>, GridCenter*> matchedCountSet;//��ͼ�л�������ļ�¼ÿ�������й켣��ƥ��ɹ��ʵļ���

void matchingCountStatistic(){
	auto trajListIter = trajList.begin();
	auto resultListIter = resultList.begin();
	auto answerListIter = answerList.begin();
	for (; trajListIter != trajList.end(); ++trajListIter, ++resultListIter, ++answerListIter){
		auto trajIter = (*trajListIter)->begin();
		auto resultIter = resultListIter->begin();
		auto answerIter = answerListIter->begin();
		for (; trajIter != (*trajListIter)->end(); ++trajIter, ++resultIter, ++answerIter){
			pair<int, int> cell = routeNetwork.findGridCellIndex((*trajIter)->lat, (*trajIter)->lon);
			if (historyMatchedDataSet.find(cell) != historyMatchedDataSet.end()){
				if (matchedCountSet.find(cell) != matchedCountSet.end()){
					pair<double, double> centerCoordinate = litePointGridIndex.getCenterCorrdinate(cell);
					matchedCountSet[cell] = new GridCenter(centerCoordinate.first, centerCoordinate.second);
				}
				if ((*resultIter) == (*answerIter)){
					++matchedCountSet[cell]->matchedCount;
				}
				matchedCountSet[cell]->matchingCount++;
			}
		}
	}
	for (auto setIter = matchedCountSet.begin(); setIter != matchedCountSet.end(); ++setIter){
		setIter->second->calculateCorrectRate();
	}
}

void matchingCountDataToJson(string filePath){
	ofstream writer(filePath);

	writer.close();
}

void main(int argc, char* argv[]){
	//����켣����
	scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
	//����ƥ��������
	scanTrajFolderAndAnswerFolder(rootFilePath, outputDirectory, answerDirectory, outputFileNames, resultList, answerList);
	//������������
	readGridCellBias(gridCellBiasFileName, historyMatchedDataSet, routeNetwork);//�����ѱ���ĵ�����
	
}