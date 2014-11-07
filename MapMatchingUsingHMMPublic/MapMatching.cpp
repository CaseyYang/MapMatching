#include "MapMatching.h"
#include "ReadInTrajs.h"

vector<string> outputFileNames;
list<Traj*> trajList;
string rootFilePath = "D:\\MapMatchingProject\\Data\\新加坡数据\\";
string inputDirectory = "15days\\15days_separated_high_quality_120s_input";
string outputDirectory = "15days\\15days_separated_high_quality_120s_output_HMM";
Map routeNetwork = Map(rootFilePath, 1000);
std::map<pair<int, int>, pair<double, double>> shortestDistPair = std::map<pair<int, int>, pair<double, double>>();

const double BETA_ARR[31] = {
	0,
	0.490376731, 0.82918373, 1.24364564, 1.67079581, 2.00719298,
	2.42513007, 2.81248831, 3.15745473, 3.52645392, 4.09511775,
	4.67319795, 5.41088180, 6.47666590, 6.29010734, 7.80752112,
	8.09074504, 8.08550528, 9.09405065, 11.09090603, 11.87752824,
	12.55107715, 15.82820829, 17.69496773, 18.07655652, 19.63438911,
	25.40832185, 23.76001877, 28.43289797, 32.21683062, 34.56991141
};


