#include<set>
#include "MapMatching.h"
#include "ReadInTrajs.h"

//GISCUP2012��һ�����㷨�������
#define UA 10
#define CA 4
#define UD 0.17
#define CD 1.4

//���������ȫ�ֱ���
vector<string> outputFileNames;
list<Traj*> trajList;
Map map = Map("D:\\Document\\Subjects\\Computer\\Develop\\Data\\SingaporeData\\", 500);
ofstream logOutput;

//��ͼƥ���������ݽṹ
struct CanadidateEdge{
	Edge* edge;//��ѡ·�ε�ָ��
	double score;//��ѡ·�εĵ÷֣�����ʽ(3)��
	int preColumnIndex;////��ѡ·�ε�ǰ��·�ε�������
	CanadidateEdge(Edge* edge, double score = -1, int preColumnIndex = -1){
		this->edge = edge;
		this->score = score;
		this->preColumnIndex = preColumnIndex;
	}
};

//�������ڹ켣���������·�εļн�
double CalculateAngle(GeoPoint* formerTrajPoint, GeoPoint* currentTrajPoint, Edge* edge){
	if (formerTrajPoint->time < currentTrajPoint->time){
		double x1 = currentTrajPoint->lon - formerTrajPoint->lon;
		double y1 = currentTrajPoint->lat - formerTrajPoint->lat;
		double x2 = map.nodes.at(edge->endNodeId)->lon - map.nodes.at(edge->startNodeId)->lon;
		double y2 = map.nodes.at(edge->endNodeId)->lat - map.nodes.at(edge->startNodeId)->lat;
		if ((x1 != 0 || y1 != 0) && (x2 != 0 || y2 != 0)){
			return x1*x2 + y1*y2;
		}
		else{
			return 1;
		}
	}
	else{
		cout << "��������ڹ켣��˳��ߵ���" << endl;
		return -1;
	}
}

//����÷֣�����ʽ(3)��
double CalculateScore(GeoPoint* formerTrajPoint, GeoPoint* trajPoint, Edge* edge){
	double tmp1 = pow(CalculateAngle(formerTrajPoint, trajPoint, edge), CA);
	double tmp2 = pow(map.distM(trajPoint->lat, trajPoint->lon, edge), CD);
	return UA*tmp1 - UD*tmp2;
}

//��ȡ��ѡ·��
void GetCanadidateEdges(list<GeoPoint*> &trajectory, vector<vector<CanadidateEdge>> &scoreMatrix){
	set<int> edgesSet = set<int>();
	set<int> endPointSet = set<int>();
	GeoPoint* formerTrajPoint = NULL;
	int trajPointIndex = 0;
	for each (GeoPoint* trajPoint in trajectory)
	{
		vector<CanadidateEdge> scores = vector<CanadidateEdge>();
		//��һ������ȡԭʼ��ѡ·��
		//ԭ����Ϊ��ȡ�����50��·�Σ��˴��޸�Ϊ��ȡ18m��Χ�ڵ�����·��
		//��������Ҫ������Ϊ����ȡ�����50��·�ο��Ա�֤���պ�ѡ·�����Ƿǿյģ�����ȡ18m��Χ�ڵ�����·�ο��ܵõ��ռ�
		vector<Edge*> rawCanadidates = map.getKNearEdges(trajPoint->lat, trajPoint->lon, 50);
		//����㣬��ʹ����������������֦����
		if (trajPointIndex > 0){
			for each (Edge* rawEdge in rawCanadidates)
			{
				//��֦��������ѡ·����ǰһ�켣��ĺ�ѡ·�μ����ڣ����ѡ·�������ǰһ�켣��ĺ�ѡ·�ε��յ㼯���ڣ�����֤����һ�Ժ�ѡ·�δ���ֱ�������ԣ�
				//��֦�������켣�����ߺͺ�ѡ·����ĩ�����߼н�С��90������֤�����ԣ�
				if ((edgesSet.count(rawEdge->id) == 1 || endPointSet.count(rawEdge->startNodeId) == 1)
					&& CalculateAngle(formerTrajPoint, trajPoint, rawEdge) > 0
					&& map.distM(trajPoint->lat, trajPoint->lon, rawEdge) <= 18){
					scores.push_back(CanadidateEdge(rawEdge));
				}
			}
			//������ڼ�֦������ԭʼ��ѡ·�ξ��޳��ˣ���һ�ּ�֦����
			if (rawCanadidates.size() > 0 && scores.size() == 0){
				vector<CanadidateEdge> tmpScores = vector<CanadidateEdge>();
				double tmpMax = -1e10;
				//��������ʽ(3)��ÿ����ѡ·�ν��д�֣������÷ֳ�������80%�ĺ�ѡ·��
				for each (Edge* var in rawCanadidates)
				{
					double tmpScore = CalculateScore(formerTrajPoint, trajPoint, var);
					tmpScores.push_back(CanadidateEdge(var, tmpScore));
					if (tmpScore > tmpMax){
						tmpMax = tmpScore;
					}
				}
				//Ҫ���ǵ�tmpMaxΪ�������orz
				if (tmpMax > 0){
					tmpMax = tmpMax*0.8;
				}
				else{
					tmpMax = tmpMax*1.2;
				}
				for each (CanadidateEdge var in tmpScores)
				{
					if (var.score >= tmpMax){
						scores.push_back(var);
					}
				}
			}
		}
		else{
			for each (Edge* var in rawCanadidates)
			{
				scores.push_back(CanadidateEdge(var));
			}
		}
		scoreMatrix.push_back(scores);
		edgesSet.clear();
		endPointSet.clear();
		for each (CanadidateEdge var in scores)
		{
			edgesSet.insert(var.edge->id);
			endPointSet.insert(var.edge->endNodeId);
		}
		formerTrajPoint = trajPoint;
		trajPointIndex++;
	}
}

