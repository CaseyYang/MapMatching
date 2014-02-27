#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <io.h>  
#include <fcntl.h>  
#include <process.h>  
#include <sys/stat.h>
#include <string.h>
#include <share.h>
#include "define.h"

#define NODE_INPUT_FILE "/WA_Nodes.txt"
#define EDGE_INPUT_FILE "/WA_Edges.txt"
#define NET_INPUT_FILE "/WA_EdgeGeometry.txt"

/**
 * Power of 10
 */
static const double POWER10[50] = {
	1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
	1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
	1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29,
	1e30, 1e31, 1e32, 1e33, 1e34, 1e35, 1e36, 1e37, 1e38, 1e39,
	1e40, 1e41, 1e42, 1e43, 1e44, 1e45, 1e46, 1e47, 1e48, 1e49
};

char *readBuffer;		//读文件缓存
char *t;
int nowSize = 300000000;	//文件现在大小

void FileError(char *strErr)
{
	printf("Load File Error: %s\n", strErr);
	exit(1);
}

/**
 * Get Double from String
 */
static double P_ParseDouble()
{
	double val;
	int sign = 1, power;

	if (*t == '-') {
		++t;
		sign = -1;
	}
	else if (*t == '+') {
		++t;
	}
	for (val = 0.0; *t >= '0' && *t <= '9'; ++t)
		val = 10.0 * val + (*t - '0');
	if (*t == '.') {
		++t;
		for (power = 1; *t >= '0' && *t <= '9'; ++t) {
			val = 10.0 * val + (*t - '0');
			++power;
		}
		val /= POWER10[power - 1];
	}
	return sign * val;
}


/**
 * Get Integer from String
 */
static int P_ParseInteger()
{
	int result = 0;

	for (; *t >= '0' && *t <= '9'; ++t)
		result = result * 10 + *t - '0';
	return result;
}

/**
 * Load Nodes from file
 */
void LoadNode(char *path)
{
	char fullPath[1000];
	strcpy_s(fullPath, path);
	strcat_s(fullPath, NODE_INPUT_FILE);
	int pointerFile;
	_sopen_s(&pointerFile, fullPath, O_RDONLY, _SH_DENYWR, _S_IREAD);
	//int pointerFile = _sopen_s(fullPath, O_RDONLY);
	if (pointerFile == -1)
		FileError(NODE_INPUT_FILE "can not open!");
	int size = _read(pointerFile, readBuffer, nowSize);
	if (readBuffer[size - 1] != '\n')
		readBuffer[size++] = '\n';
	readBuffer[size++] = '\0';
	coordNode = (double *)malloc((originNodeNumber << 1) * sizeof(double));

	int j, k = -1;
	t = readBuffer;
	while (*t != '\0') {
		j = P_ParseInteger(); ++t;
		if (j > k) k = j;
		//coordNode数组保存路网节点（路口）信息，用偶数保存经度；用技术保存纬度（或者反之= =）
		coordNode[j << 1] = P_ParseDouble(); ++t;
		coordNode[(j << 1) + 1] = P_ParseDouble(); ++t;
	}
	nodeNumber = k + 1;

	_close(pointerFile);
}

/**
 * Load Edges from file
 */
void LoadEdge(char* path){
	char fullPath[1000];
	strcpy_s(fullPath, path);
	strcat_s(fullPath, EDGE_INPUT_FILE);
	int pointerFile;
	_sopen_s(&pointerFile, fullPath, O_RDONLY, _SH_DENYWR, _S_IREAD);
	//int pointerFile = _open(fullPath, O_RDONLY);
	if (pointerFile == -1)
		FileError(EDGE_INPUT_FILE "can not open!");
	int size = _read(pointerFile, readBuffer, nowSize);
	if (readBuffer[size - 1] != '\n')
		readBuffer[size++] = '\n';
	readBuffer[size++] = '\0';

	int index_size = (originEdgeNumber << 1) * sizeof(int);
	pre = (int*)malloc(index_size);
	other = (int*)malloc(index_size);
	thisSide = (int*)malloc(index_size);
	cost = (int*)malloc(index_size);
	last = (int*)malloc(index_size);
	memset(last, 255, index_size);

	int j, k = -1;
	int r1, r2, l;
	t = readBuffer;
	//每个路口关联的路段以邻接表的形式保存，邻接表又以数组的形式来实现
	//每个路口会关联一系列的路段，last数组记录每个路口关联的最后一个路段id
	//pre记录每个路段在邻接表中的前一个路段在数组中的索引
	//thisside存的是每条路段的起点；other存的是路段
	//pre、thisside、other、cost的数组下标都是边的下标，是一致的
	//和索引中单元格所保存的路网信息的方式类似！！！！
	while (*t != '\0') {
		j = P_ParseInteger(); ++t;//j是序号
		if (j > k) k = j;
		r1 = P_ParseInteger(); ++t;
		r2 = P_ParseInteger(); ++t;
		l = P_ParseInteger(); ++t;
		//last存的是和某个路口向关联的最后一条路段的序号
		pre[j] = last[r1];
		last[r1] = j;
		cost[j] = l;
		other[j] = r2;
		thisSide[j] = r1;
	}
	edgeNumber = k + 1;

	_close(pointerFile);
}

