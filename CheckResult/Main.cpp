#include<iostream>
#include<fstream>
#include<string>
#include <direct.h>
#include <io.h>
#include<list>
using namespace std;

list<string> fileNameList = list<string>();//保存文件名集合
list<list<int>> matchedResultList = list<list<int>>();//保存匹配结果集合
list<list<int>> matchedAnswerList = list<list<int>>();//保存答案集合

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
void ReadFolder(string folderDir, string outputDirectory, string answerDirectory)
{
	/*文件目录结构为
	* folderDir
	* |-input
	*   |-input_000011.txt ...
	* |-output
	*   |-output_000011.txt ...
	*/
	string completeInputFilesPath = folderDir + outputDirectory + "\\" + "*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//文件信息
	long lf;//文件句柄
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		return;
	}
	else {
		do {
			string inputFileName = fileInfo.name;
			matchedResultList.push_back(ReadOneResultFile(folderDir + outputDirectory + "\\" + inputFileName));
			string answerFileName = inputFileName.substr(6, inputFileName.size() - 10);
			fileNameList.push_back(answerFileName);
			answerFileName = "output_" + answerFileName + ".txt";
			matchedAnswerList.push_back(ReadOneResultFile(folderDir + answerDirectory + "\\" + inputFileName));
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
	list<string>::iterator fileNameIter = fileNameList.begin();
	ofstream filePathFout = ofstream("files_sr_60.txt");
	for (; resultIter != matchedResultList.end(); resultIter++, answerIter++, fileNameIter++){
		int wrongCount = 0;
		list<int>::iterator resultEdgeIter = resultIter->begin();
		list<int>::iterator answerEdgeIter = answerIter->begin();
		for (; resultEdgeIter != resultIter->end(); resultEdgeIter++, answerEdgeIter++){
			if ((*resultEdgeIter) != (*answerEdgeIter)){
				wrongCount++;
			}
		}
		double tmpRate = (wrongCount + 0.0) / resultIter->size();
		//只输出匹配错误率大于0.2的轨迹文件名
		if (tmpRate > 0.2){
			filePathFout << (*fileNameIter) << " " << tmpRate << endl;
		}
		averageErrorRate += tmpRate;
	}
	filePathFout.close();
	averageErrorRate /= matchedResultList.size();
	cout << "平均错误率：" << averageErrorRate << endl;
}

int main(){
	ReadFolder("D:\\MapMatchingProject\\Data\\GISCUP2012_Data\\", "output_60", "answer_60");
	Check();
	system("pause");
	return 0;
}