#include "MapMatchingUsingBiasStatistic.h"
using namespace std;

//保存已计算路段间最短距离
map<pair<int, int>, pair<double, double>> shortestDistPair2 = std::map<pair<int, int>, pair<double, double>>();

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
struct Score2//代表某个轨迹点对应的一个候选路段
{
	Edge* edge;//候选路段的指针
	long double score;//候选路段所具有的整体概率
	int preColumnIndex;//候选路段的前序路段的列索引
	double distLeft;//轨迹点的投影点到候选路段起点的距离
	vector<long double> *priorProbs;
	Score2(Edge* edge, long double score, int pre, double distLeft){
		this->edge = edge;
		this->score = score;
		this->preColumnIndex = pre;
		this->distLeft = distLeft;
		priorProbs = new vector<long double>();
	}
};

//放射概率计算函数
double EmissionProb2(double t, double dist){
	return exp(t*dist * dist * N2_SIGMAZ2) * SQR_2PI_SIGMAZ;
}

//使用网格的路段偏好进行地图匹配
//对于某个轨迹点所在的网格，选择该网格中频数最大的路段作为匹配路段；如果所在网格没有任何路段频数，则计算前一匹配路段到该轨迹点附近的候选路段转移概率最大的作为该轨迹点的匹配路段
list<Edge*> MapMatchingUsingBiasStatistic(list<GeoPoint*> &trajectory){
	long double BT = 34.56991141;
	list<Edge*> matchedEdges = list<Edge*>();
	GeoPoint* formerTrajPoint = NULL;
	Edge* formerMatchedEdge = NULL;
	//int trajPointIndex = 0;//轨迹点序号，调试用
	for each (GeoPoint* trajPoint in trajectory)
	{
		//cout << "轨迹点序号：" << trajPointIndex << endl;
		//trajPointIndex++;
		Edge* matchedEdge = NULL;
		pair<int, int>gridCellIndex = litePointGridIndex.getRowCol(trajPoint);//routeNetwork.findGridCellIndex(trajPoint->lat, trajPoint->lon);
		//如果轨迹点所在网格中有历史匹配路段频数信息，则选取其中频数最大的路段作为匹配路段
		if (biasSet.find(gridCellIndex) != biasSet.end()){
			int max = 0;//记录当前最大频数
			for each (pair<Edge*, int> edgeCountPair in biasSet[gridCellIndex])
			{
				if (edgeCountPair.second > max){
					matchedEdge = edgeCountPair.first;
					max = edgeCountPair.second;
				}
			}
		}
		//如果轨迹点所在网格中没有历史匹配路段频数信息，则选择前一匹配路段到候选路段中转移概率最大的作为匹配路段
		else{
			//首先获得该轨迹点的候选路段，然后选择转移概率最大的候选路段
			vector<Edge*> canadidateEdges;//候选路段集合
			routeNetwork.getNearEdges(trajPoint->lat, trajPoint->lon, RANGEOFCANADIDATEEDGES, canadidateEdges);//获得所有在指定范围内的候选路段集合
			if (formerTrajPoint != NULL){
				long double distBetweenTwoTrajPoints = GeoPoint::distM(trajPoint->lat, trajPoint->lon, formerTrajPoint->lat, formerTrajPoint->lon);
				double formerDistLeft = 0;
				routeNetwork.distMFromTransplantFromSRC(formerTrajPoint->lat, formerTrajPoint->lon, formerMatchedEdge, formerDistLeft);
				double deltaT = trajPoint->time - formerTrajPoint->time;
				long double maxProb = 0;
				for each (Edge* canadidateEdge in canadidateEdges)
				{
					double currentDistLeft = 0;
					routeNetwork.distMFromTransplantFromSRC(trajPoint->lat, trajPoint->lon, canadidateEdge, currentDistLeft);
					double formerDistToEnd = formerMatchedEdge->lengthM - formerDistLeft;//前一个轨迹点在候选路段上的投影点距路段终点的距离
					list<Edge*> shortestPath = list<Edge*>();
					double routeNetworkDistBetweenTwoEdges = routeNetwork.shortestPathLength(formerMatchedEdge->endNodeId, canadidateEdge->startNodeId, shortestPath, currentDistLeft, formerDistToEnd, deltaT);
					long double routeNetworkDistBetweenTwoTrajPoints = routeNetworkDistBetweenTwoEdges + currentDistLeft + formerDistToEnd;
					long double transactionProb = exp(-fabs(distBetweenTwoTrajPoints - routeNetworkDistBetweenTwoTrajPoints) / BT) / BT;//转移概率
					if (transactionProb >= maxProb){
						matchedEdge = canadidateEdge;
					}
				}
			}
			else{
				long double maxProb = 0;
				for each (Edge* canadidateEdge in canadidateEdges)
				{
					double currentDistLeft = 0;
					double DistBetweenTrajPointAndEdge = routeNetwork.distMFromTransplantFromSRC(trajPoint->lat, trajPoint->lon, canadidateEdge, currentDistLeft);
					long double prob = EmissionProb2(1, DistBetweenTrajPointAndEdge);
					if (prob > maxProb){
						matchedEdge = canadidateEdge;
					}
				}
			}
		}
		matchedEdges.push_back(matchedEdge);
		formerTrajPoint = trajPoint;
		formerMatchedEdge = matchedEdge;
	}
	return matchedEdges;
}

