#include<set>
#include "MapMatching.h"
#include "ReadInTrajs.h"

//GISCUP2012第一名的算法所需参数
#define UA 10
#define CA 4
#define UD 0.17
#define CD 1.4

//运行所需的全局变量
vector<string> outputFileNames;
list<Traj*> trajList;
Map map = Map("D:\\Document\\Subjects\\Computer\\Develop\\Data\\SingaporeData\\", 500);
ofstream logOutput;

//地图匹配所用数据结构
struct CanadidateEdge{
	Edge* edge;//候选路段的指针
	double score;//候选路段的得分（文中式(3)）
	int preColumnIndex;////候选路段的前序路段的列索引
	CanadidateEdge(Edge* edge, double score = -1, int preColumnIndex = -1){
		this->edge = edge;
		this->score = score;
		this->preColumnIndex = preColumnIndex;
	}
};

//计算相邻轨迹点间连线与路段的夹角
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
		cout << "传入的相邻轨迹点顺序颠倒！" << endl;
		return -1;
	}
}

//计算得分（文中式(3)）
double CalculateScore(GeoPoint* formerTrajPoint, GeoPoint* trajPoint, Edge* edge){
	double tmp1 = pow(CalculateAngle(formerTrajPoint, trajPoint, edge), CA);
	double tmp2 = pow(map.distM(trajPoint->lat, trajPoint->lon, edge), CD);
	return UA*tmp1 - UD*tmp2;
}

//获取候选路段
void GetCanadidateEdges(list<GeoPoint*> &trajectory, vector<vector<CanadidateEdge>> &scoreMatrix){
	set<int> edgesSet = set<int>();
	set<int> endPointSet = set<int>();
	GeoPoint* formerTrajPoint = NULL;
	int trajPointIndex = 0;
	for each (GeoPoint* trajPoint in trajectory)
	{
		vector<CanadidateEdge> scores = vector<CanadidateEdge>();
		//第一步：获取原始候选路段
		//原文中为获取最近的50条路段，此处修改为获取18m范围内的所有路段
		//两者最重要的区别为：获取最近的50条路段可以保证最终候选路段总是非空的；而获取18m范围内的所有路段可能得到空集
		vector<Edge*> rawCanadidates = map.getKNearEdges(trajPoint->lat, trajPoint->lon, 50);
		//非起点，则使用文中提出的诸项剪枝方法
		if (trajPointIndex > 0){
			for each (Edge* rawEdge in rawCanadidates)
			{
				//剪枝方法：候选路段在前一轨迹点的候选路段集合内；或候选路段起点在前一轨迹点的候选路段的终点集合内（即保证至少一对候选路段存在直接相连性）
				//剪枝方法：轨迹点连线和候选路段首末点连线夹角小于90（即保证方向性）
				if ((edgesSet.count(rawEdge->id) == 1 || endPointSet.count(rawEdge->startNodeId) == 1)
					&& CalculateAngle(formerTrajPoint, trajPoint, rawEdge) > 0
					&& map.distM(trajPoint->lat, trajPoint->lon, rawEdge) <= 18){
					scores.push_back(CanadidateEdge(rawEdge));
				}
			}
			//如果由于剪枝把所有原始候选路段均剔除了，则换一种剪枝方法
			if (rawCanadidates.size() > 0 && scores.size() == 0){
				vector<CanadidateEdge> tmpScores = vector<CanadidateEdge>();
				double tmpMax = -1e10;
				//根据文中式(3)对每条候选路段进行打分，保留得分超过最大分80%的候选路段
				for each (Edge* var in rawCanadidates)
				{
					double tmpScore = CalculateScore(formerTrajPoint, trajPoint, var);
					tmpScores.push_back(CanadidateEdge(var, tmpScore));
					if (tmpScore > tmpMax){
						tmpMax = tmpScore;
					}
				}
				//要考虑到tmpMax为负的情况orz
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

//动态规划求解最小权值
list<Edge*> ShortestWeight(vector<vector<CanadidateEdge>> &scoreMatrix){
	list<Edge*> mapMatchingResult;//全局匹配路径
	vector<vector<CanadidateEdge>>::iterator loopIter = scoreMatrix.begin();
	vector<vector<CanadidateEdge>>::iterator formerIter = scoreMatrix.begin();
	int columnIndex = 0;
	//动态规划
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
	//回溯，记录全局最小权值解
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
	vector<vector<CanadidateEdge>> scoreMatrix = vector<vector<CanadidateEdge>>();//候选路段矩阵；行代表一个轨迹点；列代表一个候选路段
	//获取候选路段
	GetCanadidateEdges(trajectory, scoreMatrix);
	//调试代码：输出scoreMatrix以供查看
	//ofstream logOutput("DebugLog.txt");
	//logOutput << "scoreMatrix矩阵有" << scoreMatrix.size() << "行：" << endl;
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
	//调试结束
	//返回全局最小权值解，即全局匹配路径
	return ShortestWeight(scoreMatrix);
}

void main(){
	scanTrajFolder("D:\\Document\\Subjects\\Computer\\Develop\\Data\\SingaporeData\\", trajList, outputFileNames);
	int trajIndex = 0;
	cout << "开始地图匹配！" << endl;
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
		cout << "第" << trajIndex << "条轨迹匹配完毕！" << endl;
		trajIndex++;
	}
}