/**
 * Load Edge Geometry from file
 */
void LoadNet(char* path)
{
	char fullPath[1000];
	strcpy_s(fullPath, path);
	strcat_s(fullPath, NET_INPUT_FILE);
	int pointerFile;
	_sopen_s(&pointerFile, fullPath, O_RDONLY, _SH_DENYWR, _S_IREAD);
	//int pointerFile = _open(fullPath, O_RDONLY);
	if (pointerFile == -1)
		FileError(NET_INPUT_FILE "can not open!");
	int size = _read(pointerFile, readBuffer, nowSize);
	if (readBuffer[size - 1] != '\n') {
		readBuffer[size++] = '\n';
	}
	readBuffer[size++] = '\0';
	double x, y;
	edgeStart = (int*)malloc(originEdgeNumber * sizeof(int));
	coordNet = (double*)malloc((originLineNumber << 1) * sizeof(double));
	len = (double*)malloc(originLineNumber * sizeof(double));

	int i = 0, j, m = 0;
	t = readBuffer;
	//separate the first loop from others to simplfy the loop
	//读入EdgeGeometry.txt中第一行记录
	{
		j = P_ParseInteger(); ++t;//j是序号
		for (; *t != '^'; ++t); ++t;
		for (; *t != '^'; ++t); ++t;
		len[j] = P_ParseDouble(); ++t;//路段长度
		edgeStart[j] = m;//edgeStart数组保存路段起点的ID。注意无论NodeID还是EdgeID都是从0开始连续递增，所以id可以拿来直接当索引量用
		//读入EdgeGeometry.txt中第一行第一组记录
		{
			x = P_ParseDouble(); ++t;
			y = P_ParseDouble();
			//coordNet数组保存每个路段中每条边的信息
			//方法如下：每个路段中边的信息在coordNet中连续保存，占用空间为edgeStart[j]到edgeStart[j+1]-1。
			//其中偶数保存经度；用技术保存纬度（或者反之= =）。所以占用空间必然为偶数个位置
			coordNet[m << 1] = x;
			coordNet[(m << 1) + 1] = y;
			minLat = maxLat = x;
			minLong = maxLong = y;
			++m;
		}
		//读入EdgeGeometry.txt中第一行其余各组记录
		while (*(t++) == '^') {
			x = P_ParseDouble(); ++t;
			y = P_ParseDouble();
			coordNet[m << 1] = x;
			coordNet[(m << 1) + 1] = y;
			if (x < minLat)
				minLat = x;
			else if (x > maxLat)
				maxLat = x;
			if (y < minLong)
				minLong = y;
			else if (y > maxLong)
				maxLong = y;
			++m;
		}
	}
	//读入EdgeGeometry.txt中其余各行记录
	while (*t != '\0') {
		j = P_ParseInteger(); ++t;
		for (; *t != '^'; ++t); ++t;
		for (; *t != '^'; ++t); ++t;
		len[j] = P_ParseDouble();
		edgeStart[j] = m;
		while (*(t++) == '^') {
			x = P_ParseDouble(); ++t;
			y = P_ParseDouble();
			coordNet[m << 1] = x;
			coordNet[(m << 1) + 1] = y;
			if (x < minLat)
				minLat = x;
			else if (x > maxLat)
				maxLat = x;
			if (y < minLong)
				minLong = y;
			else if (y > maxLong)
				maxLong = y;
			++m;
		}
	}
	++j;
	edgeStart[j] = m;
	_close(pointerFile);
	lineNumber = m;
}

/**
 * Load Test File
 */
void loadSequence(char* str){
	int tRate = 0;//采样率
	int pointerFile;
	_sopen_s(&pointerFile, str, O_RDONLY, _SH_DENYWR, _S_IREAD);
	//int pointerFile = _open(str, O_RDONLY);
	if (pointerFile == -1) {
		FileError("test file can not open!");
	}
	int size = _read(pointerFile, readBuffer, nowSize);

	if (readBuffer[size - 1] != '\n') {
		readBuffer[size++] = '\n';
	}
	readBuffer[size++] = '\0';
	int i = 2, j = 1;
	t = readBuffer;
	timeSequence[0] = P_ParseInteger(); ++t;
	nodeSequence[0] = P_ParseDouble(); ++t;
	nodeSequence[1] = P_ParseDouble(); ++t;
	while (*t != '\0') {
		timeSequence[j] = P_ParseInteger(); ++t;
		nodeSequence[i++] = P_ParseDouble(); ++t;
		nodeSequence[i++] = P_ParseDouble(); ++t;
		tRate += (timeSequence[j] - timeSequence[j - 1]);
		++j;
	}
	nowTime = j;
	if (j < 2) j = 2;
	tRate /= (j - 1);
	if (tRate < 1)
		tRate = 1;
	else if (tRate > 30)
		tRate = 30;
	rate = tRate;

	_close(pointerFile);
}

/**
 * Load Road Network Data
 */
void loadData(char *path)
{
	readBuffer = (char *)malloc(nowSize);
	LoadNode(path);
	LoadEdge(path);
	LoadNet(path);
}

void FreeBuffer()
{
	printf("Free buffer!\n");
	free(readBuffer);
}