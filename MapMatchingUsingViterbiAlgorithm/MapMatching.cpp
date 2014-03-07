#include "MapMatching.h"
#include "ReadInTrajs.h"

//运行所需的全局变量
vector<string> outputFileNames;
list<Traj*> trajList;
string rootFilePath = "D:\\Document\\Subjects\\Computer\\Develop\\Data\\SingaporeData\\";
Map routeNetwork = Map(rootFilePath, 500);
std::map<pair<int, int>, double> shortestDistPair = std::map<pair<int, int>, double>();//保存计算过的两点间最短距离，键pair对表示起点和终点，值表示两点间最短距离
//ofstream logOutput;

//在不同采样率（1~30s/点）下各种beta数值，打表得出= =
const double BETA_ARR[31] = {
	0,
	0.490376731, 0.82918373, 1.24364564, 1.67079581, 2.00719298,
	2.42513007, 2.81248831, 3.15745473, 3.52645392, 4.09511775,
	4.67319795, 5.41088180, 6.47666590, 6.29010734, 7.80752112,
	8.09074504, 8.08550528, 9.09405065, 11.09090603, 11.87752824,
	12.55107715, 15.82820829, 17.69496773, 18.07655652, 19.63438911,
	25.40832185, 23.76001877, 28.43289797, 32.21683062, 34.56991141
};

//地图匹配所用数据结构
struct Score//代表某个轨迹点对应的一个候选路段
{
	Edge* edge;//候选路段的指针
	long double score;//候选路段所具有的整体概率
	int preColumnIndex;//候选路段的前序路段的列索引
	double distLeft;//轨迹点的投影点到候选路段起点的距离
	Score(Edge* edge, long double score, int pre, double distLeft){
		this->edge = edge;
		this->score = score;
		this->preColumnIndex = pre;
		this->distLeft = distLeft;
	}
};

//放射概率计算函数by Wang Yin：
//放射概率：使用轨迹点到候选路段的距离在高斯分布上的概率
//参数t：为Wang Yin算法而设，表示前后轨迹点间的时间差
//参数dist：轨迹点到候选路段的距离
double EmissionProb(double t, double dist){
	return t*sqrt(dist)*COEFFICIENT_FOR_EMISSIONPROB;
}
//放射概率计算函数by宋壬初：
//double EmissionProb(double t, double dist){
//	return exp(t*dist * dist * N2_SIGMAZ2) * SQR_2PI_SIGMAZ;
//}

//辅助函数：给定行索引，计算scoreMatrix中该行中整体概率最大的候选路段的索引
//by Wang Yin：
int GetStartColumnIndex(vector<Score> &row){
	int resultIndex = -1;
	long double currentMaxProb = 1e10;
	for (int i = 0; i < row.size(); i++){
		if (currentMaxProb > row.at(i).score){
			currentMaxProb = row.at(i).score;
			resultIndex = i;
		}
	}
	return resultIndex;
}
////by宋壬初
//int GetStartColumnIndex(vector<Score> &row){
//	int resultIndex = -1;
//	long double currentMaxProb = -1;
//	for (int i = 0; i < row.size(); i++){
//		if (currentMaxProb < row.at(i).score){
//			currentMaxProb = row.at(i).score;
//			resultIndex = i;
//		}
//	}
//	return resultIndex;
//}

//若相邻匹配路段不连接，则在两个路段间插入其最短路径通过的路段
list<Edge*> linkMatchedResult(list<Edge*> &mapMatchingResult){
	list<Edge*> result = list<Edge*>();
	Edge* lastEdge = NULL;
	for (list<Edge*>::iterator iter = mapMatchingResult.begin(); iter != mapMatchingResult.end(); iter++)
	{
		if (lastEdge == NULL){
			lastEdge = *iter;
			result.push_back(*iter);
			continue;
		}
		else{
			if ((*iter) == NULL){
				continue;
			}
			else{
				if (lastEdge != *iter&&lastEdge->endNodeId != (*iter)->startNodeId){
					list<Edge*> shortestPath = list<Edge*>();
					routeNetwork.shortestPathLength(lastEdge->endNodeId, (*iter)->startNodeId, shortestPath);
					result.insert(result.end(), shortestPath.begin(), shortestPath.end());
					result.push_back(*iter);
					lastEdge = *iter;
				}
			}
		}
	}
	return result;
}

