#define PI 3.1415926535898
#define EARTHRAD 6372797												//����뾶��ƽ����
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

extern long double dp[][CANADIDATEEDGES];								//��̬�滮����
extern int dpSize[];									//ÿ���㸽���ж��ٱ�
extern int dpEdge[][CANADIDATEEDGES];								//��̬�滮��¼�ߵ�Id
extern double dpLeft[][CANADIDATEEDGES];							//��̬�滮ʣ��
extern double dpLen[][CANADIDATEEDGES];							//��̬�滮ÿ���ߵļ��μ��㳤��
extern int dpPre[][CANADIDATEEDGES];								//��̬�滮��¼·��


extern double BETA_ARR[];

extern int nodeInsertedToIndex[];					//ÿ�����Ƿ񱻲������ڽӱ�
extern int indexCreateFlag;
extern int doneFlag;
//extern int inputLoaded[];
extern int dataNumber;
extern char inPathSet[][100];//����·������
extern char outPathSet[][100];//���·������
extern int gridFlag;

extern int DijkstraIndex[];
//extern int DijkstraIndex2[];