struct Score
{
	Edge* edge;
	long double score;
	int preColumnIndex;
	double distLeft;
	Score(Edge* edge, long double score, int pre, double distLeft){
		this->edge = edge;
		this->score = score;
		this->preColumnIndex = pre;
		this->distLeft = distLeft;
	}
};
double EmissionProb(double t, double dist){
	return exp(t*dist * dist * N2_SIGMAZ2) * SQR_2PI_SIGMAZ;
}
int GetStartColumnIndex(vector<Score> &row){
	int resultIndex = -1;
	long double currentMaxProb = -1;
	for (size_t i = 0; i < row.size(); ++i){
		if (currentMaxProb < row.at(i).score){
			currentMaxProb = row.at(i).score;
			resultIndex = static_cast<int>(i);
		}
	}
	return resultIndex;
}
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
	list<Edge*> mapMatchingResult;
	int sampleRate = (static_cast<int>(trajectory.size()) > 1 ? (trajectory.back()->time - trajectory.front()->time) / (static_cast<int>(trajectory.size()) - 1) : (trajectory.back()->time - trajectory.front()->time));
	if (sampleRate > 30){ sampleRate = 30; }
	long double BT = (long double)BETA_ARR[sampleRate];
	vector<vector<Score>> scoreMatrix = vector<vector<Score>>();
	GeoPoint* formerTrajPoint = NULL;
	bool cutFlag = true;
	int currentTrajPointIndex = 0;	
	for (list<GeoPoint*>::iterator trajectoryIterator = trajectory.begin(); trajectoryIterator != trajectory.end(); trajectoryIterator++)
	{
		double distBetweenTwoTrajPoints;
		double deltaT = -1;
		if (formerTrajPoint != NULL){
			deltaT = (*trajectoryIterator)->time - formerTrajPoint->time;
			distBetweenTwoTrajPoints = GeoPoint::distM((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, formerTrajPoint->lat, formerTrajPoint->lon);
		}
		long double currentMaxProb = -1e10;
		vector<Score> scores = vector<Score>();
		vector<Edge*> canadidateEdges;
		routeNetwork.getNearEdges((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, RANGEOFCANADIDATEEDGES, canadidateEdges);
		long double *emissionProbs = new long double[canadidateEdges.size()];
		int currentCanadidateEdgeIndex = 0;
		for each (Edge* canadidateEdge in canadidateEdges)
		{
			int preColumnIndex = -1;
			double currentDistLeft = 0;
			double DistBetweenTrajPointAndEdge = routeNetwork.distMFromTransplantFromSRC((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, canadidateEdge, currentDistLeft);
			emissionProbs[currentCanadidateEdgeIndex] = EmissionProb(1, DistBetweenTrajPointAndEdge);
			if (!cutFlag){
				long double currentMaxProbTmp = -1e10;			
				int formerCanadidateEdgeIndex = 0;
				for each(Score formerCanadidateEdge in scoreMatrix.back()){
					double formerDistLeft = formerCanadidateEdge.distLeft;
					double formerDistToEnd = formerCanadidateEdge.edge->lengthM - formerDistLeft;
					double routeNetworkDistBetweenTwoEdges;
					double routeNetworkDistBetweenTwoTrajPoints;
					if (canadidateEdge == formerCanadidateEdge.edge){
						routeNetworkDistBetweenTwoTrajPoints = fabs(currentDistLeft - formerCanadidateEdge.distLeft);
					}
					else{
						pair<int, int> odPair = make_pair(formerCanadidateEdge.edge->endNodeId, canadidateEdge->startNodeId);
						if (shortestDistPair.find(odPair) != shortestDistPair.end() && shortestDistPair[odPair].first < INF){
							shortestDistPair[odPair].first <= MAXSPEED*deltaT ? routeNetworkDistBetweenTwoEdges = shortestDistPair[odPair].first : routeNetworkDistBetweenTwoEdges = INF;
						}
						else{
							if (shortestDistPair.find(odPair) != shortestDistPair.end() && deltaT <= shortestDistPair[odPair].second){
								routeNetworkDistBetweenTwoEdges = INF;
							}
							else{
								list<Edge*> shortestPath = list<Edge*>();
								routeNetworkDistBetweenTwoEdges = routeNetwork.shortestPathLength(formerCanadidateEdge.edge->endNodeId, canadidateEdge->startNodeId, shortestPath, currentDistLeft, formerDistToEnd, deltaT);
								shortestDistPair[odPair] = make_pair(routeNetworkDistBetweenTwoEdges, deltaT);
							}
						}
						routeNetworkDistBetweenTwoTrajPoints = routeNetworkDistBetweenTwoEdges + currentDistLeft + formerDistToEnd;
					}
					long double transactionProb = exp(-fabs((long double)distBetweenTwoTrajPoints - (long double)routeNetworkDistBetweenTwoTrajPoints) / BT) / BT;
					long double tmpTotalProbForTransaction = formerCanadidateEdge.score * transactionProb;
					if (currentMaxProbTmp < tmpTotalProbForTransaction){
						currentMaxProbTmp = tmpTotalProbForTransaction;
						preColumnIndex = formerCanadidateEdgeIndex;
					}
					++formerCanadidateEdgeIndex;
				}
				emissionProbs[currentCanadidateEdgeIndex] *= currentMaxProbTmp;
			}
			scores.push_back(Score(canadidateEdge, emissionProbs[currentCanadidateEdgeIndex], preColumnIndex, currentDistLeft));
			if (currentMaxProb < emissionProbs[currentCanadidateEdgeIndex]){ currentMaxProb = emissionProbs[currentCanadidateEdgeIndex]; }
			++currentCanadidateEdgeIndex;
		}
		delete[]emissionProbs;
		formerTrajPoint = *trajectoryIterator;
		++currentTrajPointIndex;
		for (size_t i = 0; i < scores.size(); ++i)	{ scores[i].score /= currentMaxProb; }
		scoreMatrix.push_back(scores);
		if (scores.size() == 0){
			cutFlag = true;
			formerTrajPoint = NULL;
		}
		else
		{
			cutFlag = false;
		}
	}
	int startColumnIndex = GetStartColumnIndex(scoreMatrix.back());
	for (int i = static_cast<int>(scoreMatrix.size()) - 1; i >= 0; --i){
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
	return mapMatchingResult;
	//return linkMatchedResult(mapMatchingResult);
}

void main(int argc, char *argv[]){
	if (argc != 1 && argc != 3){
		cout << "应该有两个参数：第一个为输入文件所在文件夹路径；第二个为输出文件所在文件夹路径！" << endl;
		system("pause");
		return;
	}
	else{
		if (argc == 3){
			inputDirectory = argv[1];
			outputDirectory = argv[2];
		}
		cout << "输入文件所在文件夹路径：" << inputDirectory << endl;
		cout << "输出文件所在文件夹路径：" << outputDirectory << endl;
		scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
		int trajIndex = 0;
		cout << "开始地图匹配！" << endl;
		for (list<Traj*>::iterator trajIter = trajList.begin(); trajIter != trajList.end(); trajIter++){
			list<Edge*> resultList = MapMatching(*(*trajIter));
			ofstream MatchedEdgeOutput(rootFilePath + outputDirectory + "\\" + outputFileNames[trajIndex]);
			Traj::iterator trajPointIter = (*trajIter)->begin();
			for (list<Edge*>::iterator edgeIter = resultList.begin(); edgeIter != resultList.end(); edgeIter++, trajPointIter++){
				if (*edgeIter != NULL){
					int currentIndex = (*edgeIter)->id;
					MatchedEdgeOutput << (*trajPointIter)->time << "," << currentIndex << ",1.0" << endl;
				}
				else{
					MatchedEdgeOutput << (*trajPointIter)->time << "," << -1 << ",1.0" << endl;
				}
			}
			MatchedEdgeOutput.close();
			++trajIndex;
		}
		system("pause");
	}

}