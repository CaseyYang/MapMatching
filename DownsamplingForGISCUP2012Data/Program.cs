using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DownsamplingForGISCUP2012Data
{
    class Program
    {
        static bool CheckFileName(string name1,string name2){
            string tmp1 = name1.Substring(6);
            string tmp2 = name2.Substring(7);
            return tmp1.Equals(tmp2);
        }
        static void Main(string[] args)
        {
            int targetSampling = 120;
            DirectoryInfo inputDirectory = new DirectoryInfo(@"E:\MapMatchingProject\Data\GISCUP2012_Data\origin\input");
            DirectoryInfo answerDirecory = new DirectoryInfo(@"E:\MapMatchingProject\Data\GISCUP2012_Data\origin\answer");
            FileInfo[] answerFiles = answerDirecory.GetFiles();
            int count = 0;
            foreach (FileInfo file in inputDirectory.GetFiles())
            {
                FileInfo answerFile = answerFiles[count];
                if (CheckFileName(file.Name, answerFile.Name))
                {

                }
                else
                {
                    Console.WriteLine("文件名不一致！");
                    return;
                }
                ++count;
            }
        }
    }
}
