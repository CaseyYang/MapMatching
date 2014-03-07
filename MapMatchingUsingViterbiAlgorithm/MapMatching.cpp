#include "MapMatching.h"
#include "ReadInTrajs.h"

//���������ȫ�ֱ���
vector<string> outputFileNames;
list<Traj*> trajList;
string rootFilePath = "D:\\Document\\Subjects\\Computer\\Develop\\Data\\SingaporeData\\";
Map routeNetwork = Map(rootFilePath, 500);
std::map<pair<int, int>, double> shortestDistPair = std::map<pair<int, int>, double>();//�����������������̾��룬��pair�Ա�ʾ�����յ㣬ֵ��ʾ�������̾���
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

//������ʼ��㺯��by Wang Yin��
//������ʣ�ʹ�ù켣�㵽��ѡ·�εľ����ڸ�˹�ֲ��ϵĸ���
//����t��ΪWang Yin�㷨���裬��ʾǰ��켣����ʱ���
//����dist���켣�㵽��ѡ·�εľ���
double EmissionProb(double t, double dist){
	return t*sqrt(dist)*COEFFICIENT_FOR_EMISSIONPROB;
}
//������ʼ��㺯��by���ɳ���
//double EmissionProb(double t, double dist){
//	return exp(t*dist * dist * N2_SIGMAZ2) * SQR_2PI_SIGMAZ;
//}