list<Edge*> MapMatching(list<GeoPoint*> &trajectory){
	list<Edge*> mapMatchingResult;//全局匹配路径
	//beta值by宋壬初
	//int sampleRate = (trajectory.back()->time - trajectory.front()->time) / (trajectory.size() - 1);
	//cout << "采样率：" << sampleRate << endl;
	//if (sampleRate > 30){
	//	sampleRate = 30;
	//}
	//long double BT = (long double)BETA_ARR[sampleRate];//当前轨迹所采用的beta值，计算转移概率时使用
	vector<vector<Score>> scoreMatrix = vector<vector<Score>>();//所有轨迹点的概率矩阵
	//需要在每次循环结束前更新的变量
	GeoPoint* formerTrajPoint = NULL;//上一个轨迹点，计算路网距离时需要
	bool cutFlag = true;//没有前一轨迹点或前一轨迹点没有达标的候选路段
	int currentTrajPointIndex = 0;//当前轨迹点的索引
	//遍历每个轨迹点
	list<GeoPoint*>::iterator trajectoryIterator = trajectory.begin();
	for (; trajectoryIterator != trajectory.end(); trajectoryIterator++)
	{
		double deltaT = -1;//当前序轨迹点存在时，delaT表示前后两轨迹点间的时间差
		if (formerTrajPoint != NULL){
			deltaT = (*trajectoryIterator)->time - formerTrajPoint->time;
		}
		//整体概率初始值by Wang Yin：
		long double currentMaxProb = 1e10;//当前最大概率
		//整体概率初始值by宋壬初：
		//long double currentMaxProb = -1e10;//当前最大概率
		vector<Score> scores = vector<Score>();//当前轨迹点的Score集合
		vector<Edge*> canadidateEdges;
		routeNetwork.getNearEdges((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, RANGEOFCANADIDATEEDGES, canadidateEdges);//获得所有在指定范围内的候选路段集合
		long double *emissionProbs = new long double[canadidateEdges.size()];//保存这些候选路段的放射概率
		int currentCanadidateEdgeIndex = 0;//当前候选路段的索引
		//cout << "进入循环前" << endl;
		//system("pause");
		//for (vector<Edge*>::iterator canadidateEdgeIter = canadidateEdges.begin(); canadidateEdgeIter != canadidateEdges.end(); canadidateEdgeIter++)
		for each (Edge* canadidateEdge in canadidateEdges)
		{
			int preColumnIndex = -1;//保存当前候选路段的前序路段的列索引
			double currentDistLeft = 0;//当前轨迹点在候选路段上的投影点距路段起点的距离
			double DistBetweenTrajPointAndEdge = routeNetwork.distM((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, canadidateEdge, currentDistLeft);
			//计算这些候选路段的放射概率
			if (cutFlag){
				//基于隐马尔科夫模型的计算当前轨迹点对应每个候选路段的放射概率
				//Wang Yin的算法中使用同样方法计算轨迹起点的放射概率
				emissionProbs[currentCanadidateEdgeIndex] = EmissionProb(1, DistBetweenTrajPointAndEdge);
			}
			else{
				//计算非轨迹起点的放射概率by Wang Yin：
				emissionProbs[currentCanadidateEdgeIndex] = EmissionProb(deltaT, DistBetweenTrajPointAndEdge);
				//计算非轨迹起点的放射概率by宋壬初：
				//emissionProbs[currentCanadidateEdgeIndex] = EmissionProb(1, DistBetweenTrajPointAndEdge);
				//转移概率初始值by Wang Yin：
				long double currentMaxProbTmp = 1e10;
				//cout << "进入小循环前" << endl;
				//system("pause");
				//转移概率初始值by宋壬初：
				//long double currentMaxProbTmp = -1e10;
				for (int i = 0; i < scoreMatrix.back().size(); i++)
				{
					double formerDistLeft = scoreMatrix[currentTrajPointIndex - 1][i].distLeft;//前一个轨迹点在候选路段上的投影点距路段起点的距离
					double routeNetworkDistBetweenTwoEdges;//两路段起点间的距离
					double routeNetworkDistBetweenTwoTrajPoints;//两轨迹点对应的投影点间的路网距离
					if (canadidateEdge == scoreMatrix[currentTrajPointIndex - 1][i].edge){
						routeNetworkDistBetweenTwoTrajPoints = currentDistLeft - scoreMatrix[currentTrajPointIndex - 1][i].distLeft;
					}
					else
					{
						pair<int, int> odPair = make_pair(scoreMatrix[currentTrajPointIndex - 1][i].edge->startNodeId, canadidateEdge->startNodeId);
						if (shortestDistPair.find(odPair) != shortestDistPair.end()){
							routeNetworkDistBetweenTwoEdges = shortestDistPair[odPair];
						}
						else{
							list<Edge*> shortestPath = list<Edge*>();
							routeNetworkDistBetweenTwoEdges = routeNetwork.shortestPathLength(scoreMatrix[currentTrajPointIndex - 1][i].edge->startNodeId, canadidateEdge->startNodeId, shortestPath, currentDistLeft, formerDistLeft, deltaT);
							shortestDistPair[odPair] = routeNetworkDistBetweenTwoEdges;
						}
						routeNetworkDistBetweenTwoTrajPoints = routeNetworkDistBetweenTwoEdges + currentDistLeft - formerDistLeft;
					}
					//两轨迹点间的直接距离
					double distBetweenTwoTrajPoints = GeoPoint::distM((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, formerTrajPoint->lat, (*trajectoryIterator)->lon);
					//double transactionProb = exp(-fabs((long double)routeNetworkDistBetweenTwoTrajPoints - (long double)distBetweenTwoTrajPoints) / BT) / BT;
					//计算转移概率by Wang Yin：
					long double transactionProb = (long double)routeNetworkDistBetweenTwoTrajPoints*COEFFICIENT_FOR_TRANSATIONPROB;
					//计算转移概率by宋壬初：
					//long double transactionProb = exp(-fabs((long double)distBetweenTwoTrajPoints - (long double)routeNetworkDistBetweenTwoTrajPoints) / BT) / BT;
					/*GIS2012CUP的优化加在此处，对transactionProb进行修改*/
					long double tmpTotalProbForTransaction = scoreMatrix[currentTrajPointIndex - 1][i].score * transactionProb;
					//调试代码：输出指定轨迹点整体概率的各个组成部分，若有某个值特别小/大，则可能就是这个值
					//if (currentTrajPointIndex == 638){
					//	cout << "前序整体概率为：" << scoreMatrix[currentTrajPointIndex - 1][i].score << endl;
					//	cout << "当前转移概率为：" << transactionProb << endl;
					//	cout << "当前放射概率为：" << emissionProbs[currentCanadidateEdgeIndex] << endl;
					//	cout << "当前整体概率为：" << tmpTotalProbForTransaction << endl;
					//	system("pause");
					//}
					//调试结束
					//计算转移概率by Wang Yin：
					//用currentMaxProbTmp保存所有前序候选路段中转移概率最大者
					if (currentMaxProbTmp > tmpTotalProbForTransaction){
						currentMaxProbTmp = tmpTotalProbForTransaction;
						preColumnIndex = i;
					}
					//计算转移概率by宋壬初：
					//if (currentMaxProbTmp < tmpTotalProbForTransaction){
					//	currentMaxProbTmp = tmpTotalProbForTransaction;
					//	preColumnIndex = i;
					//}
				}
				//此时，emissionProbs保存的已经是某个候选路段的整体概率了
				//计算整体概率by Wang Yin：
				emissionProbs[currentCanadidateEdgeIndex] += currentMaxProbTmp;
				//计算整体概率by宋壬初：
				//emissionProbs[currentCanadidateEdgeIndex] *= currentMaxProbTmp;
			}
			//若需要提升代码运行速度，则只把整体概率大于MINPROB的候选路段放入当前轨迹点的Score集合中；
			//否则把所有候选路段放入Score集合中
			scores.push_back(Score(canadidateEdge, emissionProbs[currentCanadidateEdgeIndex], preColumnIndex, currentDistLeft));
			//得到当前最大整体概率by Wang Yin：
			if (currentMaxProb > emissionProbs[currentCanadidateEdgeIndex]){
				currentMaxProb = emissionProbs[currentCanadidateEdgeIndex];
			}
			//得到当前最大整体概率by宋壬初：
			//if (currentMaxProb < emissionProbs[currentCanadidateEdgeIndex]){
			//	currentMaxProb = emissionProbs[currentCanadidateEdgeIndex];
			//}
			currentCanadidateEdgeIndex++;
		}
		delete[]emissionProbs;
		formerTrajPoint = (*trajectoryIterator);
		currentTrajPointIndex++;
		//system("pause");
		//归一化
		for (int i = 0; i < scores.size(); i++)
		{
			scores[i].score /= currentMaxProb;
		}
		//把该轨迹点的Scores数组放入scoreMatrix中
		scoreMatrix.push_back(scores);
		//若scores数组为空，则说明没有一个达标的候选路段，cutFlag设为true，后续轨迹作为新轨迹进行匹配
		if (scores.size() == 0){
			cutFlag = true;
			formerTrajPoint = NULL;
		}
		else
		{
			cutFlag = false;
		}
	}

	//得到全局匹配路径
	int startColumnIndex = GetStartColumnIndex(scoreMatrix.back());//得到最后一个轨迹点的在scoreMatrix对应行中得分最高的列索引，即全局匹配路径的终点
	for (int i = scoreMatrix.size() - 1; i >= 0; i--){
		if (startColumnIndex != -1){
			mapMatchingResult.push_front(scoreMatrix[i][startColumnIndex].edge);
			startColumnIndex = scoreMatrix[i][startColumnIndex].preColumnIndex;
		}
		else
		{
			mapMatchingResult.push_front(NULL);
			if (i > 0){
				startColumnIndex = GetStartColumnIndex(scoreMatrix[i - 1]);
			}
		}
	}
	//调试代码：输出最终的概率矩阵：如果有某个轨迹点的所有候选路段的整体概率均为均为无穷小/无穷大，那可能就不正常，需要进一步检查该行概率的得到过程
	//for (int i = 0; i < scoreMatrix.size(); i++){
	//	logOutput << scoreMatrix.at(i).size() << "\t";
	//	for (int j = 0; j < scoreMatrix.at(i).size(); j++){
	//		logOutput << scoreMatrix.at(i).at(j).edge->id << "$" << scoreMatrix.at(i).at(j).preColumnIndex << "\t";
	//	}
	//	logOutput << endl;
	//}
	//调试结束

	return mapMatchingResult;
	//return linkMatchedResult(mapMatchingResult);
}

void main(){
	//logOutput = ofstream("debug2.txt");
	scanTrajFolder(rootFilePath, trajList, outputFileNames);
	int trajIndex = 0;
	cout << "开始地图匹配！" << endl;
	for (list<Traj*>::iterator trajIter = trajList.begin(); trajIter != trajList.end(); trajIter++){
		list<Edge*> resultList = MapMatching(*(*trajIter));
		ofstream MatchedEdgeOutput(rootFilePath + "test_output\\" + outputFileNames[trajIndex]);
		Traj::iterator trajPointIter = (*trajIter)->begin();
		for (list<Edge*>::iterator edgeIter = resultList.begin(); edgeIter != resultList.end(); edgeIter++, trajPointIter++){
			if (*edgeIter != NULL){
				int currentIndex = (*edgeIter)->id;
				//MatchedEdgeOutput << (*trajPointIter)->time << "," << currentIndex << ",1.0" << endl;
				MatchedEdgeOutput << "86400," << currentIndex << ",1.0" << endl;
			}
			else{
				//MatchedEdgeOutput << (*trajPointIter)->time << "," << -1 << ",1.0" << endl;
				MatchedEdgeOutput << "86400," << -1 << ",1.0" << endl;
			}
		}
		MatchedEdgeOutput.close();
		cout << "第" << trajIndex << "条轨迹匹配完毕！" << endl;
		trajIndex++;
	}
	//logOutput.close();
}