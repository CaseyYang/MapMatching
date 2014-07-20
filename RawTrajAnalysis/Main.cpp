#include <iostream>
#include<sstream>
#include <math.h>
#include "ReadInTrajs.h"
using namespace std;


string rootFilePath = "D:\\Document\\MDM Lab\\Data\\GISCUP2012_Data\\";
Map routeNetwork(rootFilePath, 500);
list<Traj*> trajList = list<Traj*>();
int sampleRate = 90;//Ҫ�����Ĳ��������DegradeInput��DegradeAnswer��������

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

//����Viterbi�㷨�����õ���ת�Ƹ��ʺͷ�����ʵ���ز���
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
			cout << "�����" << indexOfTrajs * 100 / trajList.size() << "%" << endl;
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

//���һ���켣���������켣���ŷ�Ͼ������ʱ�����maxSpeed����λ����/�룩�����ڴ˴��ָ������켣���õ����µĹ켣�ļ������ںͳ�����ͬĿ¼��
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

//����켣ƽ��������
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
	cout << "ƽ�������ʣ�" << totalAverageSampleRate << endl;
}

//�Ը߲����ʹ켣���ݽ���������ʣ�ÿ��sampleRateѡȡһ�������㣬�õ��µķ���sampleRate�����ʵĹ켣
//inputDirectory���¹켣�ļ����ڵ��ļ�����
void DegradeInput(string inputDirectory){
	string completeInputFilesPath = rootFilePath + "input\\*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//�ļ���Ϣ
	long lf;//�ļ����
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		return;
	}
	else {
		int index = 0;
		do {
			string inputFileName = fileInfo.name;
			ifstream fin(rootFilePath + "input\\" + inputFileName);
			ofstream *fout = new ofstream[sampleRate];
			for (size_t i = 0; i < sampleRate; i++)
			{
				fout[i] = ofstream(rootFilePath + inputDirectory + "\\" + inputFileName.substr(0, 6) + ToString(index) + ToString(i) + ".txt");
				fout[i].precision(13);
			}
			int time;
			double lat, lon;
			char useless;
			while (fin >> time){
				fin >> useless >> lat >> useless >> lon;
				fout[time%sampleRate] << time << useless << lat << useless << lon << endl;
			}
			for (size_t i = 0; i < sampleRate; i++)
			{
				fout[i].close();
			}
			//delete fout;
			fin.close();
			index++;
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		return;
	}
}
//��DegradeInput���Ӧ���Ը߲����ʹ켣�������Ӧ��·����Ϣ���г�ȡ���õ����µĲ����ʵĹ켣��Ӧ��·������
//answerDirectory���´��ļ����ڵ��ļ�����
void DegradeAnswer(string answerDirectory){
	string completeInputFilesPath = rootFilePath + "answer\\*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//�ļ���Ϣ
	long lf;//�ļ����
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		return;
	}
	else {
		int index = 0;
		do {
			string inputFileName = fileInfo.name;
			ifstream fin(rootFilePath + "answer\\" + inputFileName);
			ofstream *fout = new ofstream[sampleRate];
			for (size_t i = 0; i < sampleRate; i++)
			{
				fout[i] = ofstream(rootFilePath + answerDirectory + "\\" + inputFileName.substr(0, 7) + ToString(index) + ToString(i) + ".txt");
				fout[i].setf(ios::showpoint);
				fout[i].precision(13);
			}
			int time, edge;
			double confidence;
			char useless;
			while (fin >> time){
				fin >> useless >> edge >> useless >> confidence;
				fout[time%sampleRate] << time << useless << edge << useless << confidence << endl;
			}
			for (size_t i = 0; i < sampleRate; i++)
			{
				fout[i].close();
			}
			fin.close();
			index++;
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		return;
	}
}

//�����ļ�������ԭʼ�켣תΪJson�ļ�
//��ʽΪ��data={"rawTrajsId":XXX, "points":[{"x":XXX,"y":XXX,"t":XXX},����]}
void RawTrajToJson(string filePath){
	ifstream fin(filePath);
	ofstream fout("RawTrajData.js");
	fout.precision(20);
	int timeStamp;
	double x, y;
	bool start = true;
	string city = "Shanghai";
	fout << "data = {" << endl;
	fout << "\"city\":\"" << city << "\",\"rawTrajsId\":\"" << filePath << "\",\"points\":[" << endl;
	while (fin >> timeStamp){
		fin >> y >> x;
		if (!start){
			fout << "," << endl << "{";
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
	trajList = list<Traj*>();
	sampleRate = 90;
	DegradeInput("input_90");
	DegradeAnswer("answer_90");

	trajList = list<Traj*>();
	sampleRate = 120;
	DegradeInput("input_120");
	DegradeAnswer("answer_120");

	trajList = list<Traj*>();
	sampleRate = 150;
	DegradeInput("input_150");
	DegradeAnswer("answer_150");

	vector<string> outputFileNames;
	//scanTrajFolder(rootFilePath, trajList, outputFileNames);
	//cout << "�ļ�������ϣ�" << endl;
	//CalculateParametersForViterbiAlgorithm();
	//CalculateAverageSampleRate();
	//RawTrajToJson("2014-03-25 16_44_11.txt");
	return 0;
}