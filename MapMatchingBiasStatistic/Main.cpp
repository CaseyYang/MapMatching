#include <iostream>
#include "FileIO.h"
#include "Map.h"
#include "MapMatching.h"
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\GISCUP2012_Data\\";
string inputDirectory = "input_60";
string outputDirectory = "output_60";

vector<string> outputFileNames;//匹配结果
list<Traj*> trajList;

void biasStatistic(Traj* traj,list<Edge*> result){
	int trajPointIndex = 0;
	Traj::iterator trajIter = traj->begin();
	for each (Edge* edge in result)
	{
		GeoPoint* trajPoint = *trajIter;
		trajIter++;
		if (edge != NULL){

		}
		else{
			continue;
		}
	}
}

void main(){
	Map routeNetwork = Map(rootFilePath, 1000);

	scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
	int trajIndex = 0;
	cout << "开始地图匹配！" << endl;
	for (list<Traj*>::iterator trajIter = trajList.begin(); trajIter != trajList.end(); trajIter++){
		list<Edge*> resultList = MapMatching(*(*trajIter));
		//outputMatchedEdges(rootFilePath + outputDirectory + "\\" + outputFileNames[trajIndex], *trajIter, resultList);
		//cout << "第" << trajIndex << "条轨迹匹配完毕！" << endl;
		trajIndex++;
	}


}