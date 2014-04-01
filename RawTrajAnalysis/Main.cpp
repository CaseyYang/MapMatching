#include <iostream>
#include<sstream>
#include <math.h>
#include "ReadInTrajs.h"
using namespace std;


string rootFilePath = "D:\\Document\\MDM Lab\\Data\\GISCUP2012_Data\\";
Map routeNetwork(rootFilePath, 500);
list<Traj*> trajList = list<Traj*>();


double CalculateMAD(list<double> &dist){
	dist.sort();
	int mid = dist.size() / 2;
	list<double>::iterator dist1Iter = dist.begin();
	for (int i = 0; i < mid; i++){
		dist1Iter++;
	}
	double median = -1;
	if (dist.size() % 2 != 0){
		median = *dist1Iter;
	}
	else
	{
		median = *dist1Iter;
		dist1Iter++;
		median += *dist1Iter;
		median /= 2;
	}
	for (dist1Iter = dist.begin(); dist1Iter != dist.end(); dist1Iter++){
		(*dist1Iter) = abs((*dist1Iter) - median);
	}
	dist.sort();
	dist1Iter = dist.begin();
	for (int i = 0; i < mid; i++){
		dist1Iter++;
	}
	if (dist.size() % 2 != 0){
		median = *dist1Iter;
	}
	else
	{
		median = *dist1Iter;
		dist1Iter++;
		median += *dist1Iter;
		median /= 2;
	}
	return median;
}

//计算Viterbi算法中所用到的转移概率和放射概率的相关参数
void CalculateParametersForViterbiAlgorithm(){
	long double sigma = 0, beta = 0;
	int indexOfTrajs = 0;
	for (list<Traj*>::iterator trajIter = trajList.begin(); trajIter != trajList.end(); trajIter++)
	{
		list<double> dist1 = list<double>();
		list<double> dist2 = list<double>();
		bool isStart = true;
		GeoPoint* formerTrajPoint = NULL;
		Edge* formerEdge = NULL;
		int indexOfTrajPoints = 0;
		for (list<GeoPoint*>::iterator trajPointIter = (*trajIter)->begin(); trajPointIter != (*trajIter)->end(); trajPointIter++){
			double shortestDist = 100000000.0;
			Edge* nearestEdge = routeNetwork.getNearestEdge((*trajPointIter)->lat, (*trajPointIter)->lon, shortestDist);
			if (nearestEdge != NULL){
				if (!isStart){
					list<Edge*> shortestPath;
					dist2.push_back(abs(GeoPoint::distM(formerTrajPoint, (*trajPointIter)) - routeNetwork.shortestPathLength(formerEdge->startNodeId, nearestEdge->startNodeId, shortestPath)));
				}
				dist1.push_back(shortestDist);
				formerTrajPoint = (*trajPointIter);
				formerEdge = nearestEdge;
				isStart = false;
			}
			indexOfTrajPoints++;
		}
		sigma += CalculateMAD(dist1);
		beta += CalculateMAD(dist2) / log(2);
		indexOfTrajs++;
		if (indexOfTrajs % 30 == 0){
			cout << "已完成" << indexOfTrajs * 100 / trajList.size() << "%" << endl;
		}
	}
	sigma /= trajList.size();
	cout.precision(11);
	cout << "sigma = " << sigma << endl;
	beta /= trajList.size();
	cout << "beta = " << beta << endl;
	cout << "2*(sigma^2)/beta = " << 2 * sqrt(sigma) / beta << endl;
}

string ToString(int i){
	stringstream ss;
	ss << i;
	return ss.str();
}

//如果一条轨迹的相邻两轨迹点的欧氏距离除以时间大于maxSpeed（单位是米/秒），则在此处分割这条轨迹；得到的新的轨迹文件保存在和程序相同目录下
void trajSplit(double maxSpeed){
	int trajIndex = 0;
	ofstream *fout;
	for (list<Traj*>::iterator trajIter = trajList.begin(); trajIter != trajList.end(); trajIter++){
		int goodTrajIndex = 0;
		fout = new ofstream("input_" + ToString(trajIndex) + "_" + ToString(goodTrajIndex) + ".txt");
		fout->precision(11);
		GeoPoint* formerTrajPoint = NULL;
		for each (GeoPoint* trajPoint in *(*trajIter))
		{
			if (formerTrajPoint != NULL && GeoPoint::distM(formerTrajPoint, trajPoint) / (trajPoint->time - formerTrajPoint->time) > maxSpeed){
				fout->close();
				delete fout;
				goodTrajIndex++;
				fout = new ofstream("input_" + ToString(trajIndex) + "_" + ToString(goodTrajIndex) + ".txt");
				cout << "input_" + ToString(trajIndex) + "_" + ToString(goodTrajIndex) + ".txt" << endl;
				fout->precision(11);
			}
			*fout << trajPoint->time << "," << trajPoint->lat << "," << trajPoint->lon << endl;
			formerTrajPoint = trajPoint;
		}
		trajIndex++;
		fout->close();
	}
	delete fout;
}

//计算轨迹平均采样率
void CalculateAverageSampleRate(){
	double totalAverageSampleRate = 0;
	for (list<Traj*>::iterator iter = trajList.begin(); iter != trajList.end(); iter++){
		double formerTrajPointTimeStamp = -1;
		double averageSampleRate = 0;
		for (Traj::iterator trajPointIter = (*iter)->begin(); trajPointIter != (*iter)->end(); trajPointIter++){
			if (formerTrajPointTimeStamp != -1){
				averageSampleRate += (*trajPointIter)->time - formerTrajPointTimeStamp;
			}
			formerTrajPointTimeStamp = (*trajPointIter)->time;
		}
		averageSampleRate /= ((*iter)->size() - 1);
		totalAverageSampleRate += averageSampleRate;
	}
	totalAverageSampleRate /= trajList.size();
	cout << "平均采样率：" << totalAverageSampleRate << endl;
}


//读入文件流，把原始轨迹转为Json文件
//格式为：data={"rawTrajsId":XXX, "points":[{"x":XXX,"y":XXX,"t":XXX},……]}
void RawTrajToJson(string filePath){
	ifstream fin(filePath);
	ofstream fout("RawTrajData.js");
	fout.precision(20);
	int timeStamp;
	double x, y;
	bool start = true;
	string city = "Shanghai";
	fout << "data = {" << endl;
	fout << "\"city\":\""<<city<<"\",\"rawTrajsId\":\"" << filePath << "\",\"points\":[" << endl;
	while (fin>>timeStamp){
		fin >> y >> x;
		if (!start){
			fout << "," << endl<<"{";
		}
		else{
			fout << "{";
			start = false;
		}
		fout << "\"x\":" << x << ",\"y\":" << y << ",\"t\":" << timeStamp << "}";
	}
	fout << "]}" << endl;
	fin.close();
	fout.close();
}

int main(){
	vector<string> outputFileNames;
	//scanTrajFolder(rootFilePath, trajList, outputFileNames);
	//cout << "文件读入完毕！" << endl;
	//CalculateParametersForViterbiAlgorithm();
	//CalculateAverageSampleRate();
	RawTrajToJson("2014-03-25 16_44_11.txt");
	return 0;
}