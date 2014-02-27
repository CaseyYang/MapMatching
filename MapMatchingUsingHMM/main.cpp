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

//�����ļ��������й켣�ļ����ļ���
void getInputPathSet(char* path)
{
	//�ļ����  
	long hFile = 0;
	//�ļ���Ϣ  
	struct _finddata_t fileinfo;
	char tmpPath[1000];
	strcpy_s(tmpPath, path);
	strcat_s(tmpPath, "/*");
	if ((hFile = _findfirst(tmpPath, &fileinfo)) != -1) {
		do {
			//���ļ��У���Windows�ļ�ϵͳĬ�Ϻ��еġ�.���͡�..��Ҳ�ų�����
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

//��������·����������ļ���
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

//�����ͼƥ������tt�ǵ�ͼƥ���������
void outputResult(int& tt)
{
	int i;
	FILE *fp;
	fopen_s(&fp, outPathSet[tt], "w");
	for (i = 0; i < nowTime; ++i) {
		//timeSequence����ʱ�䣻ansƥ��·�κ�
		fprintf(fp, "%d,%d,1.00\n", timeSequence[i], ans[i]);
	}
	fclose(fp);
}

int main(int argc, char *argv[])
{
	//4��������1��exe�ļ�����2��·���ļ���·����3���켣�ļ���·����4��ƥ�����ļ���·��
	//if (argc < 4) {
	//    puts("Argument Exception!");
	//    return 1;
	//}
	loadData("D:/Document/MDM Lab/Data/MapMatching����");
	getInputPathSet("D:/Document/MDM Lab/Data/MapMatching����/input");
	setOutputPathSet("D:/Document/MDM Lab/Data/MapMatching����/output");

	resetCellSize();

	printf("reset cell size done\n");

	createGirdIndex();

	printf("before load gps\n");

	//��ͼƥ�乤��
	int i;
	for (i = 0; i < dataNumber; ++i) {
		printf("%s\n", inPathSet[i]);
		loadSequence(inPathSet[i]);
		printf("before DP\n");
		//inputLoaded[i] = 1;
		coreDP(dp, dpSize, dpEdge, dpLeft, dpLen, dpPre);
		printf("core DP done\n");
		//�˾�����ע�͵�����core DP done���ǰ�ͻᱨ��
		//printf("i=%d, dataNumber=%d\n", i, dataNumber);
		outputResult(i);
	}
	printf("done\n");

	return 0;
}