//��̬�滮�����СȨֵ
list<Edge*> ShortestWeight(vector<vector<CanadidateEdge>> &scoreMatrix){
	list<Edge*> mapMatchingResult;//ȫ��ƥ��·��
	vector<vector<CanadidateEdge>>::iterator loopIter = scoreMatrix.begin();
	vector<vector<CanadidateEdge>>::iterator formerIter = scoreMatrix.begin();
	int columnIndex = 0;
	//��̬�滮
	for (; loopIter != scoreMatrix.end(); loopIter++){
		for (vector<CanadidateEdge>::iterator currentEdgeIter = loopIter->begin(); currentEdgeIter != loopIter->end(); currentEdgeIter++)
		{
			currentEdgeIter->preColumnIndex = -1;
			if (loopIter != scoreMatrix.begin()){
				currentEdgeIter->score = 1e10;
				int formerEdgeIndex = 0;
				for each (CanadidateEdge formerEdge in *formerIter)
				{
					double dist;
					if (formerEdge.edge->id == currentEdgeIter->edge->id){
						dist = 0;
					}
					if (formerEdge.edge->endNodeId == currentEdgeIter->edge->startNodeId){
						dist = formerEdge.edge->lengthM;
					}
					else{
						dist = map.shortestPathLength(currentEdgeIter->edge->id, formerEdge.edge->id);
					}
					if (dist + formerEdge.score < currentEdgeIter->score){
						currentEdgeIter->score = dist + formerEdge.score;
						currentEdgeIter->preColumnIndex = formerEdgeIndex;
					}
					formerEdgeIndex++;
				}
			}
			else
			{
				currentEdgeIter->score = 0;
			}
		}
		formerIter = loopIter;
		columnIndex++;
	}
	//���ݣ���¼ȫ����СȨֵ��
	double tmpMin = 1e10;
	int matchedEdgeIndex = -1;
	for (int i = 0; i < scoreMatrix.back().size(); i++){
		cout << scoreMatrix.back().at(i).edge->id << ":" << scoreMatrix.back().at(i).preColumnIndex << endl;
		if (scoreMatrix.back().at(i).score < tmpMin){
			tmpMin = scoreMatrix.back().at(i).score;
			matchedEdgeIndex = i;
		}
	}
	for (int i = scoreMatrix.size() - 1; i >= 0; i--){
		mapMatchingResult.push_front(scoreMatrix.at(i).at(matchedEdgeIndex).edge);
		matchedEdgeIndex = scoreMatrix.at(i).at(matchedEdgeIndex).preColumnIndex;
	}
	return mapMatchingResult;
}

list<Edge*> MapMatching(list<GeoPoint*> &trajectory){
	vector<vector<CanadidateEdge>> scoreMatrix = vector<vector<CanadidateEdge>>();//��ѡ·�ξ����д���һ���켣�㣻�д���һ����ѡ·��
	//��ȡ��ѡ·��
	GetCanadidateEdges(trajectory, scoreMatrix);
	//���Դ��룺���scoreMatrix�Թ��鿴
	//ofstream logOutput("DebugLog.txt");
	//logOutput << "scoreMatrix������" << scoreMatrix.size() << "�У�" << endl;
	//int count2 = 0;
	//for each (vector<CanadidateEdge> canadidateEdges in scoreMatrix)
	//{
	//	logOutput << canadidateEdges.size() << "\t";
	//	if (canadidateEdges.size() == 0){
	//		count2++;
	//	}
	//	for each (CanadidateEdge edge in canadidateEdges)
	//	{
	//		logOutput << edge.edge->id << "$" << edge.score << "$" << edge.preColumnIndex << "\t";
	//	}
	//	logOutput << endl;
	//}
	//logOutput.close();
	//system("pause");
	//���Խ���
	//����ȫ����СȨֵ�⣬��ȫ��ƥ��·��
	return ShortestWeight(scoreMatrix);
}

void main(){
	scanTrajFolder("D:\\Document\\Subjects\\Computer\\Develop\\Data\\SingaporeData\\", trajList, outputFileNames);
	int trajIndex = 0;
	cout << "��ʼ��ͼƥ�䣡" << endl;
	for (list<Traj*>::iterator trajIter = trajList.begin(); trajIter != trajList.end(); trajIter++){
		list<Edge*> resultList = MapMatching(*(*trajIter));
		ofstream MatchedEdgeOutput("D:\\Document\\Subjects\\Computer\\Develop\\Data\\SingaporeData\\test_output\\" + outputFileNames[trajIndex]);
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
}