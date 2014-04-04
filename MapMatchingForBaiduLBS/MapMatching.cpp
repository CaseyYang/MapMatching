#include <sstream>
#include "MapMatching.h"
#include "baiduMap.h"
#include "baiduTrajReader.h"
#include "MapDrawer.h"

//运行所需的全局变量
vector<string> outputFileNames;
list<Traj*> trajList;
string rootFilePath = "D:\\Document\\MDM Lab\\Data\\LBS_DATASET\\";
BaiduMap routeNetwork = BaiduMap(rootFilePath, 1000);
//保存计算过的两点间最短距离，键pair对表示起点和终点，值pair表示两点间最短距离和对应的deltaT
//保存的deltaT的原因是：如果deltaT过小，则返回的最短距离可能为INF；而当再遇到相同起点和终点、而deltaT变大时，最短距离可能就不是INF了
//类似的，当已保存的最短距离不是INF，而遇到某个更小的deltaT时，最短距离可能就是INF了
std::map<pair<int, int>, pair<double, double>> shortestDistPair = std::map<pair<int, int>, pair<double, double>>();
//ofstream logOutput;


/*为百度LBS所做的修改*/
MapDrawer md;
double minLat = 31.853;
double maxLat = 31.893;
double minLon = 117.250;
double maxLon = 117.336;
/*为百度LBS所做的修改*/


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

string ToString(int i){
	stringstream ss;
	ss << i;
	return ss.str();
}

//地图匹配所用数据结构
struct Score//代表某个轨迹点对应的一个候选路段
{
	Edge* edge;//候选路段的指针
	/*为百度LBS所做的修改*/
	double distBetweenTrajPointAndEdge;//轨迹点到候选路段的投影距离
	double emissionProb;//记录轨迹点到候选路段的放射概率
	double distBetweenTwoTrajPoints;//轨迹点和前序轨迹点的球面距离
	double routeNetworkDistBetweenTwoTrajPoints;//记录该候选路段和前序路段的路网距离
	long double transactionProb;//候选路段所具有的转移概率
	/*为百度LBS所做的修改*/
	long double score;//候选路段所具有的整体概率
	int preColumnIndex;//候选路段的前序路段的列索引
	double distLeft;//轨迹点的投影点到候选路段起点的距离
	Score(Edge* edge, long double score, int pre, double distLeft, double distBetweenTrajPointAndEdge = 0, double emissionProb = 0, double distBetweenTwoTrajPoints = 0, double routeNetworkDistBetweenTwoTrajPoints = 0, double transactionProb = 0){
		this->edge = edge;
		this->score = score;
		this->preColumnIndex = pre;
		this->distLeft = distLeft;
		/*为百度LBS所做的修改*/
		this->distBetweenTrajPointAndEdge = distBetweenTrajPointAndEdge;
		this->emissionProb = emissionProb;
		this->distBetweenTwoTrajPoints = distBetweenTwoTrajPoints;
		this->routeNetworkDistBetweenTwoTrajPoints = routeNetworkDistBetweenTwoTrajPoints;
		this->transactionProb = transactionProb;
		/*为百度LBS所做的修改*/
	}
};

/*为百度LBS所做的修改*/
struct MatchedInfo{
	GeoPoint* formerPoint;
	Edge* formerMatchedEdge;
	GeoPoint* latterPoint;
	Edge* latterMatchedEdge;
	double transactionProb;
	MatchedInfo(GeoPoint* formerPoint, GeoPoint* latterPoint, Edge* formerMatchedEdge, Edge* latterMatchedEdge, double transactionProb){
		this->formerPoint = formerPoint;
		this->latterPoint = latterPoint;
		this->formerMatchedEdge = formerMatchedEdge;
		this->latterMatchedEdge = latterMatchedEdge;
		this->transactionProb = transactionProb;
	}
};
list<MatchedInfo*> matchedInfos;
/*为百度LBS所做的修改*/

//放射概率计算函数
double EmissionProb(double t, double dist){
	return exp(t*dist * dist * N2_SIGMAZ2) * SQR_2PI_SIGMAZ;
}

