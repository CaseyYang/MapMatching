#include "Map.h"
#include "ReadInTrajs.h"

const double east = cos(22.5*PI180);
const double northMax = cos(67.5*PI180);
const double northMin = cos(112.5*PI180);
const double west = cos(157.5*PI180);

string rootFilePath = "D:\\MapMatchingProject\\Data\\";
Map routeNetwork = Map(rootFilePath, 500);
list<Traj*> trajList = list<Traj*>();

//返回指定边的大致方向，有正东、正南、正西、正北、东南、东北、西南、西北八种可能的返回值
string GetEdgeDirection(Edge* edge){
	//edgeVector表示路段起点和终点连线组成的向量
	pair<double, double> edgeVector = make_pair(routeNetwork.nodes.at(edge->endNodeId)->lon - routeNetwork.nodes.at(edge->startNodeId)->lon, routeNetwork.nodes.at(edge->endNodeId)->lat - routeNetwork.nodes.at(edge->startNodeId)->lat);
	double hypotenuse = sqrt(pow(edgeVector.first, 2) + pow(edgeVector.second, 2));//hypotenuse是直角三角形中斜边的英文
	double cosValue = edgeVector.first / hypotenuse;
	if (cosValue > east){
		return "正东方向";
	}
	if (cosValue<east&&cosValue>northMax){
		if (edgeVector.second > 0){
			return "东北方向";
		}
		else{
			return "东南方向";
		}
	}
	if (cosValue<northMax&&cosValue>northMin){
		if (edgeVector.second > 0){
			return "正北方向";
		}
		else{
			return "正南方向";
		}
	}
	if (cosValue<northMin&&cosValue>west){
		if (edgeVector.second > 0){
			return "西北方向";
		}
		else{
			return "西南方向";
		}
	}
	if (cosValue < west){
		return "正西方向";
	}
}

//把路段转成JSON格式保存，如果trajList不为空，则保存给定轨迹附近的路段和轨迹采样点
void EdgesToJson(list<Traj*> &trajList, vector<Edge*> &edges){
	ofstream fout("RouteNetworkData.js");
	fout.precision(11);
	fout << "routeNetwork = " << endl;
	fout << "{\"city\":\"Singapore\"," << endl;
	if (trajList.size() > 0){
		fout << "\"trajs\":[";
		size_t trajIndex = 0;
		for each (Traj* traj in trajList)
		{
			if (trajIndex > 0){
				fout << "," << endl;
			}
			fout << "{\"trajName\":" << trajIndex + 1 << ",\"numOfTrajPoints\":" << traj->size() << ",\"trajPoints\":[";
			size_t trajPointIndex = 0;
			for each (GeoPoint* trajPoint in *traj)
			{
				fout << "{\"x\":" << trajPoint->lon << ",\"y\":" << trajPoint->lat << "}";
				if (trajPointIndex < traj->size() - 1){
					fout << ",";
				}
				++trajPointIndex;
			}
			fout << "]}";
			++trajIndex;
		}
		fout << "]," << endl;
	}
	fout << "\"edges\":[";
	int edgeIndex = 0;
	for each(Edge* edge in edges){
		if (edge != NULL){
			if (edgeIndex > 0){
				fout << "," << endl;
			}
			string direction = GetEdgeDirection(edge);
			fout << "{\"edgeId\":" << edge->id << ",\"numOfFigures\":" << edge->figure->size() << ",\"direction\":\"" << direction << "\",\"figures\":[";
			size_t figureIndex = 0;
			for each (GeoPoint* figPoint in *(edge->figure)){
				fout << "{\"x\":" << figPoint->lon << ",\"y\":" << figPoint->lat << "}";
				if (figureIndex < edge->figure->size() - 1){
					fout << ",";
				}
				++figureIndex;
			}
			fout << "]}";
		}
		++edgeIndex;
	}
	fout << "]}" << endl;
	fout.close();
}

//根据给定的轨迹，把该轨迹附近的路段转成JSON格式保存，同时在Json中保存轨迹采样点
void UsefulEdgesToJson(){
	vector<string> outputFileNames;
	list<Traj*> trajList = list<Traj*>();
	scanTrajFolder(rootFilePath, trajList, outputFileNames);
	set<Edge*> usefulEdges = set<Edge*>();
	for each(Traj* traj in trajList){
		for each(GeoPoint* trajPoint in *traj){
			vector<Edge*> edges = routeNetwork.getNearEdges(trajPoint->lat, trajPoint->lon, 100.0);
			usefulEdges.insert(edges.begin(), edges.end());
		}
	}
	EdgesToJson(trajList, vector<Edge*>(usefulEdges.begin(), usefulEdges.end()));
}

void main(){
	vector<string> outputFileNames;
	scanTrajFolder(rootFilePath, trajList, outputFileNames);//只有在需要导出某些轨迹附近的路段到Json文件时，才需要读入轨迹文件夹
	cout << "文件读入完毕！" << endl;
	UsefulEdgesToJson();
}