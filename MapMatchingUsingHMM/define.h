#define PI 3.1415926535898
#define EARTHRAD 6372797												//地球半径（平均）
#define PI180 0.0174532925199432957694
#define N_MAX_NODE 1000000
#define LENGTH_PER_RAD 111226.29021121707545
#define CANADIDATEEDGES 1000

extern double *coordNode;
extern int *edgeStart;
extern double *coordNet;
extern double maxLat, minLat, maxLong, minLong;

extern int nodeNumber;
extern int edgeNumber;
extern int lineNumber;
extern int originNodeNumber;
extern int originEdgeNumber;
extern int originLineNumber;

extern int *pre, *other, *thisSide, *last, *cost, *typeList;
extern double *len;

extern int roadTypeNumber;
extern char roadTypeCurrentList[][30];

extern int rate;

extern int nowTime;
extern int timeSequence[20000];
extern double nodeSequence[20000];
extern int ans[20000];

extern int indexPre[], indexOther[], indexLast[];
extern int crossSize;

extern long double dp[][CANADIDATEEDGES];								//动态规划概率
extern int dpSize[];									//每个点附近有多少边
extern int dpEdge[][CANADIDATEEDGES];								//动态规划记录边的Id
extern double dpLeft[][CANADIDATEEDGES];							//动态规划剩余
extern double dpLen[][CANADIDATEEDGES];							//动态规划每条边的几何计算长度
extern int dpPre[][CANADIDATEEDGES];								//动态规划记录路径


extern double BETA_ARR[];

extern int nodeInsertedToIndex[];					//每个点是否被插入了邻接表
extern int indexCreateFlag;
extern int doneFlag;
//extern int inputLoaded[];
extern int dataNumber;
extern char inPathSet[][100];//输入路径集合
extern char outPathSet[][100];//输出路径集合
extern int gridFlag;

extern int DijkstraIndex[];
//extern int DijkstraIndex2[];

