#include<iostream>
#include<fstream>
#include<string>
#include <direct.h>
#include <io.h>
#include<list>
using namespace std;

string rootFilePath = "D:\\MapMatchingProject\\Data\\�¼�������\\";
string outputDirectory = "15days\\15days_separated_high_quality_120s_output_HMM";//�켣�ļ������ļ���·�������а����Ĺ켣�ļ���Ҫ���ԡ�input_����ͷ
string answerDirectory = "15days\\15days_separated_high_quality_120s_answer";//���Ͳ����ʺ�Ĺ켣�ļ������ļ���·��
string resultFileName = "null";//���������ļ���
list<string> fileNameList = list<string>();//�����ļ�������
list<list<int>> matchedResultList = list<list<int>>();//����ƥ��������
list<list<int>> matchedAnswerList = list<list<int>>();//����𰸼���

//�������·����һ����ͼƥ�����ļ�
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

//��������ƥ�����ļ�����Ӧ�Ĵ��ļ�
void ReadFolder()
{
	/*�ļ�Ŀ¼�ṹΪ
	* folderDir
	* |-outputDirectory
	*   |-output_000011.txt ...
	* |-answerDirectory
	*   |-output_000011.txt ...
	*/
	string completeInputFilesPath = rootFilePath + outputDirectory + "\\" + "*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//�ļ���Ϣ
	long lf;//�ļ����
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


//�Ƚ�ƥ�����ʹ�
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
		//ֻ���ƥ������ʴ���0.2�Ĺ켣�ļ���
		if (resultFileName != "null"&&tmpRate > 0.2){
			filePathFout << (*fileNameIter) << " " << tmpRate << endl;
		}
		averageErrorRate += tmpRate;
	}
	filePathFout.close();
	averageErrorRate /= matchedResultList.size();
	cout << "ƽ�������ʣ�" << averageErrorRate << endl;
}

int main(int argc, char* argv[]){
	if (argc != 1 && argc != 3 && argc != 4){
		cout << "����Ӧ����������������һ��Ϊƥ�����ļ������ļ���·�����ڶ���Ϊ���ļ������ļ���·����������Ϊ���������ļ�����" << endl;
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
		cout << "ƥ�����ļ������ļ���·����" << outputDirectory << endl;
		cout << "���ļ������ļ���·����" << answerDirectory << endl;
		ReadFolder();
		Check();
	}
	system("pause");
	return 0;
}