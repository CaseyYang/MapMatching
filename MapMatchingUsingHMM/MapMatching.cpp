#include "MapMatching.h"
#include "ReadInTrajs.h"

//���������ȫ�ֱ���
vector<string> outputFileNames;
list<Traj*> trajList;
string rootFilePath = "E:\\Documents\\Computer\\Data\\TrajData\\WashingtonState\\";
string inputDirectory = "input_90";
string outputDirectory = "output_90";
Map routeNetwork = Map(rootFilePath, 1000);
//�����������������̾��룬��pair�Ա�ʾ�����յ㣬ֵpair��ʾ�������̾���Ͷ�Ӧ��deltaT
//�����deltaT��ԭ���ǣ����deltaT��С���򷵻ص���̾������ΪINF��������������ͬ�����յ㡢��deltaT���ʱ����̾�����ܾͲ���INF��
//���Ƶģ����ѱ������̾��벻��INF��������ĳ����С��deltaTʱ����̾�����ܾ���INF��
std::map<pair<int, int>, pair<double, double>> shortestDistPair = std::map<pair<int, int>, pair<double, double>>();
//ofstream logOutput;

//�ڲ�ͬ�����ʣ�1~30s/�㣩�¸���beta��ֵ�����ó�= =
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
struct Score//����ĳ���켣���Ӧ��һ����ѡ·��
{
	Edge* edge;//��ѡ·�ε�ָ��
	long double score;//��ѡ·�������е��������
	int preColumnIndex;//��ѡ·�ε�ǰ��·�ε�������
	double distLeft;//�켣���ͶӰ�㵽��ѡ·�����ľ���
	Score(Edge* edge, long double score, int pre, double distLeft){
		this->edge = edge;
		this->score = score;
		this->preColumnIndex = pre;
		this->distLeft = distLeft;
	}
};

//������ʼ��㺯��
double EmissionProb(double t, double dist){
	return exp(t*dist * dist * N2_SIGMAZ2) * SQR_2PI_SIGMAZ;
}

//��������������������������scoreMatrix�и���������������ĺ�ѡ·�ε�����
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

