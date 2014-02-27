#include "ReadInTrajs.h"
using namespace std;

string rootFilePath = "D:\\Document\\Subjects\\Computer\\Develop\\Data\\SingaporeData\\";
vector<string> outputFileNames;
list<MatchedTraj> resultList;//��ͼƥ��������
list<Traj*> trajList;//ԭʼ�켣����

Map map = Map(rootFilePath, 500);

//�������·����һ����ͼƥ�����ļ�
void ReadOneResultFile(string filePath){
	ifstream fin(filePath);
	MatchedTraj traj = MatchedTraj();
	int time, edgeId;
	double confidence;
	char useless;
	while (fin >> time){
		fin >> useless >> edgeId >> useless >> confidence;
		traj.push_back(edgeId);
	}
	fin.close();
	resultList.push_back(traj);
}

//�ѵ�ͼƥ����ת�����ܹ����ӻ���JSON�ļ���ʽ
void MatchedTrajToJson(){
	ofstream fout("MapMatching.js");
	fout << "data = [" << endl;
	int trajIndex = 0;
	list<MatchedTraj>::iterator resultListIter = resultList.begin();
	for each (Traj* traj in trajList)
	{
		fout << "{\"city:\":\"Singapore\",\"trajPoints\":[";
		int trajPointIndex = 0;
		for each (GeoPoint* trajPoint in *traj)
		{
			fout.precision(11);
			fout << "{\"x\":" << trajPoint->lon << ",\"y\":" << trajPoint->lat << "}";
			if (trajPointIndex < traj->size() - 1){
				fout << ",";
			}
			trajPointIndex++;
		}
		fout << "],\"matchedEdges\":[";
		int edgeIndex = 0;
		for each (int edgeId in *resultListIter)
		{
			if (edgeId == -1){
				fout << "{\"numOfFigures\":0}";
			}
			else
			{
				fout << "{\"numOfFigures\":" << map.edges.at(edgeId)->figure->size() << ",\"figures\":[";
				int figPointIndex = 0;
				for each (GeoPoint* figPoint in *(map.edges.at(edgeId)->figure))
				{
					fout.precision(11);
					fout << "{\"x\":" << figPoint->lon << ",\"y\":" << figPoint->lat << "}";
					if (figPointIndex < map.edges.at(edgeId)->figure->size() - 1){
						fout << ",";
					}
					figPointIndex++;
				}
				fout << "]}";
			}
			if (edgeIndex < resultListIter->size() - 1){
				fout << ",";
			}
			edgeIndex++;
		}
		fout << "]}";
		if (trajIndex < resultList.size() - 1){
			fout << "," << endl;
		}
		else{
			fout << endl;
		}
		trajIndex++;
		resultListIter++;
	}
	fout << "]";
	fout.close();
}

//����������ͼƥ��·�ε���ͨ��
void MatchedTrajConnection(){
	int trajIndex = 0;
	list<MatchedTraj>::iterator resultListIter = resultList.begin();
	for each (Traj* traj in trajList)
	{
		double unconnectedEdgeCount = 0;
		double allValidEdgeCount = 0;
		double allEdgeCount = 0;
		int formerEdgeId = -1;
		for each (int edgeId in *resultListIter)
		{
			if (formerEdgeId != -1 && edgeId != -1){
				if (formerEdgeId != edgeId && map.edges.at(formerEdgeId)->endNodeId != map.edges.at(edgeId)->startNodeId){
					//cout << formerEdgeId << endl;
					//cout << edgeId << endl;
					//system("pause");
					unconnectedEdgeCount++;
				}
				allValidEdgeCount++;
			}
			formerEdgeId = edgeId;
			allEdgeCount++;
		}
		cout << "��" << trajIndex << "���켣�ķ���ͨ����" << unconnectedEdgeCount / allValidEdgeCount << endl;
		cout << "��" << trajIndex << "���켣�ĺϹ�����" << allValidEdgeCount / allEdgeCount << endl;
		trajIndex++;
		resultListIter++;
	}
}

void main(){
	scanTrajFolder(rootFilePath, trajList, outputFileNames);
	readResultFiles(rootFilePath, outputFileNames, resultList);
	MatchedTrajToJson();
	MatchedTrajConnection();
}