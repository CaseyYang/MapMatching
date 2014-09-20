#include <iostream>
#include<sstream>
#include <math.h>
#include "ReadInTrajs.h"
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\�¼�������\\";
string inputDirectory = "15days\\15days_separated_high_quality_input";//�켣�ļ������ļ���·�������а����Ĺ켣�ļ���Ҫ���ԡ�input_����ͷ
string newInputDirectory = "15days\\15days_separated_high_quality_120s_input";//���Ͳ����ʺ�Ĺ켣�ļ������ļ���·����
string answerDirectory = "15days\\15days_separated_high_quality_answer";//ƥ�����ļ������ļ���·����ƥ���ļ������Ӧ�Ĺ켣�ļ���ͬ
string newAnswerDirectory = "15days\\15days_separated_high_quality_120s_answer";//���Ͳ����ʺ��ƥ�����ļ������ļ���·����
Map routeNetwork(rootFilePath, 500);
list<Traj*> trajList = list<Traj*>();
int sampleRate = 120;//Ҫ�����Ĳ��������DegradeInput��DegradeAnswer��������

double CalculateMAD(list<double> &dist){
	dist.sort();
	int mid = static_cast<int>(dist.size()) / 2;
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

//������켣ƽ����������
void CalculateAverageTrajPointCount(){
	int count = 0;
	for each(Traj* traj in trajList){
		count += static_cast<int>(traj->size());
	}
	count /= static_cast<int>(trajList.size());
	cout << "ƽ������������" << count << endl;
}

//����켣ƽ��������
void CalculateAverageSampleRate(){
	double totalAverageSampleRate = 0;
	for each (Traj* traj in trajList)
	{
		double averageSampleRate = (traj->back()->time - traj->front()->time) / (static_cast<double>(traj->size()) - 1);
		totalAverageSampleRate += averageSampleRate;
	}
	totalAverageSampleRate /= trajList.size();
	cout << "ƽ�������ʣ�" << totalAverageSampleRate << endl;
}

//�Ը߲����ʹ켣���ݽ���������ʣ�ÿ��sampleRateѡȡһ�������㣬�õ��µķ���sampleRate�����ʵĹ켣
//ʹ�����ַ�������ÿ��ԭ�켣�пɵ�sampleRate���Ͳ����ʵĹ켣
//inputDirectory��ԭ�켣�ļ����ڵ��ļ���·��
//newInputDirectory���¹켣�ļ����ڵ��ļ���·��
void DegradeInputFixedIntervals(){
	string completeInputFilesPath = rootFilePath + inputDirectory + "\\*.txt";
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
			ifstream fin(rootFilePath + inputDirectory + "\\" + inputFileName);
			ofstream *fout = new ofstream[sampleRate];
			for (size_t i = 0; i < sampleRate; i++)
			{
				fout[i] = ofstream(rootFilePath + newInputDirectory + "\\" + inputFileName.substr(0, 6) + ToString(index) + ToString(static_cast<int>(i)) + ".txt");
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
//��DegradeInputFixedIntervals���Ӧ���Ը߲����ʹ켣�������Ӧ��·����Ϣ���г�ȡ���õ����µĲ����ʵĹ켣��Ӧ��·������
//answerDirectory��ԭ���ļ����ڵ��ļ���·��
//newAnswerDirectory���´��ļ����ڵ��ļ���·��
void DegradeAnswerFixedIntervals(){
	string completeInputFilesPath = rootFilePath + answerDirectory + "\\*.txt";
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
			ifstream fin(rootFilePath + answerDirectory + "\\" + inputFileName);
			ofstream *fout = new ofstream[sampleRate];
			for (size_t i = 0; i < sampleRate; i++)
			{
				fout[i] = ofstream(rootFilePath + newAnswerDirectory + "\\" + inputFileName.substr(0, 7) + ToString(index) + ToString(static_cast<int>(i)) + ".txt");
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

//�Ը߲����ʹ켣���ݽ���������ʣ���ĳ���켣�����ʱ�����ѡȡ��Ĳ���ʱ��������sampleRateʱ��ѡȡ�ò�����
//ʹ�����ַ�������ÿ��ԭ�켣��ֻ�ܵõ�һ���Ͳ����ʵĹ켣
//inputDirectory��ԭ�켣�ļ����ڵ��ļ���·��
//newInputDirectory���¹켣�ļ����ڵ��ļ���·��
void DegradeInputFloatIntervals(){
	string completeInputFilesPath = rootFilePath + inputDirectory + "\\*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//�ļ���Ϣ
	long lf;//�ļ����
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		return;
	}
	else {
		do {
			string inputFileName = fileInfo.name;
			ifstream fin(rootFilePath + inputDirectory + "\\" + inputFileName);
			ofstream fout(rootFilePath + newInputDirectory + "\\" + inputFileName);
			fout.precision(13);
			int formerTimeStamp = -1;//��¼ǰһ���켣��Ĳ���ʱ�䡣����뵱ǰ�켣�����ʱ��������SampleRate������Ϊ��ǰ�켣����ϲ�����Ҫ��������µĹ켣�ļ���
			int time;
			double lat, lon;
			char useless;
			while (fin >> time){
				fin >> useless >> lat >> useless >> lon;
				if (formerTimeStamp == -1 || time - formerTimeStamp > sampleRate){
					fout << time << useless << lat << useless << lon << endl;
					formerTimeStamp = time;
				}
			}
			fout.close();
			fin.close();
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		cout << "�¹켣�ļ������ϣ�" << endl;
		return;
	}
}
//��DegradeInputFloatIntervals���Ӧ���Ը߲����ʹ켣�������Ӧ��·����Ϣ���г�ȡ���õ����µĲ����ʵĹ켣��Ӧ��·������
//answerDirectory��ԭ���ļ����ڵ��ļ���·��
//newAnswerDirectory���´��ļ����ڵ��ļ���·��
void DegradeAnswerFloatIntervals(){
	string completeInputFilesPath = rootFilePath + answerDirectory + "\\*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//�ļ���Ϣ
	long lf;//�ļ����
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		return;
	}
	else {
		do {
			string inputFileName = fileInfo.name;
			ifstream fin(rootFilePath + answerDirectory + "\\" + inputFileName);
			ofstream fout(rootFilePath + newAnswerDirectory + "\\" + inputFileName);
			int formerTimeStamp = -1;
			int time, edge;
			double confidence;
			char useless;
			while (fin >> time){
				fin >> useless >> edge >> useless >> confidence;
				if (formerTimeStamp == -1 || time - formerTimeStamp > sampleRate){
					fout << time << useless << edge << useless << confidence << endl;
					formerTimeStamp = time;
				}
			}
			fin.close();
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		cout << "��ƥ�����ļ������ϣ�" << endl;
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

int main(int argc, char*argv[]){
	//ͳ�ƹ켣�ļ�����ƽ����������
	//cout << "ͳ�ƹ켣�ļ�����ƽ����������" << endl;
	//vector<string> outputFileNames;
	//scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
	//CalculateAverageTrajPointCount();
	//system("pause");
	//return 0;

	//ͳ�ƹ켣�ļ���ƽ��������
	//cout << "ͳ�ƹ켣�ļ���ƽ��������" << endl;
	//if (argc != 1 && argc != 2){
	//	cout << "Ӧ����һ����������һ��Ϊ�켣�ļ������ļ���·����" << endl;
	//	system("pause");
	//	return 1;
	//}
	//else{
	//	if (argc == 2){
	//		inputDirectory = argv[1];
	//	}
	//	cout << "�����ļ������ļ���·����" << inputDirectory << endl;
	//	trajList = list<Traj*>();
	//	vector<string> outputFileNames;
	//	scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
	//	CalculateAverageSampleRate();
	//	system("pause");
	//	return 0;
	//}

	//���͹켣�ļ��Ĳ�����
	cout << "���͹켣�ļ��Ĳ�����" << endl;
	if (argc != 1 && argc != 2 && argc != 6){
		cout << "Ӧ��������5����������һ��ΪĿ������ʣ��ڶ�������Ϊԭ�켣�ļ����¹켣�ļ������ļ���·�������ġ����Ϊԭƥ�����ļ�����ƥ�����ļ������ļ���·��" << endl;
	}
	else{
		if (argc == 2){
			sampleRate = atoi(argv[1]);
		}
		if (argc == 6){
			sampleRate = atoi(argv[1]);
			inputDirectory = argv[2];
			newInputDirectory = argv[3];
			answerDirectory = argv[4];
			newAnswerDirectory = argv[5];
		}
		cout << "Ŀ������ʣ�" << sampleRate << endl;
		cout << "ԭ�켣�ļ������ļ��У�" << inputDirectory << endl;
		cout << "�¹켣�ļ������ļ��У�" << newInputDirectory << endl;
		cout << "ԭƥ�����ļ������ļ��У�" << answerDirectory << endl;
		cout << "��ƥ�����ļ������ļ��У�" << newAnswerDirectory << endl;
		DegradeInputFloatIntervals();
		DegradeAnswerFloatIntervals();
	}
	system("pause");
	return 0;

	//CalculateParametersForViterbiAlgorithm();

	//RawTrajToJson("2014-03-25 16_44_11.txt");

}