//辅助函数：给定行索引，计算scoreMatrix中该行中整体概率最大的候选路段的索引
int GetStartColumnIndex(vector<Score> &row){
	int resultIndex = -1;
	long double currentMaxProb = -1;
	for (size_t i = 0; i < row.size(); i++){
		if (currentMaxProb < row.at(i).score){
			currentMaxProb = row.at(i).score;
			resultIndex = i;
		}
	}
	return resultIndex;
}

//若相邻匹配路段不连接，则在两个路段间插入其最短路径通过的路段
//list<Edge*> linkMatchedResult(list<Edge*> &mapMatchingResult){
//	list<Edge*> result = list<Edge*>();
//	Edge* lastEdge = NULL;
//	for (list<Edge*>::iterator iter = mapMatchingResult.begin(); iter != mapMatchingResult.end(); iter++)
//	{
//		if (lastEdge == NULL){
//			lastEdge = *iter;
//			result.push_back(*iter);
//			continue;
//		}
//		else{
//			if ((*iter) == NULL){
//				continue;
//			}
//			else{
//				if (lastEdge != *iter&&lastEdge->endNodeId != (*iter)->startNodeId){
//					routeNetwork.shortestPathLength(lastEdge->endNodeId, (*iter)->startNodeId);//TODO：两版本shortestpath参数不同
//					result.insert(result.end(), shortestPath.begin(), shortestPath.end());
//					result.push_back(*iter);
//					lastEdge = *iter;
//				}
//			}
//		}
//	}
//	return result;
//}

void DrawLowTransationProb(double rate){
	cout << "size：" << matchedInfos.size() << endl;
	matchedInfos.sort([&](const MatchedInfo* m1, const MatchedInfo* m2){
		return (m1->transactionProb) < (m2->transactionProb);
	});
	size_t index = 0;
	for each (MatchedInfo* var in matchedInfos)
	{
		if (index < matchedInfos.size()*rate){
			md.drawBigPoint(Color::Green, var->formerPoint->lat, var->formerPoint->lon);
			md.drawBigPoint(Color::Red, var->latterPoint->lat, var->latterPoint->lon);
			md.drawLine(Color::Red, var->formerPoint->lat, var->formerPoint->lon, var->latterPoint->lat, var->latterPoint->lon);
		}
		index++;
	}
}

