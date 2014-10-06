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

string rootFilePath = "D:\\MapMatchingProject\\Data\\新加坡数据\\";
string inputDirectory = "day1\\day1_splited_input";//输入的轨迹文件名要求：以“input_”开头
string outputDirectory = "day1\\day1_splited_answer";//输出的匹配结果文件名均以“output_”开头
string gridCellBiasFileName = "biasStatistic_15000.txt";
string mergedTrajFilePath = "D:\\MapMatchingProject\\Data\\新加坡数据\\15days\\wy_MMTrajs.txt";
int pointIndexGranularity = 15000;

Map routeNetwork = Map(rootFilePath, 1000);
PointGridIndex pointGridIndex;//针对所有轨迹点建立的网格索引
vector<string> outputFileNames;//匹配结果文件名集合
list<Traj*> trajList;//轨迹集合

map<pair<int, int>, map<Edge*, int>> biasSet;//地图中基于网格的地图匹配统计情况集合

//把轨迹集合trajList中所有轨迹点合并到集合trajPointList中
void makeTrajPointGridIndex(int gridWidth){
	list<GeoPoint*> trajPointList = list<GeoPoint*>();//所有轨迹点集合
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

//统计轨迹traj中所有轨迹点的地图匹配情况
void biasStatistic(Traj* traj, list<Edge*> result){
	int trajPointIndex = 0;
	Traj::iterator trajIter = traj->begin();
	//对于一个网格，同一条轨迹最多只能有一个轨迹点投在该网格中，防止同一条轨迹匹配到的路段在同一网格内多次计算
	set<pair<int, int>> countedGridCellSet = set<pair<int, int>>();
	for each (Edge* edge in result)
	{
		GeoPoint* trajPoint = *trajIter;
		trajIter++;
		if (edge != NULL){
			pair<int, int> gridCellIndex = pointGridIndex.getRowCol(trajPoint);//使用单独的针对轨迹点建立的网格索引来统计匹配信息
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

//从每条轨迹对应的匹配结果文件中读取匹配路段，并依此进行匹配信息统计
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
		cout << "至多应该有四个参数：第一个参数为轨迹文件所在文件夹；第二个参数为匹配结果所在文件夹；第三个参数为采样点网格索引粒度；第四个参数为匹配路段信息统计文件名。" << endl;
	}
	else{
		if (argc == 5){
			inputDirectory = argv[1];
			outputDirectory = argv[2];
			pointIndexGranularity = atoi(argv[3]);
			gridCellBiasFileName = argv[4];
		}
		cout << "轨迹文件所在文件夹：" << inputDirectory << endl;
		cout << "匹配结果所在文件夹：" << outputDirectory << endl;
		cout << "采样点网格索引粒度：" << pointIndexGranularity << endl;
		cout << "匹配路段信息统计文件名：" << gridCellBiasFileName << endl;
		/*单个文件单条轨迹读取方法*/
		scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
		/*单个文件多条轨迹读取方法*/
		//TrajReader trajReader(mergedTrajFilePath);
		//trajReader.readTrajs(trajList);
		//trajReader.makeOutputFileNames(outputFileNames);
		//trajReader.outputMatchedEdges(trajList, rootFilePath + "15days\\15days_answer");//输出15天轨迹文件中已匹配答案至一个单独的文件中
		readGridCellBias(gridCellBiasFileName, biasSet, routeNetwork);//读入已保存的点索引
		pointGridIndex.setGridIndexParameters(routeNetwork.getMapRange(), pointIndexGranularity);
		/*对轨迹点集合单独建立网格索引：使用历史数据建立匹配路段统计信息时使用*/
		/*
		TODO：事实上，只是统计基于网格的匹配信息并不需要建立网格索引，只需要维护biasSet这一数据结构。
		因此，在必要情况下可以去掉函数makeTrajPointGridIndex和网格索引pointGridIndex。
		现有代码中所有调用pointGridIndex.getRowCol方法的地方计算出某个点所在的网格（row，col）即可。
		*/
		cout << "zaizheli0" << endl;
		makeTrajPointGridIndex(pointIndexGranularity);
		cout << "zaizheli1" << endl;
		biasStatisticFromResults();
		cout << "zaizheli2" << endl;
		//int trajIndex = 0;
		//cout << "开始地图匹配！" << endl;
		//for (list<Traj*>::iterator trajIter = trajList.begin(); trajIter != trajList.end(); trajIter++){
		//	//if (trajIndex == 1365){
		//	//cout << "轨迹长度：" << (*trajIter)->size() << endl;
		//	/*匹配路段信息统计*/
		//	//list<Edge*> resultList = MapMatching(*(*trajIter));
		//	//biasStatistic(*trajIter, resultList);
		//	/*利用匹配路段统计信息进行地图匹配*/
		//	//list<Edge*> resultList = MapMatchingUsingBiasStatistic(*(*trajIter));
		//	/*利用匹配路段统计信息作为后验概率进行地图匹配*/
		//	list<Edge*> resultList = MapMatchingUsingBiasStatisticAsPriorProb(*(*trajIter));
		//	//cout << "第" << trajIndex << "条轨迹匹配完毕！" << endl;
		//	outputMatchedEdges(rootFilePath + outputDirectory + "\\" + outputFileNames[trajIndex], *trajIter, resultList);
		//	//cout << "第" << trajIndex << "条轨迹匹配路段输出完毕！" << endl;
		//	//}
		//	trajIndex++;
		//}
		//cout << "地图匹配完成！" << endl;
		/*输出匹配路段统计信息至文件：使用历史数据建立匹配路段统计信息时使用*/
		outputGridCellBias(gridCellBiasFileName, biasSet);
	}
	system("pause");
}