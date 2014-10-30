#include "Map.h"
#include "ReadInTrajs.h"

const double east = cos(22.5*PI180);
const double northMax = cos(67.5*PI180);
const double northMin = cos(112.5*PI180);
const double west = cos(157.5*PI180);

string rootFilePath = "D:\\MapMatchingProject\\Data\\";
Map routeNetwork = Map(rootFilePath, 500);
list<Traj*> trajList = list<Traj*>();

//����ָ���ߵĴ��·��������������ϡ����������������ϡ����������ϡ��������ֿ��ܵķ���ֵ
string GetEdgeDirection(Edge* edge){
	//edgeVector��ʾ·�������յ�������ɵ�����
	pair<double, double> edgeVector = make_pair(routeNetwork.nodes.at(edge->endNodeId)->lon - routeNetwork.nodes.at(edge->startNodeId)->lon, routeNetwork.nodes.at(edge->endNodeId)->lat - routeNetwork.nodes.at(edge->startNodeId)->lat);
	double hypotenuse = sqrt(pow(edgeVector.first, 2) + pow(edgeVector.second, 2));//hypotenuse��ֱ����������б�ߵ�Ӣ��
	double cosValue = edgeVector.first / hypotenuse;
	if (cosValue > east){
		return "��������";
	}
	if (cosValue<east&&cosValue>northMax){
		if (edgeVector.second > 0){
			return "��������";
		}
		else{
			return "���Ϸ���";
		}
	}
	if (cosValue<northMax&&cosValue>northMin){
		if (edgeVector.second > 0){
			return "��������";
		}
		else{
			return "���Ϸ���";
		}
	}
	if (cosValue<northMin&&cosValue>west){
		if (edgeVector.second > 0){
			return "��������";
		}
		else{
			return "���Ϸ���";
		}
	}
	if (cosValue < west){
		return "��������";
	}
}

//��·��ת��JSON��ʽ���棬���trajList��Ϊ�գ��򱣴�����켣������·�κ͹켣������
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

//���ݸ����Ĺ켣���Ѹù켣������·��ת��JSON��ʽ���棬ͬʱ��Json�б���켣������
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
	scanTrajFolder(rootFilePath, trajList, outputFileNames);//ֻ������Ҫ����ĳЩ�켣������·�ε�Json�ļ�ʱ������Ҫ����켣�ļ���
	cout << "�ļ�������ϣ�" << endl;
	UsefulEdgesToJson();
}