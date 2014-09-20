#include <iostream>
#include<sstream>
#include <math.h>
#include "ReadInTrajs.h"
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\新加坡数据\\";
string inputDirectory = "15days\\15days_separated_high_quality_input";//轨迹文件所在文件夹路径。其中包含的轨迹文件名要求：以“input_”开头
string newInputDirectory = "15days\\15days_separated_high_quality_120s_input";//降低采样率后的轨迹文件所在文件夹路径。
string answerDirectory = "15days\\15days_separated_high_quality_answer";//匹配结果文件所在文件夹路径。匹配文件名与对应的轨迹文件相同
string newAnswerDirectory = "15days\\15days_separated_high_quality_120s_answer";//降低采样率后的匹配结果文件所在文件夹路径。
Map routeNetwork(rootFilePath, 500);
list<Traj*> trajList = list<Traj*>();
int sampleRate = 120;//要降到的采样间隔，DegradeInput和DegradeAnswer函数所用

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

//计算个轨迹平均采样点数
void CalculateAverageTrajPointCount(){
	int count = 0;
	for each(Traj* traj in trajList){
		count += static_cast<int>(traj->size());
	}
	count /= static_cast<int>(trajList.size());
	cout << "平均采样点数：" << count << endl;
}

//计算轨迹平均采样率
void CalculateAverageSampleRate(){
	double totalAverageSampleRate = 0;
	for each (Traj* traj in trajList)
	{
		double averageSampleRate = (traj->back()->time - traj->front()->time) / (static_cast<double>(traj->size()) - 1);
		totalAverageSampleRate += averageSampleRate;
	}
	totalAverageSampleRate /= trajList.size();
	cout << "平均采样率：" << totalAverageSampleRate << endl;
}

//对高采样率轨迹数据降低其采样率，每个sampleRate选取一个采样点，得到新的符合sampleRate采样率的轨迹
//使用这种方法，从每条原轨迹中可得sampleRate条低采样率的轨迹
//inputDirectory：原轨迹文件所在的文件夹路径
//newInputDirectory：新轨迹文件所在的文件夹路径
void DegradeInputFixedIntervals(){
	string completeInputFilesPath = rootFilePath + inputDirectory + "\\*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//文件信息
	long lf;//文件句柄
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
//和DegradeInputFixedIntervals相对应，对高采样率轨迹数据相对应的路段信息进行抽取，得到和新的采样率的轨迹对应的路段序列
//answerDirectory：原答案文件所在的文件夹路径
//newAnswerDirectory：新答案文件所在的文件夹路径
void DegradeAnswerFixedIntervals(){
	string completeInputFilesPath = rootFilePath + answerDirectory + "\\*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//文件信息
	long lf;//文件句柄
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

//对高采样率轨迹数据降低其采样率；当某个轨迹点采样时间和已选取点的采样时间间隔超过sampleRate时，选取该采样点
//使用这种方法，从每条原轨迹中只能得到一条低采样率的轨迹
//inputDirectory：原轨迹文件所在的文件夹路径
//newInputDirectory：新轨迹文件所在的文件夹路径
void DegradeInputFloatIntervals(){
	string completeInputFilesPath = rootFilePath + inputDirectory + "\\*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//文件信息
	long lf;//文件句柄
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		return;
	}
	else {
		do {
			string inputFileName = fileInfo.name;
			ifstream fin(rootFilePath + inputDirectory + "\\" + inputFileName);
			ofstream fout(rootFilePath + newInputDirectory + "\\" + inputFileName);
			fout.precision(13);
			int formerTimeStamp = -1;//记录前一个轨迹点的采样时间。如果与当前轨迹点采样时间间隔超过SampleRate，则认为当前轨迹点符合采样率要求，输出到新的轨迹文件中
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
		cout << "新轨迹文件输出完毕！" << endl;
		return;
	}
}
//和DegradeInputFloatIntervals相对应，对高采样率轨迹数据相对应的路段信息进行抽取，得到和新的采样率的轨迹对应的路段序列
//answerDirectory：原答案文件所在的文件夹路径
//newAnswerDirectory：新答案文件所在的文件夹路径
void DegradeAnswerFloatIntervals(){
	string completeInputFilesPath = rootFilePath + answerDirectory + "\\*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//文件信息
	long lf;//文件句柄
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
		cout << "新匹配结果文件输出完毕！" << endl;
		return;
	}
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
	//统计轨迹文件集的平均采样点数
	//cout << "统计轨迹文件集的平均采样点数" << endl;
	//vector<string> outputFileNames;
	//scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
	//CalculateAverageTrajPointCount();
	//system("pause");
	//return 0;

	//统计轨迹文件的平均采样率
	//cout << "统计轨迹文件的平均采样率" << endl;
	//if (argc != 1 && argc != 2){
	//	cout << "应该有一个参数：第一个为轨迹文件所在文件夹路径！" << endl;
	//	system("pause");
	//	return 1;
	//}
	//else{
	//	if (argc == 2){
	//		inputDirectory = argv[1];
	//	}
	//	cout << "输入文件所在文件夹路径：" << inputDirectory << endl;
	//	trajList = list<Traj*>();
	//	vector<string> outputFileNames;
	//	scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
	//	CalculateAverageSampleRate();
	//	system("pause");
	//	return 0;
	//}

	//降低轨迹文件的采样率
	cout << "降低轨迹文件的采样率" << endl;
	if (argc != 1 && argc != 2 && argc != 6){
		cout << "应该至多有5个参数：第一个为目标采样率；第二、三个为原轨迹文件和新轨迹文件所在文件夹路径；第四、五个为原匹配结果文件和新匹配结果文件所在文件夹路径" << endl;
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
		cout << "目标采样率：" << sampleRate << endl;
		cout << "原轨迹文件所在文件夹：" << inputDirectory << endl;
		cout << "新轨迹文件所在文件夹：" << newInputDirectory << endl;
		cout << "原匹配结果文件所在文件夹：" << answerDirectory << endl;
		cout << "新匹配结果文件所在文件夹：" << newAnswerDirectory << endl;
		DegradeInputFloatIntervals();
		DegradeAnswerFloatIntervals();
	}
	system("pause");
	return 0;

	//CalculateParametersForViterbiAlgorithm();

	//RawTrajToJson("2014-03-25 16_44_11.txt");

}