//��������������������������scoreMatrix�и���������������ĺ�ѡ·�ε�����
//by Wang Yin��
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
////by���ɳ�
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
	//betaֵby���ɳ�
	//int sampleRate = (trajectory.back()->time - trajectory.front()->time) / (trajectory.size() - 1);
	//cout << "�����ʣ�" << sampleRate << endl;
	//if (sampleRate > 30){
	//	sampleRate = 30;
	//}
	//long double BT = (long double)BETA_ARR[sampleRate];//��ǰ�켣�����õ�betaֵ������ת�Ƹ���ʱʹ��
	vector<vector<Score>> scoreMatrix = vector<vector<Score>>();//���й켣��ĸ��ʾ���
	//��Ҫ��ÿ��ѭ������ǰ���µı���
	GeoPoint* formerTrajPoint = NULL;//��һ���켣�㣬����·������ʱ��Ҫ
	bool cutFlag = true;//û��ǰһ�켣���ǰһ�켣��û�д��ĺ�ѡ·��
	int currentTrajPointIndex = 0;//��ǰ�켣�������
	//����ÿ���켣��
	list<GeoPoint*>::iterator trajectoryIterator = trajectory.begin();
	for (; trajectoryIterator != trajectory.end(); trajectoryIterator++)
	{
		double deltaT = -1;//��ǰ��켣�����ʱ��delaT��ʾǰ�����켣����ʱ���
		if (formerTrajPoint != NULL){
			deltaT = (*trajectoryIterator)->time - formerTrajPoint->time;
		}
		//������ʳ�ʼֵby Wang Yin��
		long double currentMaxProb = 1e10;//��ǰ������
		//������ʳ�ʼֵby���ɳ���
		//long double currentMaxProb = -1e10;//��ǰ������
		vector<Score> scores = vector<Score>();//��ǰ�켣���Score����
		vector<Edge*> canadidateEdges;
		routeNetwork.getNearEdges((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, RANGEOFCANADIDATEEDGES, canadidateEdges);//���������ָ����Χ�ڵĺ�ѡ·�μ���
		long double *emissionProbs = new long double[canadidateEdges.size()];//������Щ��ѡ·�εķ������
		int currentCanadidateEdgeIndex = 0;//��ǰ��ѡ·�ε�����
		//cout << "����ѭ��ǰ" << endl;
		//system("pause");
		//for (vector<Edge*>::iterator canadidateEdgeIter = canadidateEdges.begin(); canadidateEdgeIter != canadidateEdges.end(); canadidateEdgeIter++)
		for each (Edge* canadidateEdge in canadidateEdges)
		{
			int preColumnIndex = -1;//���浱ǰ��ѡ·�ε�ǰ��·�ε�������
			double currentDistLeft = 0;//��ǰ�켣���ں�ѡ·���ϵ�ͶӰ���·�����ľ���
			double DistBetweenTrajPointAndEdge = routeNetwork.distM((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, canadidateEdge, currentDistLeft);
			//������Щ��ѡ·�εķ������
			if (cutFlag){
				//����������Ʒ�ģ�͵ļ��㵱ǰ�켣���Ӧÿ����ѡ·�εķ������
				//Wang Yin���㷨��ʹ��ͬ����������켣���ķ������
				emissionProbs[currentCanadidateEdgeIndex] = EmissionProb(1, DistBetweenTrajPointAndEdge);
			}
			else{
				//����ǹ켣���ķ������by Wang Yin��
				emissionProbs[currentCanadidateEdgeIndex] = EmissionProb(deltaT, DistBetweenTrajPointAndEdge);
				//����ǹ켣���ķ������by���ɳ���
				//emissionProbs[currentCanadidateEdgeIndex] = EmissionProb(1, DistBetweenTrajPointAndEdge);
				//ת�Ƹ��ʳ�ʼֵby Wang Yin��
				long double currentMaxProbTmp = 1e10;
				//cout << "����Сѭ��ǰ" << endl;
				//system("pause");
				//ת�Ƹ��ʳ�ʼֵby���ɳ���
				//long double currentMaxProbTmp = -1e10;
				for (int i = 0; i < scoreMatrix.back().size(); i++)
				{
					double formerDistLeft = scoreMatrix[currentTrajPointIndex - 1][i].distLeft;//ǰһ���켣���ں�ѡ·���ϵ�ͶӰ���·�����ľ���
					double routeNetworkDistBetweenTwoEdges;//��·������ľ���
					double routeNetworkDistBetweenTwoTrajPoints;//���켣���Ӧ��ͶӰ����·������
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
					//���켣����ֱ�Ӿ���
					double distBetweenTwoTrajPoints = GeoPoint::distM((*trajectoryIterator)->lat, (*trajectoryIterator)->lon, formerTrajPoint->lat, (*trajectoryIterator)->lon);
					//double transactionProb = exp(-fabs((long double)routeNetworkDistBetweenTwoTrajPoints - (long double)distBetweenTwoTrajPoints) / BT) / BT;
					//����ת�Ƹ���by Wang Yin��
					long double transactionProb = (long double)routeNetworkDistBetweenTwoTrajPoints*COEFFICIENT_FOR_TRANSATIONPROB;
					//����ת�Ƹ���by���ɳ���
					//long double transactionProb = exp(-fabs((long double)distBetweenTwoTrajPoints - (long double)routeNetworkDistBetweenTwoTrajPoints) / BT) / BT;
					/*GIS2012CUP���Ż����ڴ˴�����transactionProb�����޸�*/
					long double tmpTotalProbForTransaction = scoreMatrix[currentTrajPointIndex - 1][i].score * transactionProb;
					//���Դ��룺���ָ���켣��������ʵĸ�����ɲ��֣�����ĳ��ֵ�ر�С/������ܾ������ֵ
					//if (currentTrajPointIndex == 638){
					//	cout << "ǰ���������Ϊ��" << scoreMatrix[currentTrajPointIndex - 1][i].score << endl;
					//	cout << "��ǰת�Ƹ���Ϊ��" << transactionProb << endl;
					//	cout << "��ǰ�������Ϊ��" << emissionProbs[currentCanadidateEdgeIndex] << endl;
					//	cout << "��ǰ�������Ϊ��" << tmpTotalProbForTransaction << endl;
					//	system("pause");
					//}
					//���Խ���
					//����ת�Ƹ���by Wang Yin��
					//��currentMaxProbTmp��������ǰ���ѡ·����ת�Ƹ��������
					if (currentMaxProbTmp > tmpTotalProbForTransaction){
						currentMaxProbTmp = tmpTotalProbForTransaction;
						preColumnIndex = i;
					}
					//����ת�Ƹ���by���ɳ���
					//if (currentMaxProbTmp < tmpTotalProbForTransaction){
					//	currentMaxProbTmp = tmpTotalProbForTransaction;
					//	preColumnIndex = i;
					//}
				}
				//��ʱ��emissionProbs������Ѿ���ĳ����ѡ·�ε����������
				//�����������by Wang Yin��
				emissionProbs[currentCanadidateEdgeIndex] += currentMaxProbTmp;
				//�����������by���ɳ���
				//emissionProbs[currentCanadidateEdgeIndex] *= currentMaxProbTmp;
			}
			//����Ҫ�������������ٶȣ���ֻ��������ʴ���MINPROB�ĺ�ѡ·�η��뵱ǰ�켣���Score�����У�
			//��������к�ѡ·�η���Score������
			scores.push_back(Score(canadidateEdge, emissionProbs[currentCanadidateEdgeIndex], preColumnIndex, currentDistLeft));
			//�õ���ǰ����������by Wang Yin��
			if (currentMaxProb > emissionProbs[currentCanadidateEdgeIndex]){
				currentMaxProb = emissionProbs[currentCanadidateEdgeIndex];
			}
			//�õ���ǰ����������by���ɳ���
			//if (currentMaxProb < emissionProbs[currentCanadidateEdgeIndex]){
			//	currentMaxProb = emissionProbs[currentCanadidateEdgeIndex];
			//}
			currentCanadidateEdgeIndex++;
		}
		delete[]emissionProbs;
		formerTrajPoint = (*trajectoryIterator);
		currentTrajPointIndex++;
		//system("pause");
		//��һ��
		for (int i = 0; i < scores.size(); i++)
		{
			scores[i].score /= currentMaxProb;
		}
		//�Ѹù켣���Scores�������scoreMatrix��
		scoreMatrix.push_back(scores);
		//��scores����Ϊ�գ���˵��û��һ�����ĺ�ѡ·�Σ�cutFlag��Ϊtrue�������켣��Ϊ�¹켣����ƥ��
		if (scores.size() == 0){
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
	//		logOutput << scoreMatrix.at(i).at(j).edge->id << "$" << scoreMatrix.at(i).at(j).preColumnIndex << "\t";
	//	}
	//	logOutput << endl;
	//}
	//���Խ���

	return mapMatchingResult;
	//return linkMatchedResult(mapMatchingResult);
}

void main(){
	//logOutput = ofstream("debug2.txt");
	scanTrajFolder(rootFilePath, trajList, outputFileNames);
	int trajIndex = 0;
	cout << "��ʼ��ͼƥ�䣡" << endl;
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
		cout << "��" << trajIndex << "���켣ƥ����ϣ�" << endl;
		trajIndex++;
	}
	//logOutput.close();
}