//������ƥ��·�β����ӣ���������·�μ���������·��ͨ����·��
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
	list<Edge*> mapMatchingResult;//ȫ��ƥ��·��
	int sampleRate = (trajectory.size() > 1 ? (trajectory.back()->time - trajectory.front()->time) / (trajectory.size() - 1) : (trajectory.back()->time - trajectory.front()->time));//����켣ƽ��������
	//cout << "�����ʣ�" << sampleRate << endl;
	if (sampleRate > 30){ sampleRate = 30; }
	long double BT = (long double)BETA_ARR[sampleRate];//���ݹ켣ƽ��������ȷ��betaֵ������ת�Ƹ���ʱʹ��
	vector<vector<Score>> scoreMatrix = vector<vector<Score>>();//���й켣��ĸ��ʾ���
	//��Ҫ��ÿ��ѭ������ǰ���µı���
	GeoPoint* formerTrajPoint = NULL;//��һ���켣�㣬����·������ʱ��Ҫ
	bool cutFlag = true;//û��ǰһ�켣���ǰһ�켣��û�д��ĺ�ѡ·��
	int currentTrajPointIndex = 0;//��ǰ�켣�������	
	for (list<GeoPoint*>::iterator trajectoryIterator = trajectory.begin(); trajectoryIterator != trajectory.end(); trajectoryIterator++)//����ÿ���켣��
	{
		double distBetweenTwoTrajPoints;//���켣����ֱ�Ӿ���
		double deltaT = -1;//��ǰ��켣�����ʱ��deltaT��ʾǰ�����켣����ʱ���
		if (formerTrajPoint != NULL){
			deltaT = (*trajectoryIterator)->time - formerTrajPoint->time;
			distBetweenTwoTrajPoints = GeoPoint::distM((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, formerTrajPoint->lat, formerTrajPoint->lon);
		}
		long double currentMaxProb = -1e10;//��ǰ���������ʣ���ʼֵΪ-1e10
		vector<Score> scores = vector<Score>();//��ǰ�켣���Score����
		vector<Edge*> canadidateEdges;//��ѡ·�μ���
		routeNetwork.getNearEdges((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, RANGEOFCANADIDATEEDGES, canadidateEdges);//���������ָ����Χ�ڵĺ�ѡ·�μ���
		long double *emissionProbs = new long double[canadidateEdges.size()];//�����ѡ·�εķ������
		int currentCanadidateEdgeIndex = 0;//��ǰ��ѡ·�ε�����
		for each (Edge* canadidateEdge in canadidateEdges)
		{
			int preColumnIndex = -1;//���浱ǰ��ѡ·�ε�ǰ��·�ε�������
			double currentDistLeft = 0;//��ǰ�켣���ں�ѡ·���ϵ�ͶӰ���·�����ľ���
			double DistBetweenTrajPointAndEdge = routeNetwork.distMFromTransplantFromSRC((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, canadidateEdge, currentDistLeft);
			//������Щ��ѡ·�εķ������
			emissionProbs[currentCanadidateEdgeIndex] = EmissionProb(1, DistBetweenTrajPointAndEdge);
			if (!cutFlag){
				//��ǰ�����㲻�ǹ켣��һ�����ƥ���жϺ�ĵ�һ���㣬�����ת�Ƹ���
				long double currentMaxProbTmp = -1e10;//��ǰ���ת�Ƹ��ʣ���ʼֵΪ-1e10				
				int formerCanadidateEdgeIndex = 0;
				for each(Score formerCanadidateEdge in scoreMatrix.back()){
					double formerDistLeft = formerCanadidateEdge.distLeft;//ǰһ���켣���ں�ѡ·���ϵ�ͶӰ���·�����ľ���
					double formerDistToEnd = formerCanadidateEdge.edge->lengthM - formerDistLeft;//ǰһ���켣���ں�ѡ·���ϵ�ͶӰ���·���յ�ľ���
					double routeNetworkDistBetweenTwoEdges;//��·������ľ���
					double routeNetworkDistBetweenTwoTrajPoints;//���켣���Ӧ��ͶӰ����·������
					if (canadidateEdge == formerCanadidateEdge.edge){//���ǰһƥ��·�κ͵�ǰ��ѡ·����ͬһ·�Σ������߼���������ĲΪ·������
						routeNetworkDistBetweenTwoTrajPoints = fabs(currentDistLeft - formerCanadidateEdge.distLeft);
					}
					else{
						pair<int, int> odPair = make_pair(formerCanadidateEdge.edge->endNodeId, canadidateEdge->startNodeId);
						//���������յ����·�Ѿ���������Ҳ���INF
						if (shortestDistPair.find(odPair) != shortestDistPair.end() && shortestDistPair[odPair].first < INF){
							//�����ǰdeltaT�µ��ƶ��������ޱ���̾���Ҫ�󣬵������·�����õ���Ҳ�Ǳ���ľ���ֵ����֮�õ��ľ���INF
							shortestDistPair[odPair].first <= MAXSPEED*deltaT ? routeNetworkDistBetweenTwoEdges = shortestDistPair[odPair].first : routeNetworkDistBetweenTwoEdges = INF;
						}
						else{
							if (shortestDistPair.find(odPair) != shortestDistPair.end() && deltaT <= shortestDistPair[odPair].second){//����ĸ��������յ����·�����INF���ҵ�ǰdeltaT���ϴμ������·ʱ���ƶ�ʱ��ҪС��˵����ǰdeltaT�µõ������·��������INF
								routeNetworkDistBetweenTwoEdges = INF;
							}
							else{
								//����δ������������յ�����·��������ߵ�ǰdeltaT�ȱ����deltaTҪ�󣬿��ܵõ����������·�������֮����Ҫ���ú����������·
								list<Edge*> shortestPath = list<Edge*>();
								routeNetworkDistBetweenTwoEdges = routeNetwork.shortestPathLength(formerCanadidateEdge.edge->endNodeId, canadidateEdge->startNodeId, shortestPath, currentDistLeft, formerDistToEnd, deltaT);
								shortestDistPair[odPair] = make_pair(routeNetworkDistBetweenTwoEdges, deltaT);
							}
						}
						routeNetworkDistBetweenTwoTrajPoints = routeNetworkDistBetweenTwoEdges + currentDistLeft + formerDistToEnd;
					}
					long double transactionProb = exp(-fabs((long double)distBetweenTwoTrajPoints - (long double)routeNetworkDistBetweenTwoTrajPoints) / BT) / BT;//ת�Ƹ���
					/*GIS2012CUP���Ż����ڴ˴�����transactionProb�����޸�*/
					long double tmpTotalProbForTransaction = formerCanadidateEdge.score * transactionProb;
					if (currentMaxProbTmp < tmpTotalProbForTransaction){//������ǰת�Ƹ��ʺ���֪���ת�Ƹ����нϴ���
						currentMaxProbTmp = tmpTotalProbForTransaction;
						preColumnIndex = formerCanadidateEdgeIndex;
					}
					formerCanadidateEdgeIndex++;
				}
				//��ʱ��emissionProbs������Ǻ�ѡ·�εķ�����ʣ�����ת�Ƹ������Ϊ��ѡ·�ε��������
				emissionProbs[currentCanadidateEdgeIndex] *= currentMaxProbTmp;
			}
			/*����Ҫ�������������ٶȣ���ֻ��������ʴ���MINPROB�ĺ�ѡ·�η��뵱ǰ�켣���Score�����У���������к�ѡ·�η���Score������*/
			scores.push_back(Score(canadidateEdge, emissionProbs[currentCanadidateEdgeIndex], preColumnIndex, currentDistLeft));
			//�õ���ǰ���������ʣ��Ա��һ��
			if (currentMaxProb < emissionProbs[currentCanadidateEdgeIndex]){ currentMaxProb = emissionProbs[currentCanadidateEdgeIndex]; }
			currentCanadidateEdgeIndex++;
		}
		delete[]emissionProbs;
		formerTrajPoint = *trajectoryIterator;
		currentTrajPointIndex++;
		for (size_t i = 0; i < scores.size(); i++)	{ scores[i].score /= currentMaxProb; }//��һ��
		scoreMatrix.push_back(scores);//�Ѹù켣���Scores�������scoreMatrix��
		if (scores.size() == 0){//��scores����Ϊ�գ���˵��û��һ�����ĺ�ѡ·�Σ�cutFlag��Ϊtrue�������켣��Ϊ�¹켣����ƥ��
			cutFlag = true;
			formerTrajPoint = NULL;
		}
		else
		{
			cutFlag = false;
		}
	}

	//�õ�ȫ��ƥ��·��
	int startColumnIndex = GetStartColumnIndex(scoreMatrix.back());//�õ����һ���켣�����scoreMatrix��Ӧ���е÷���ߵ�����������ȫ��ƥ��·�����յ�
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
	//���Դ��룺������յĸ��ʾ��������ĳ���켣������к�ѡ·�ε�������ʾ�Ϊ��Ϊ����С/������ǿ��ܾͲ���������Ҫ��һ�������и��ʵĵõ�����
	//for (int i = 0; i < scoreMatrix.size(); i++){
	//	logOutput << scoreMatrix.at(i).size() << "\t";
	//	for (int j = 0; j < scoreMatrix.at(i).size(); j++){
	//		logOutput << "[" << scoreMatrix.at(i).at(j).edge->id << "][" << scoreMatrix.at(i).at(j).preColumnIndex << "][" << scoreMatrix.at(i).at(j).score << "]\t";
	//	}
	//	logOutput << endl;
	//}
	//���Խ���

	return mapMatchingResult;
	//return linkMatchedResult(mapMatchingResult);
}

void main(){
	//logOutput = ofstream("debug.txt");
	//logOutput.setf(ios::showpoint);
	//logOutput.precision(8);
	scanTrajFolder(rootFilePath, inputDirectory, trajList, outputFileNames);
	int trajIndex = 0;
	cout << "��ʼ��ͼƥ�䣡" << endl;
	for (list<Traj*>::iterator trajIter = trajList.begin(); trajIter != trajList.end(); trajIter++){
		list<Edge*> resultList = MapMatching(*(*trajIter));
		ofstream MatchedEdgeOutput(rootFilePath +outputDirectory+ "\\" + outputFileNames[trajIndex]);
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
		cout << "��" << trajIndex << "���켣ƥ����ϣ�" << endl;
		trajIndex++;
	}
	//logOutput.close();
}