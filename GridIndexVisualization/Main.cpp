#pragma once
#include <iostream>
#include <map>
#include <set>
#include "FileIO.h"
#include "Map.h"
#include "PointGridIndex.h"
#include "GridCenter.h"
#include "Area.h"
#include "GeoPoint.h"
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\�¼�������\\";
string inputDirectory = "15days\\15days_separated_high_quality_200s_input";//����Ĺ켣�ļ���Ҫ���ԡ�input_����ͷ
string outputDirectory = "15days\\15days_separated_high_quality_200s_M2_output";//�����ƥ�����ļ������ԡ�output_����ͷ
string answerDirectory = "15days\\15days_separated_high_quality_200s_answer";//�����ƥ�����ļ������ԡ�output_����ͷ
string gridCellBiasFileName = "biasStatistic_6000.txt";
int pointIndexGranularity = 6000;

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
	int trajIndex = 0;
	for (; trajListIter != trajList.end(); ++trajListIter, ++resultListIter, ++answerListIter){
		//cout << "�켣"<<trajIndex << endl;
		//++trajIndex;
		auto trajIter = (*trajListIter)->begin();
		auto resultIter = resultListIter->begin();
		auto answerIter = answerListIter->begin();
		int pointIndex = 0;
		for (; trajIter != (*trajListIter)->end(); ++trajIter, ++resultIter, ++answerIter){
			//cout << "��" << pointIndex << endl;
			//++pointIndex;
			pair<int, int> cell = litePointGridIndex.getRowCol(*trajIter);
			if (historyMatchedDataSet.find(cell) != historyMatchedDataSet.end()){
				if (matchedCountSet.find(cell) == matchedCountSet.end()){
					pair<double, double> centerCoordinate = litePointGridIndex.getCenterCorrdinate(cell);
					matchedCountSet[cell] = new GridCenter(centerCoordinate.first, centerCoordinate.second);
				}
				if ((*resultIter) == (*answerIter)){
					++matchedCountSet[cell]->matchedCount;
				}
				++matchedCountSet[cell]->matchingCount;
			}
		}
	}
	cout << matchedCountSet.size() << endl;
	for (auto setIter = matchedCountSet.begin(); setIter != matchedCountSet.end(); ++setIter){
		setIter->second->calculateCorrectRate();
	}
}

void matchingCountDataToJson(string filePath){
	ofstream writer(filePath);
	writer.precision(13);
	writer << "matchingCountData={" << endl;
	writer << "\"city\":\"Singapore\"," << endl;
	writer << "\"grid\":[" << endl;
	int commaIndex = static_cast<int>(matchedCountSet.size()) - 1;
	for each(auto gridInfoPair in matchedCountSet){
		writer << "{" << gridInfoPair.second->toJsonStr() << "}";
		if (commaIndex > 0){
			writer << ",";
			--commaIndex;
		}
	}
	writer << "]" << endl;
	writer << "}" << endl;
	writer.close();
}

void disposeAll(){
	for (auto iter = trajList.begin(); iter != trajList.end(); ++iter){
		for (auto subIter = (*iter)->begin(); subIter != (*iter)->end(); ++subIter){
			delete *subIter;
		}
		delete *iter;
	}
	for (auto iter = matchedCountSet.begin(); iter != matchedCountSet.end(); ++iter){
		delete iter->second;
	}
}

void main(int argc, char* argv[]){
	//����켣����
	scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
	//����ƥ��������
	scanTrajFolderAndAnswerFolder(rootFilePath, outputDirectory, answerDirectory, outputFileNames, resultList, answerList);
	//������������
	readGridCellBias(gridCellBiasFileName, historyMatchedDataSet, routeNetwork);
	//ͳ��ƥ�������Ϣ
	matchingCountStatistic();
	//�����JSON�ļ�
	matchingCountDataToJson("MatchedRatePerGrid.js");
	//�����ڴ�
	disposeAll();
	system("pause");
}