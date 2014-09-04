#include <iostream>
#include <map>
#include <set>
#include "FileIO.h"
#include "Map.h"
#include "MapMatching.h"
#include "MapMatchingUsingBiasStatistic.h"
#include "TrajReader.h"
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\新加坡数据\\";
string inputDirectory = "day3\\day3_unsplit";//输入的轨迹文件名要求：以“input_”开头
string outputDirectory = "15days\\15days_output2";//输出的匹配结果文件名均以“output_”开头
string gridCellBiasFileName = "biasStatistic.txt";
string mergedTrajFilePath = "D:\\MapMatchingProject\\Data\\新加坡数据\\15days\\wy_MMTrajs.txt";
Map routeNetwork = Map(rootFilePath, 3100);

vector<string> outputFileNames;//匹配结果文件名集合
list<Traj*> trajList;//轨迹集合
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
	/*单个文件单条轨迹读取方法*/
	scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
	/*单个文件多条轨迹读取方法*/
	//TrajReader trajReader(mergedTrajFilePath);
	//trajReader.readTrajs(trajList);
	//trajReader.outputMatchedEdges(trajList, rootFilePath + "15days\\15days_answer");
	//trajReader.makeOutputFileNames(outputFileNames);
	readGridCellBias(gridCellBiasFileName, biasSet, routeNetwork);
	int trajIndex = 0;
	cout << "开始地图匹配！" << endl;
	for (list<Traj*>::iterator trajIter = trajList.begin(); trajIter != trajList.end(); trajIter++){
		//if (trajIndex == 1365){
			//cout << "轨迹长度：" << (*trajIter)->size() << endl;
			/*匹配路段信息统计*/
			list<Edge*> resultList = MapMatching(*(*trajIter));
			biasStatistic(*trajIter, resultList);
			/*利用匹配路段统计信息进行地图匹配*/
			//list<Edge*> resultList = MapMatchingUsingBiasStatistic(*(*trajIter));
			/*利用匹配路段统计信息作为后验概率进行地图匹配*/
			//list<Edge*> resultList = MapMatchingUsingBiasStatisticAsPriorProb(*(*trajIter));
			//cout << "第" << tra2000jIndex << "条轨迹匹配完毕！" << endl;
			//outputMatchedEdges(rootFilePath + outputDirectory + "\\" + outputFileNames[trajIndex], *trajIter, resultList);
			cout << "第" << trajIndex << "条轨迹匹配路段输出完毕！" << endl;
		//}
		trajIndex++;
	}
	cout << "地图匹配完成！" << endl;
	outputGridCellBias(gridCellBiasFileName, biasSet);
	system("pause");
}