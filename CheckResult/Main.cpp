#include<iostream>
#include<fstream>
#include<string>
#include <direct.h>
#include <io.h>
#include<list>
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\新加坡数据\\";
string outputDirectory = "15days\\15days_separated_high_quality_120s_output_HMM";//轨迹文件所在文件夹路径。其中包含的轨迹文件名要求：以“input_”开头
string answerDirectory = "15days\\15days_separated_high_quality_120s_answer";//降低采样率后的轨迹文件所在文件夹路径
string resultFileName = "null";//检查结果输出文件名
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
void ReadFolder()
{
	/*文件目录结构为
	* folderDir
	* |-outputDirectory
	*   |-output_000011.txt ...
	* |-answerDirectory
	*   |-output_000011.txt ...
	*/
	string completeInputFilesPath = rootFilePath + outputDirectory + "\\" + "*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//文件信息
	long lf;//文件句柄
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		return;
	}
	else {
		do {
			string inputFileName = fileInfo.name;
			matchedResultList.push_back(ReadOneResultFile(rootFilePath + outputDirectory + "\\" + inputFileName));
			string answerFileName = inputFileName.substr(6, inputFileName.size() - 10);
			fileNameList.push_back(answerFileName);
			answerFileName = "output_" + answerFileName + ".txt";
			matchedAnswerList.push_back(ReadOneResultFile(rootFilePath + answerDirectory + "\\" + inputFileName));
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
	ofstream filePathFout;
	if (resultFileName != "null"){
		filePathFout = ofstream("files_sr_60.txt");
	}
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
		if (resultFileName != "null"&&tmpRate > 0.2){
			filePathFout << (*fileNameIter) << " " << tmpRate << endl;
		}
		averageErrorRate += tmpRate;
	}
	filePathFout.close();
	averageErrorRate /= matchedResultList.size();
	cout << "平均错误率：" << averageErrorRate << endl;
}

int main(int argc, char* argv[]){
	if (argc != 1 && argc != 3 && argc != 4){
		cout << "至多应该有三个参数：第一个为匹配结果文件所在文件夹路径，第二个为答案文件所在文件夹路径，第三个为检查结果输出文件名。" << endl;
	}
	else{
		if (argc == 3){
			outputDirectory = argv[1];
			answerDirectory = argv[2];
		}
		if (argc == 4){
			outputDirectory = argv[1];
			answerDirectory = argv[2];
			resultFileName = argv[3];
		}
		cout << "匹配结果文件所在文件夹路径：" << outputDirectory << endl;
		cout << "答案文件所在文件夹路径：" << answerDirectory << endl;
		ReadFolder();
		Check();
	}
	system("pause");
	return 0;
}