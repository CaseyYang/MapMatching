#include<iostream>
#include<fstream>
#include <direct.h>
#include <io.h>
#include<list>
using namespace std;

list<list<int>> matchedResultList = list<list<int>>();
list<list<int>> matchedAnswerList = list<list<int>>();

//读入给定路径的一个地图匹配结果文件
list<int> ReadOneResultFile(string filePath){
	ifstream fin(filePath);
	list<int> result = list<int>();
	int time, edgeId;
	double confidence;
	char useless;
	while (fin >> time){
		fin >> useless >> edgeId >> useless >> confidence;
		result.push_back(edgeId);
	}
	fin.close();
	return result;
}

//读入所有匹配结果文件和相应的答案文件
void ReadFolder(string folderDir)
{
	/*文件目录结构为
	* folderDir
	* |-input
	*   |-input_000011.txt ...
	* |-output
	*   |-output_000011.txt ...
	*/
	string completeInputFilesPath = folderDir + "output\\" + "*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//文件信息
	long lf;//文件句柄
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		return;
	}
	else {
		do {
			string inputFileName = fileInfo.name;
			matchedResultList.push_back(ReadOneResultFile(folderDir + "output\\" + inputFileName));
			string answerFileName = inputFileName.substr(6, inputFileName.size() - 10);
			answerFileName = "output_" + answerFileName + ".txt";
			matchedAnswerList.push_back(ReadOneResultFile(folderDir + "answer\\" + inputFileName));
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		return;
	}
}

//比较匹配结果和答案
void Check(){
	double averageErrorRate = 0;
	list<list<int>>::iterator resultIter = matchedResultList.begin();
	list<list<int>>::iterator answerIter = matchedAnswerList.begin();
	for (; resultIter != matchedResultList.end(); resultIter++, answerIter++){
		int wrongCount = 0;
		list<int>::iterator resultEdgeIter = resultIter->begin();
		list<int>::iterator answerEdgeIter = answerIter->begin();
		for (; resultEdgeIter != resultIter->end(); resultEdgeIter++, answerEdgeIter++){
			if ((*resultEdgeIter) != (*answerEdgeIter)){
				wrongCount++;
			}
		}
		double tmpRate = (wrongCount + 0.0) / resultIter->size();
		cout << tmpRate << endl;
		averageErrorRate += tmpRate;
	}
	averageErrorRate /= matchedResultList.size();
	cout << "平均错误率：" << averageErrorRate << endl;
}

int main(){
	ReadFolder("D:\\Document\\Subjects\\Computer\\Develop\\Data\\GISCUP2012_Data\\");
	Check();
	return 0;
}