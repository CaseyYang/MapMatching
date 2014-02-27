#include <stdio.h>
#include <time.h>
#include <direct.h>
#include <iomanip>
#include <io.h> 
#include <string.h>
#include "define.h"
#include "geometry.h"
#include "file.h"
#include "index.h"
#include "core.h"

//读入文件夹中所有轨迹文件的文件名
void getInputPathSet(char* path)
{
	//文件句柄  
	long hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	char tmpPath[1000];
	strcpy_s(tmpPath, path);
	strcat_s(tmpPath, "/*");
	if ((hFile = _findfirst(tmpPath, &fileinfo)) != -1) {
		do {
			//非文件夹，把Windows文件系统默认含有的“.”和“..”也排除在外
			if ((!(fileinfo.attrib & _A_SUBDIR))) {
				strcpy_s(inPathSet[dataNumber], path);
				strcat_s(inPathSet[dataNumber], "/");
				strcat_s(inPathSet[dataNumber], fileinfo.name);
				++dataNumber;
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
		printf("dataNumber=%d\n", dataNumber);
	}

	//struct dirent* ent = NULL;
	//DIR *dir;
	//dir = opendir(path);
	//while((ent = readdir(dir))!= NULL){ 
	//	if (ent->d_name[0]=='.') continue;
	//	strcpy(inPathSet[dataNumber],path);
	//	strcat(inPathSet[dataNumber],"/");
	//	strcat(inPathSet[dataNumber],ent->d_name);
	//	++dataNumber;
	//}
	//closedir(dir);
}

//根据输入路径构造输出文件名
void setOutputPathSet(char* path)
{
	int i, j, k;
	for (i = 0; i<dataNumber; ++i){
		strcpy_s(outPathSet[i], path);
		strcat_s(outPathSet[i], "/output");
		int len = strlen(inPathSet[i]);
		for (j = len - 1; j>0; --j) {
			if (inPathSet[i][j] == '_') {
				break;
			}
		}
		int len2 = strlen(outPathSet[i]);
		for (k = j; k < len; ++k) {
			outPathSet[i][len2 + k - j] = inPathSet[i][k];
		}
		outPathSet[i][len2 + k - j] = 0;
	}
}

//输出地图匹配结果，tt是地图匹配结果的序号
void outputResult(int& tt)
{
	int i;
	FILE *fp;
	fopen_s(&fp, outPathSet[tt], "w");
	for (i = 0; i < nowTime; ++i) {
		//timeSequence采样时间；ans匹配路段号
		fprintf(fp, "%d,%d,1.00\n", timeSequence[i], ans[i]);
	}
	fclose(fp);
}

int main(int argc, char *argv[])
{
	//4个参数：1：exe文件名；2：路网文件夹路径；3：轨迹文件夹路径；4：匹配结果文件夹路径
	//if (argc < 4) {
	//    puts("Argument Exception!");
	//    return 1;
	//}
	loadData("D:/Document/MDM Lab/Data/MapMatching数据");
	getInputPathSet("D:/Document/MDM Lab/Data/MapMatching数据/input");
	setOutputPathSet("D:/Document/MDM Lab/Data/MapMatching数据/output");

	resetCellSize();

	printf("reset cell size done\n");

	createGirdIndex();

	printf("before load gps\n");

	//地图匹配工作
	int i;
	for (i = 0; i < dataNumber; ++i) {
		printf("%s\n", inPathSet[i]);
		loadSequence(inPathSet[i]);
		printf("before DP\n");
		//inputLoaded[i] = 1;
		coreDP(dp, dpSize, dpEdge, dpLeft, dpLen, dpPre);
		printf("core DP done\n");
		//此句若不注释掉，在core DP done输出前就会报错
		//printf("i=%d, dataNumber=%d\n", i, dataNumber);
		outputResult(i);
	}
	printf("done\n");

	return 0;
}
