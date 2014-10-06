#include "MapMatchingUsingBiasStatistic.h"
using namespace std;

//�����Ѽ���·�μ���̾���
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

//��ͼƥ���������ݽṹ
struct Score2//����ĳ���켣���Ӧ��һ����ѡ·��
{
	Edge* edge;//��ѡ·�ε�ָ��
	long double score;//��ѡ·�������е��������
	int preColumnIndex;//��ѡ·�ε�ǰ��·�ε�������
	double distLeft;//�켣���ͶӰ�㵽��ѡ·�����ľ���
	vector<long double> *priorProbs;
	Score2(Edge* edge, long double score, int pre, double distLeft){
		this->edge = edge;
		this->score = score;
		this->preColumnIndex = pre;
		this->distLeft = distLeft;
		priorProbs = new vector<long double>();
	}
};

//������ʼ��㺯��
double EmissionProb2(double t, double dist){
	return exp(t*dist * dist * N2_SIGMAZ2) * SQR_2PI_SIGMAZ;
}

//ʹ�������·��ƫ�ý��е�ͼƥ��
//����ĳ���켣�����ڵ�����ѡ���������Ƶ������·����Ϊƥ��·�Σ������������û���κ�·��Ƶ���������ǰһƥ��·�ε��ù켣�㸽���ĺ�ѡ·��ת�Ƹ���������Ϊ�ù켣���ƥ��·��
list<Edge*> MapMatchingUsingBiasStatistic(list<GeoPoint*> &trajectory){
	long double BT = 34.56991141;
	list<Edge*> matchedEdges = list<Edge*>();
	GeoPoint* formerTrajPoint = NULL;
	Edge* formerMatchedEdge = NULL;
	//int trajPointIndex = 0;//�켣����ţ�������
	for each (GeoPoint* trajPoint in trajectory)
	{
		//cout << "�켣����ţ�" << trajPointIndex << endl;
		//trajPointIndex++;
		Edge* matchedEdge = NULL;
		pair<int, int>gridCellIndex = litePointGridIndex.getRowCol(trajPoint);//routeNetwork.findGridCellIndex(trajPoint->lat, trajPoint->lon);
		//����켣����������������ʷƥ��·��Ƶ����Ϣ����ѡȡ����Ƶ������·����Ϊƥ��·��
		if (biasSet.find(gridCellIndex) != biasSet.end()){
			int max = 0;//��¼��ǰ���Ƶ��
			for each (pair<Edge*, int> edgeCountPair in biasSet[gridCellIndex])
			{
				if (edgeCountPair.second > max){
					matchedEdge = edgeCountPair.first;
					max = edgeCountPair.second;
				}
			}
		}
		//����켣������������û����ʷƥ��·��Ƶ����Ϣ����ѡ��ǰһƥ��·�ε���ѡ·����ת�Ƹ���������Ϊƥ��·��
		else{
			//���Ȼ�øù켣��ĺ�ѡ·�Σ�Ȼ��ѡ��ת�Ƹ������ĺ�ѡ·��
			vector<Edge*> canadidateEdges;//��ѡ·�μ���
			routeNetwork.getNearEdges(trajPoint->lat, trajPoint->lon, RANGEOFCANADIDATEEDGES, canadidateEdges);//���������ָ����Χ�ڵĺ�ѡ·�μ���
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
					double formerDistToEnd = formerMatchedEdge->lengthM - formerDistLeft;//ǰһ���켣���ں�ѡ·���ϵ�ͶӰ���·���յ�ľ���
					list<Edge*> shortestPath = list<Edge*>();
					double routeNetworkDistBetweenTwoEdges = routeNetwork.shortestPathLength(formerMatchedEdge->endNodeId, canadidateEdge->startNodeId, shortestPath, currentDistLeft, formerDistToEnd, deltaT);
					long double routeNetworkDistBetweenTwoTrajPoints = routeNetworkDistBetweenTwoEdges + currentDistLeft + formerDistToEnd;
					long double transactionProb = exp(-fabs(distBetweenTwoTrajPoints - routeNetworkDistBetweenTwoTrajPoints) / BT) / BT;//ת�Ƹ���
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

//ʹ�������·��ƫ����Ϊ������ʽ��е�ͼƥ��
list<Edge*> MapMatchingUsingBiasStatisticAsPriorProb(list<GeoPoint*> &trajectory){
	list<Edge*> mapMatchingResult;//ȫ��ƥ��·��
	int sampleRate = (static_cast<int>(trajectory.size()) > 1 ? (trajectory.back()->time - trajectory.front()->time) / (static_cast<int>(trajectory.size()) - 1) : (trajectory.back()->time - trajectory.front()->time));//����켣ƽ��������
	if (sampleRate > 30){ sampleRate = 30; }
	long double BT = (long double)BETA_ARR[sampleRate];//���ݹ켣ƽ��������ȷ��betaֵ������ת�Ƹ���ʱʹ��
	vector<vector<Score2>> scoreMatrix = vector<vector<Score2>>();//���й켣��ĸ��ʾ���
	/*��Ҫ��ÿ��ѭ������ǰ���µı���*/
	GeoPoint* formerTrajPoint = NULL;//��һ���켣�㣬����·������ʱ��Ҫ
	int currentTrajPointIndex = 0;//��ǰ�켣�������	
	for each(auto trajPoint in trajectory)//����ÿ���켣��
	{
		double distBetweenTwoTrajPoints;//���켣����ֱ�Ӿ���
		double deltaT = -1;//��ǰ��켣�����ʱ��deltaT��ʾǰ�����켣����ʱ���
		if (formerTrajPoint != NULL){
			deltaT = trajPoint->time - formerTrajPoint->time;
			distBetweenTwoTrajPoints = GeoPoint::distM(trajPoint->lat, trajPoint->lon, formerTrajPoint->lat, formerTrajPoint->lon);
		}
		vector<Score2> scores = vector<Score2>();//��ǰ�켣���Score2����
		/*������ʡ���������ʡ���·�γ����ڸ�������ʷƥ���¼�е�Ƶ�ʻ�������븽��50����·�ξ����ڸ�˹�ֲ��ϵĸ���*/
		long double totalCount = 0;//��һ��
		pair<int, int>gridCellIndex = litePointGridIndex.getRowCol(trajPoint);//routeNetwork.findGridCellIndex(trajPoint->lat, trajPoint->lon);
		//����켣����������������ʷƥ��·��Ƶ����Ϣ�������Щ��ʷƥ��·����Ϊ��ѡ·�Σ�ÿ����ʷƥ��·�γ��ֵ�Ƶ�����Ը�����������·�γ��ֵ���Ƶ����Ϊ�������
		if (biasSet.find(gridCellIndex) != biasSet.end()){
			for each (pair<Edge*, int> edgeCountPair in biasSet[gridCellIndex])
			{
				double currentDistLeft = 0;//��ǰ�켣���ں�ѡ·���ϵ�ͶӰ���·�����ľ���
				routeNetwork.distMFromTransplantFromSRC(trajPoint->lat, trajPoint->lon, edgeCountPair.first, currentDistLeft);
				scores.push_back(Score2(edgeCountPair.first, edgeCountPair.second, -1, currentDistLeft));
				totalCount += edgeCountPair.second;
			}
		}
		//����켣������������û����ʷƥ��·��Ƶ����Ϣ����Ѳ������븽��50����·�ξ����ڸ�˹�ֲ��ϵĸ�����Ϊ������ʣ���HMM�㷨��������
		else
		{
			vector<Edge*> canadidateEdges = vector<Edge*>();//��ѡ·�μ���
			routeNetwork.getNearEdges(trajPoint->lat, trajPoint->lon, RANGEOFCANADIDATEEDGES, canadidateEdges);//���������ָ����Χ�ڵĺ�ѡ·�μ���
			for each (Edge* canadidateEdge in canadidateEdges)
			{
				double currentDistLeft = 0;//��ǰ�켣���ں�ѡ·���ϵ�ͶӰ���·�����ľ���
				double DistBetweenTrajPointAndEdge = routeNetwork.distMFromTransplantFromSRC(trajPoint->lat, trajPoint->lon, canadidateEdge, currentDistLeft);
				//������Щ��ѡ·�εķ������
				long double emissionProb = EmissionProb2(1, DistBetweenTrajPointAndEdge);
				scores.push_back(Score2(canadidateEdge, emissionProb, -1, currentDistLeft));
				totalCount += emissionProb;
			}
		}
		for (size_t indexForScore2s = 0; indexForScore2s < scores.size(); indexForScore2s++){
			scores[indexForScore2s].score /= totalCount;
		}
		/*������ʡ���ת�Ƹ��ʡ����������������ŷʽ������·������֮����ָ���ֲ��ϵĸ���*/
		if (scoreMatrix.size()>0){
			for each (Score2 canadidateEdge in scores)
			{
				for (size_t formerCanadidateEdgeIndex = 0; formerCanadidateEdgeIndex < scoreMatrix.back().size(); formerCanadidateEdgeIndex++){
					double formerDistLeft = scoreMatrix.back()[formerCanadidateEdgeIndex].distLeft;//ǰһ���켣���ں�ѡ·���ϵ�ͶӰ���·�����ľ���
					double formerDistToEnd = scoreMatrix.back()[formerCanadidateEdgeIndex].edge->lengthM - formerDistLeft;//ǰһ���켣���ں�ѡ·���ϵ�ͶӰ���·���յ�ľ���
					double routeNetworkDistBetweenTwoEdges;//��·������ľ���
					double routeNetworkDistBetweenTwoTrajPoints;//���켣���Ӧ��ͶӰ����·������
					//���ǰһƥ��·�κ͵�ǰ��ѡ·����ͬһ·�Σ������߾�·��������ĲΪ·������
					//ǰһ�����㣨scoreMatrix.back()[formerCanadidateEdgeIndex]������ǰ�����㣨canadidateEdge����·��������������ڸ��Ե�distLeft��Ա��
					if (canadidateEdge.edge == scoreMatrix.back()[formerCanadidateEdgeIndex].edge){
						routeNetworkDistBetweenTwoTrajPoints = fabs(canadidateEdge.distLeft - scoreMatrix.back()[formerCanadidateEdgeIndex].distLeft);
					}
					//���ǰһƥ��·�κ͵�ǰ��ѡ·�β���ͬһ·�Σ�����Ҫʹ�����·����������·������
					else{
						pair<int, int> odPair = make_pair(scoreMatrix.back()[formerCanadidateEdgeIndex].edge->endNodeId, canadidateEdge.edge->startNodeId);
						//������������յ����̾����Ѿ���������Ҳ���INF����ֱ�Ӵӱ����Ѽ���·�μ���̾����shortestDistPair2�и�����Ӧ��·��ID��odPairȡ����̾���ֵ
						if (shortestDistPair2.find(odPair) != shortestDistPair2.end() && shortestDistPair2[odPair].first < INF){
							//�����ǰdeltaT�µ��ƶ��������ޱ���̾���Ҫ�󣬵������·�����õ���Ҳ�Ǳ���ľ���ֵ����֮�õ��ľ���INF
							shortestDistPair2[odPair].first <= MAXSPEED*deltaT ? routeNetworkDistBetweenTwoEdges = shortestDistPair2[odPair].first : routeNetworkDistBetweenTwoEdges = INF;
						}
						else{
							//������������յ����̾�����INF���ҵ�ǰdeltaT���ϴμ������·ʱ���ƶ�ʱ��ҪС��˵����ǰdeltaT�µõ������·��������INF
							if (shortestDistPair2.find(odPair) != shortestDistPair2.end() && deltaT <= shortestDistPair2[odPair].second){
								routeNetworkDistBetweenTwoEdges = INF;
							}
							/*ʣ�������
							���߸��������յ����̾����δ�������
							���߱������̾�������INF������ǰdeltaT�ȱ����deltaTҪ�󣨼����ƶ�ʱ����Ѽ���ĳ��������ܵõ��µ����·�����
							�����Ҫ���ú����������·
							*/
							else{
								list<Edge*> shortestPath = list<Edge*>();
								routeNetworkDistBetweenTwoEdges = routeNetwork.shortestPathLength(scoreMatrix.back()[formerCanadidateEdgeIndex].edge->endNodeId, canadidateEdge.edge->startNodeId, shortestPath, canadidateEdge.distLeft, formerDistToEnd, deltaT);
								shortestDistPair2[odPair] = make_pair(routeNetworkDistBetweenTwoEdges, deltaT);
							}
						}
						//����������·������=·�μ�·������+��ǰ�����㵽��ѡ·�����ľ���+ǰһ�����㵽��Ӧƥ��·��ĩβ�ľ���
						routeNetworkDistBetweenTwoTrajPoints = routeNetworkDistBetweenTwoEdges + canadidateEdge.distLeft + formerDistToEnd;
					}
					long double transactionProb = exp(-fabs((long double)distBetweenTwoTrajPoints - (long double)routeNetworkDistBetweenTwoTrajPoints) / BT) / BT;//ת�Ƹ���
					//cout << "�������� ·�μ���� ת�Ƹ��ʣ�" <<distBetweenTwoTrajPoints<<" "<<routeNetworkDistBetweenTwoTrajPoints<<" "<< transactionProb << endl;
					scoreMatrix.back()[formerCanadidateEdgeIndex].priorProbs->push_back(transactionProb);
				}
			}
		}
		scoreMatrix.push_back(scores);//�Ѹù켣���Score2s�������scoreMatrix��
		formerTrajPoint = trajPoint;
		currentTrajPointIndex++;
	}
	/*�õ�ȫ��ƥ��·��*/
	int startColumnIndex = 0;
	long double tmpMaxProb = -1;
	//�õ����һ���켣�����scoreMatrix��Ӧ���е÷���ߵ�����������ȫ��ƥ��·�����յ�
	for (size_t index = 0; index < scoreMatrix.back().size(); index++)
	{
		if (scoreMatrix.back()[index].score > tmpMaxProb){
			tmpMaxProb = scoreMatrix.back()[index].score;
			startColumnIndex = static_cast<int>(index);
		}
	}
	mapMatchingResult.push_front(scoreMatrix.back()[startColumnIndex].edge);
	int lastColumnIndex = startColumnIndex;
	for (int i = static_cast<int>(scoreMatrix.size()) - 2; i >= 0; i--){//ע�⣺�˴�i���Ͳ�����Ϊsize_t����Ϊsize_t���޷������ͣ���i==0ʱi--��ʹ��i���һ��������
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