//使用网格的路段偏好作为先验概率进行地图匹配
list<Edge*> MapMatchingUsingBiasStatisticAsPriorProb(list<GeoPoint*> &trajectory){
	list<Edge*> mapMatchingResult;//全局匹配路径
	int sampleRate = (static_cast<int>(trajectory.size()) > 1 ? (trajectory.back()->time - trajectory.front()->time) / (static_cast<int>(trajectory.size()) - 1) : (trajectory.back()->time - trajectory.front()->time));//计算轨迹平均采样率
	if (sampleRate > 30){ sampleRate = 30; }
	long double BT = (long double)BETA_ARR[sampleRate];//根据轨迹平均采样率确定beta值，计算转移概率时使用
	vector<vector<Score2>> scoreMatrix = vector<vector<Score2>>();//所有轨迹点的概率矩阵
	/*需要在每次循环结束前更新的变量*/
	GeoPoint* formerTrajPoint = NULL;//上一个轨迹点，计算路网距离时需要
	int currentTrajPointIndex = 0;//当前轨迹点的索引	
	for each(auto trajPoint in trajectory)//遍历每个轨迹点
	{
		double distBetweenTwoTrajPoints;//两轨迹点间的直接距离
		double deltaT = -1;//当前序轨迹点存在时，deltaT表示前后两轨迹点间的时间差
		if (formerTrajPoint != NULL){
			deltaT = trajPoint->time - formerTrajPoint->time;
			distBetweenTwoTrajPoints = GeoPoint::distM(trajPoint->lat, trajPoint->lon, formerTrajPoint->lat, formerTrajPoint->lon);
		}
		vector<Score2> scores = vector<Score2>();//当前轨迹点的Score2集合
		/*先验概率——放射概率——路段出现在该网格历史匹配记录中的频率或采样点与附近50米内路段距离在高斯分布上的概率*/
		long double totalCount = 0;//归一化
		pair<int, int>gridCellIndex = litePointGridIndex.getRowCol(trajPoint);//routeNetwork.findGridCellIndex(trajPoint->lat, trajPoint->lon);
		//如果轨迹点所在网格中有历史匹配路段频数信息，则把这些历史匹配路段作为候选路段；每条历史匹配路段出现的频数除以该网格中所有路段出现的总频数作为放射概率
		if (biasSet.find(gridCellIndex) != biasSet.end()){
			for each (pair<Edge*, int> edgeCountPair in biasSet[gridCellIndex])
			{
				double currentDistLeft = 0;//当前轨迹点在候选路段上的投影点距路段起点的距离
				routeNetwork.distMFromTransplantFromSRC(trajPoint->lat, trajPoint->lon, edgeCountPair.first, currentDistLeft);
				scores.push_back(Score2(edgeCountPair.first, edgeCountPair.second, -1, currentDistLeft));
				totalCount += edgeCountPair.second;
			}
		}
		//如果轨迹点所在网格中没有历史匹配路段频数信息，则把采样点与附近50米内路段距离在高斯分布上的概率作为放射概率（即HMM算法的做法）
		else
		{
			vector<Edge*> canadidateEdges = vector<Edge*>();//候选路段集合
			routeNetwork.getNearEdges(trajPoint->lat, trajPoint->lon, RANGEOFCANADIDATEEDGES, canadidateEdges);//获得所有在指定范围内的候选路段集合
			for each (Edge* canadidateEdge in canadidateEdges)
			{
				double currentDistLeft = 0;//当前轨迹点在候选路段上的投影点距路段起点的距离
				double DistBetweenTrajPointAndEdge = routeNetwork.distMFromTransplantFromSRC(trajPoint->lat, trajPoint->lon, canadidateEdge, currentDistLeft);
				//计算这些候选路段的放射概率
				long double emissionProb = EmissionProb2(1, DistBetweenTrajPointAndEdge);
				scores.push_back(Score2(canadidateEdge, emissionProb, -1, currentDistLeft));
				totalCount += emissionProb;
			}
		}
		for (size_t indexForScore2s = 0; indexForScore2s < scores.size(); indexForScore2s++){
			scores[indexForScore2s].score /= totalCount;
		}
		/*先验概率——转移概率——相邻两采样点间欧式距离与路网距离之差在指数分布上的概率*/
		if (scoreMatrix.size()>0){
			for each (Score2 canadidateEdge in scores)
			{
				for (size_t formerCanadidateEdgeIndex = 0; formerCanadidateEdgeIndex < scoreMatrix.back().size(); formerCanadidateEdgeIndex++){
					double formerDistLeft = scoreMatrix.back()[formerCanadidateEdgeIndex].distLeft;//前一个轨迹点在候选路段上的投影点距路段起点的距离
					double formerDistToEnd = scoreMatrix.back()[formerCanadidateEdgeIndex].edge->lengthM - formerDistLeft;//前一个轨迹点在候选路段上的投影点距路段终点的距离
					double routeNetworkDistBetweenTwoEdges;//两路段起点间的距离
					double routeNetworkDistBetweenTwoTrajPoints;//两轨迹点对应的投影点间的路网距离
					//如果前一匹配路段和当前候选路段是同一路段，则两者距路段起点距离的差即为路网距离
					//前一采样点（scoreMatrix.back()[formerCanadidateEdgeIndex]）、当前采样点（canadidateEdge）距路段起点距离均保存在给自的distLeft成员内
					if (canadidateEdge.edge == scoreMatrix.back()[formerCanadidateEdgeIndex].edge){
						routeNetworkDistBetweenTwoTrajPoints = fabs(canadidateEdge.distLeft - scoreMatrix.back()[formerCanadidateEdgeIndex].distLeft);
					}
					//如果前一匹配路段和当前候选路段不在同一路段，则需要使用最短路来计算两者路网距离
					else{
						pair<int, int> odPair = make_pair(scoreMatrix.back()[formerCanadidateEdgeIndex].edge->endNodeId, canadidateEdge.edge->startNodeId);
						//如果给定起点和终点的最短距离已经计算过，且不是INF，则直接从保存已计算路段间最短距离的shortestDistPair2中根据相应的路段ID对odPair取出最短距离值
						if (shortestDistPair2.find(odPair) != shortestDistPair2.end() && shortestDistPair2[odPair].first < INF){
							//如果当前deltaT下的移动距离上限比最短距离要大，调用最短路函数得到的也是保存的距离值；反之得到的就是INF
							shortestDistPair2[odPair].first <= MAXSPEED*deltaT ? routeNetworkDistBetweenTwoEdges = shortestDistPair2[odPair].first : routeNetworkDistBetweenTwoEdges = INF;
						}
						else{
							//如果给定起点和终点的最短距离是INF，且当前deltaT比上次计算最短路时的移动时间要小，说明当前deltaT下得到的最短路距离仍是INF
							if (shortestDistPair2.find(odPair) != shortestDistPair2.end() && deltaT <= shortestDistPair2[odPair].second){
								routeNetworkDistBetweenTwoEdges = INF;
							}
							/*剩余情况：
							或者给定起点和终点的最短距离从未计算过；
							或者保存的最短距离结果是INF，但当前deltaT比保存的deltaT要大（即可移动时间比已计算的长），可能得到新的最短路结果；
							因此需要调用函数计算最短路
							*/
							else{
								list<Edge*> shortestPath = list<Edge*>();
								routeNetworkDistBetweenTwoEdges = routeNetwork.shortestPathLength(scoreMatrix.back()[formerCanadidateEdgeIndex].edge->endNodeId, canadidateEdge.edge->startNodeId, shortestPath, canadidateEdge.distLeft, formerDistToEnd, deltaT);
								shortestDistPair2[odPair] = make_pair(routeNetworkDistBetweenTwoEdges, deltaT);
							}
						}
						//两采样点间的路网距离=路段间路网距离+当前采样点到候选路段起点的距离+前一采样点到对应匹配路段末尾的距离
						routeNetworkDistBetweenTwoTrajPoints = routeNetworkDistBetweenTwoEdges + canadidateEdge.distLeft + formerDistToEnd;
					}
					long double transactionProb = exp(-fabs((long double)distBetweenTwoTrajPoints - (long double)routeNetworkDistBetweenTwoTrajPoints) / BT) / BT;//转移概率
					//cout << "两点间距离 路段间距离 转移概率：" <<distBetweenTwoTrajPoints<<" "<<routeNetworkDistBetweenTwoTrajPoints<<" "<< transactionProb << endl;
					scoreMatrix.back()[formerCanadidateEdgeIndex].priorProbs->push_back(transactionProb);
				}
			}
		}
		scoreMatrix.push_back(scores);//把该轨迹点的Score2s数组放入scoreMatrix中
		formerTrajPoint = trajPoint;
		currentTrajPointIndex++;
	}
	/*得到全局匹配路径*/
	int startColumnIndex = 0;
	long double tmpMaxProb = -1;
	//得到最后一个轨迹点的在scoreMatrix对应行中得分最高的列索引，即全局匹配路径的终点
	for (size_t index = 0; index < scoreMatrix.back().size(); index++)
	{
		if (scoreMatrix.back()[index].score > tmpMaxProb){
			tmpMaxProb = scoreMatrix.back()[index].score;
			startColumnIndex = static_cast<int>(index);
		}
	}
	mapMatchingResult.push_front(scoreMatrix.back()[startColumnIndex].edge);
	int lastColumnIndex = startColumnIndex;
	for (int i = static_cast<int>(scoreMatrix.size()) - 2; i >= 0; i--){//注意：此处i类型不能设为size_t，因为size_t是无符号类型，当i==0时i--会使得i变成一个极大数
		startColumnIndex = 0;
		long double maxPosteriorProb = -1;
		for (size_t j = 0; j < scoreMatrix[i].size(); j++){
			long double tmpPosteriorProb = scoreMatrix[i][j].score * scoreMatrix[i][j].priorProbs->at(lastColumnIndex) / scoreMatrix[i + 1][lastColumnIndex].score;
			if (tmpPosteriorProb > maxPosteriorProb){
				maxPosteriorProb = tmpPosteriorProb;
				startColumnIndex = static_cast<int>(j);
			}
		}
		mapMatchingResult.push_front(scoreMatrix[i][startColumnIndex].edge);
		lastColumnIndex = startColumnIndex;
	}
	return mapMatchingResult;
}