list<Edge*> MapMatching(list<GeoPoint*> &trajectory){
	list<Edge*> mapMatchingResult;//全局匹配路径
	int sampleRate = (trajectory.back()->time - trajectory.front()->time) / (trajectory.size() - 1);//计算轨迹平均采样率
	//cout << "采样率：" << sampleRate << endl;
	if (sampleRate > 30){ sampleRate = 30; }
	long double BT = (long double)BETA_ARR[sampleRate];//根据轨迹平均采样率确定beta值，计算转移概率时使用
	vector<vector<Score>> scoreMatrix = vector<vector<Score>>();//所有轨迹点的概率矩阵
	//需要在每次循环结束前更新的变量
	GeoPoint* formerTrajPoint = NULL;//上一个轨迹点，计算路网距离时需要
	bool cutFlag = true;//没有前一轨迹点或前一轨迹点没有达标的候选路段
	int currentTrajPointIndex = 0;//当前轨迹点的索引	
	for (list<GeoPoint*>::iterator trajectoryIterator = trajectory.begin(); trajectoryIterator != trajectory.end(); trajectoryIterator++)//遍历每个轨迹点
	{
		double deltaT = -1;//当前序轨迹点存在时，deltaT表示前后两轨迹点间的时间差
		if (formerTrajPoint != NULL){ deltaT = (*trajectoryIterator)->time - formerTrajPoint->time; }
		long double currentMaxProb = -1e10;//当前最大整体概率，初始值为-1e10
		vector<Score> scores = vector<Score>();//当前轨迹点的Score集合
		vector<Edge*> canadidateEdges;//候选路段集合
		routeNetwork.getNearEdges((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, RANGEOFCANADIDATEEDGES, canadidateEdges);//获得所有在指定范围内的候选路段集合
		long double *emissionProbs = new long double[canadidateEdges.size()];//保存候选路段的放射概率
		int currentCanadidateEdgeIndex = 0;//当前候选路段的索引
		for each (Edge* canadidateEdge in canadidateEdges)
		{
			/*为百度LBS所做的修改*/
			double distBetweenTrajPointAndEdgeForBaiduLBS = -1;
			double emissionProbForBaiduLBS = -1;
			double distBetweenTrajPointsForBaiduLBS = -1;
			double distBetweenEdgeForBaiduLBS = -1;
			long double transactionProbForBaiduLBS = -1;
			/*为百度LBS所做的修改*/
			int preColumnIndex = -1;//保存当前候选路段的前序路段的列索引
			double currentDistLeft = 0;//当前轨迹点在候选路段上的投影点距路段起点的距离
			double distBetweenTrajPointAndEdge = routeNetwork.distMFromTransplantFromSRC((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, canadidateEdge, currentDistLeft);
			//计算这些候选路段的放射概率
			emissionProbs[currentCanadidateEdgeIndex] = EmissionProb(1, distBetweenTrajPointAndEdge);
			/*为百度LBS所做的修改*/
			distBetweenTrajPointAndEdgeForBaiduLBS = distBetweenTrajPointAndEdge;
			emissionProbForBaiduLBS = emissionProbs[currentCanadidateEdgeIndex];
			/*为百度LBS所做的修改*/
			if (!cutFlag){
				//当前采样点不是轨迹第一个点或匹配中断后的第一个点，则计算转移概率
				long double currentMaxProbTmp = -1e10;//当前最大转移概率，初始值为-1e10
				int formerCanadidateEdgeIndex = 0;
				double distBetweenTwoTrajPoints = GeoPoint::distM((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, formerTrajPoint->lat, formerTrajPoint->lon);//两轨迹点间的直接距离
				/*为百度LBS所做的修改*/
				distBetweenTrajPointsForBaiduLBS = distBetweenTwoTrajPoints;
				/*为百度LBS所做的修改*/
				for each(Score formerCanadidateEdge in scoreMatrix.back()){
					double formerDistLeft = formerCanadidateEdge.distLeft;//前一个轨迹点在候选路段上的投影点距路段起点的距离
					double formerDistToEnd = formerCanadidateEdge.edge->lengthM - formerDistLeft;//前一个轨迹点在候选路段上的投影点距路段终点的距离
					double routeNetworkDistBetweenTwoEdges;//两路段起点间的距离
					double routeNetworkDistBetweenTwoTrajPoints;//两轨迹点对应的投影点间的路网距离
					if (canadidateEdge == formerCanadidateEdge.edge){//如果前一匹配路段和当前候选路段是同一路段，则两者计算距离起点的差即为路网距离
						routeNetworkDistBetweenTwoTrajPoints = fabs(currentDistLeft - formerCanadidateEdge.distLeft);
					}
					else{
						pair<int, int> odPair = make_pair(formerCanadidateEdge.edge->endNodeId, canadidateEdge->startNodeId);
						//给定起点和终点最短路已经计算过，且不是INF
						if (shortestDistPair.find(odPair) != shortestDistPair.end() && shortestDistPair[odPair].first < INF){
							//如果当前deltaT下的移动距离上限比最短距离要大，调用最短路函数得到的也是保存的距离值；反之得到的就是INF
							shortestDistPair[odPair].first <= MAXSPEED*deltaT ? routeNetworkDistBetweenTwoEdges = shortestDistPair[odPair].first : routeNetworkDistBetweenTwoEdges = INF;
						}
						else{
							if (shortestDistPair.find(odPair) != shortestDistPair.end() && deltaT <= shortestDistPair[odPair].second){//保存的给定起点和终点最短路结果是INF，且当前deltaT比上次计算最短路时的移动时间要小，说明当前deltaT下得到的最短路距离仍是INF
								routeNetworkDistBetweenTwoEdges = INF;
							}
							else{
								//或者未保存给定起点和终点的最短路结果；或者当前deltaT比保存的deltaT要大，可能得到真正的最短路结果；总之就是要调用函数计算最短路
								list<Edge*> shortestPath = list<Edge*>();
								routeNetworkDistBetweenTwoEdges = routeNetwork.shortestPathLength(formerCanadidateEdge.edge->endNodeId, canadidateEdge->startNodeId, shortestPath, currentDistLeft, formerDistToEnd, deltaT);//TODO
								shortestDistPair[odPair] = make_pair(routeNetworkDistBetweenTwoEdges, deltaT);
							}
						}
						routeNetworkDistBetweenTwoTrajPoints = routeNetworkDistBetweenTwoEdges + currentDistLeft + formerDistToEnd;
					}
					long double transactionProb = exp(-fabs((long double)distBetweenTwoTrajPoints - (long double)routeNetworkDistBetweenTwoTrajPoints) / BT) / BT;//转移概率
					/*GIS2012CUP的优化加在此处，对transactionProb进行修改*/
					long double tmpTotalProbForTransaction = formerCanadidateEdge.score * transactionProb;
					if (currentMaxProbTmp < tmpTotalProbForTransaction){//保留当前转移概率和已知最大转移概率中较大者
						currentMaxProbTmp = tmpTotalProbForTransaction;
						preColumnIndex = formerCanadidateEdgeIndex;
						/*为百度LBS所做的修改*/
						distBetweenEdgeForBaiduLBS = routeNetworkDistBetweenTwoTrajPoints;
						transactionProbForBaiduLBS = transactionProb;
						/*为百度LBS所做的修改*/
					}
					formerCanadidateEdgeIndex++;
				}
				//此时，emissionProbs保存的是候选路段的放射概率，乘以转移概率则变为候选路段的整体概率
				emissionProbs[currentCanadidateEdgeIndex] *= currentMaxProbTmp;
			}
			/*若需要提升代码运行速度，则只把整体概率大于MINPROB的候选路段放入当前轨迹点的Score集合中；否则把所有候选路段放入Score集合中*/
			scores.push_back(Score(canadidateEdge, emissionProbs[currentCanadidateEdgeIndex], preColumnIndex, currentDistLeft, distBetweenTrajPointsForBaiduLBS, distBetweenEdgeForBaiduLBS, transactionProbForBaiduLBS));
			//得到当前最大整体概率，以便归一化
			if (currentMaxProb < emissionProbs[currentCanadidateEdgeIndex]){ currentMaxProb = emissionProbs[currentCanadidateEdgeIndex]; }
			currentCanadidateEdgeIndex++;
		}
		delete[]emissionProbs;
		formerTrajPoint = *trajectoryIterator;
		currentTrajPointIndex++;
		for (size_t i = 0; i < scores.size(); i++)	{ scores[i].score /= currentMaxProb; }//归一化
		scoreMatrix.push_back(scores);//把该轨迹点的Scores数组放入scoreMatrix中
		if (scores.size() == 0){//若scores数组为空，则说明没有一个达标的候选路段，cutFlag设为true，后续轨迹作为新轨迹进行匹配
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
	list<GeoPoint*>::reverse_iterator rIter = trajectory.rbegin();
	/*为百度LBS所做的修改*/
	//ofstream foutForBaiduLBS = ofstream("prob"+ToString(currentTrajPointIndex)+".txt");
	list<GeoPoint*>::reverse_iterator tmpRIter;
	/*为百度LBS所做的修改*/
	for (int i = scoreMatrix.size() - 1; i >= 0; i--, rIter++){
		if (startColumnIndex != -1){
			/*为百度LBS所做的修改*/
			//输出格式：前序edgeId 当前edgeId 前序采样点纬度 前序采样点经度 当前采样点纬度 当前采样点经度 转移概率
			list<GeoPoint*>::reverse_iterator tmpRIter = rIter;
			tmpRIter++;
			if (tmpRIter != trajectory.rend()){
				if (scoreMatrix[i][startColumnIndex].preColumnIndex != -1){//preColumnIndex不为-1，表示上一个采样点候选路段到当前采样点候选路段是有转移概率的
					MatchedInfo *info = new MatchedInfo(*tmpRIter, *rIter, scoreMatrix[i - 1][scoreMatrix[i][startColumnIndex].preColumnIndex].edge, scoreMatrix[i][startColumnIndex].edge, scoreMatrix[i][startColumnIndex].transactionProb);
					matchedInfos.push_front(info);
					//foutForBaiduLBS << scoreMatrix[i][startColumnIndex].preColumnIndex << " " << scoreMatrix[i][startColumnIndex].edge->id << " " << (*tmpRIter)->lat << " " << (*tmpRIter)->lon << " " << (*rIter)->lat << " " << (*rIter)->lon << " " << scoreMatrix[i][startColumnIndex].transactionProb << endl;
				}
			}
			/*为百度LBS所做的修改*/

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
	/*为百度LBS所做的修改*/
	//foutForBaiduLBS.close();
	/*为百度LBS所做的修改*/
	//调试代码：输出最终的概率矩阵：如果有某个轨迹点的所有候选路段的整体概率均为均为无穷小/无穷大，那可能就不正常，需要进一步检查该行概率的得到过程
	//for (int i = 0; i < scoreMatrix.size(); i++){
	//	logOutput << scoreMatrix.at(i).size() << "\t";
	//	for (int j = 0; j < scoreMatrix.at(i).size(); j++){
	//		logOutput << "[" << scoreMatrix.at(i).at(j).edge->id << "][" << scoreMatrix.at(i).at(j).preColumnIndex << "][" << scoreMatrix.at(i).at(j).score << "]\t";
	//	}
	//	logOutput << endl;
	//}
	//调试结束

	return mapMatchingResult;
	//return linkMatchedResult(mapMatchingResult);
}

void main(){
	//logOutput = ofstream("debug.txt");
	//logOutput.setf(ios::showpoint);
	//logOutput.precision(8);
	md.setArea(minLat, maxLat, minLon, maxLon);
	md.setResolution(6000);
	routeNetwork.setArea(md);
	BaiduTrajReader fReader(rootFilePath+"traj_hefei.txt");
	fReader.readTrajs(trajList, 1000);
	int trajIndex = 0;
	md.newBitmap();
	md.lockBits();
	routeNetwork.drawMap(Color::Blue, md);
	cout << "开始地图匹配！" << endl;
	for (list<Traj*>::iterator trajIter = trajList.begin(); trajIter != trajList.end(); trajIter++){
		list<Edge*> resultList = MapMatching(*(*trajIter));
		//ofstream MatchedEdgeOutput(rootFilePath + "output\\" + outputFileNames[trajIndex]);
		//Traj::iterator trajPointIter = (*trajIter)->begin();
		//for (list<Edge*>::iterator edgeIter = resultList.begin(); edgeIter != resultList.end(); edgeIter++, trajPointIter++){
		//	if (*edgeIter != NULL){
		//		int currentIndex = (*edgeIter)->id;
		//		//MatchedEdgeOutput << (*trajPointIter)->time << "," << currentIndex << ",1.0" << endl;
		//		MatchedEdgeOutput << "0," << currentIndex << ",1.0" << endl;
		//	}
		//	else{
		//		//MatchedEdgeOutput << (*trajPointIter)->time << "," << -1 << ",1.0" << endl;
		//		MatchedEdgeOutput << "0," << -1 << ",1.0" << endl;
		//	}
		//}
		//MatchedEdgeOutput.close();
		//cout << "第" << trajIndex << "条轨迹匹配完毕！" << endl;
		//trajIndex++;
	}
	DrawLowTransationProb(0.01);
	md.unlockBits();
	md.saveBitmap("pic.png");
	//logOutput.close();
}