#pragma once
#include <iostream>
#include <map>
#include <set>
#include "FileIO.h"
#include "Map.h"
#include "PointGridIndex.h"
#include "GridCenter.h"
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\新加坡数据\\";
string inputDirectory = "test_input";//输入的轨迹文件名要求：以“input_”开头
string outputDirectory = "test_answer";//输出的匹配结果文件名均以“output_”开头
string answerDirectory = "test_answer";//输出的匹配结果文件名均以“output_”开头
string gridCellBiasFileName = "biasStatistic_100.txt";
int pointIndexGranularity = 100;

Map routeNetwork(rootFilePath, 1000);
PointGridIndex litePointGridIndex(routeNetwork.getMapRange(), pointIndexGranularity);
vector<string> outputFileNames;//匹配结果文件名集合
list<Traj*> trajList;//轨迹集合
list<list<int>> resultList;//保存匹配结果集合
list<list<int>> answerList;//保存答案集合

map<pair<int, int>, map<Edge*, int>> historyMatchedDataSet;//地图中基于网格的地图匹配统计情况集合
map<pair<int, int>, GridCenter*> matchedCountSet;//地图中基于网格的记录每个网格中轨迹点匹配成功率的集合

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
	//读入轨迹集合
	scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
	//读入匹配结果集合
	scanTrajFolderAndAnswerFolder(rootFilePath, outputDirectory, answerDirectory, outputFileNames, resultList, answerList);
	//读入网格索引
	readGridCellBias(gridCellBiasFileName, historyMatchedDataSet, routeNetwork);
	//统计匹配比率信息
	matchingCountStatistic();
	//输出至JSON文件
	matchingCountDataToJson("MatchedRatePerGrid.js");
	//回收内存
	disposeAll();

}