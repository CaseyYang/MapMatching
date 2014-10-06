using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSVToTXT
{
    class Program
    {
        static void Main(string[] args)
        {
            string csvFilePath, trajsDirectory;
            if (args.Length == 0)
            {
                csvFilePath = "logs_20090214_20090215.csv";
                trajsDirectory = @"D:\MapMatchingProject\Data\新加坡数据\test";
            }
            else
            {
                if (args.Length == 2)
                {
                    csvFilePath = args[0];
                    trajsDirectory = args[1];
                }
                else
                {
                    Console.WriteLine("参数错误！至多输入两个参数：第一个为csv文件路径，第二个为输出轨迹文件所在文件夹路径");
                    Console.Write("程序未执行，按任意键退出……");
                    Console.Read();
                    return;
                }
            }
            Dictionary<string, List<GeoPoint>> trajSet = new Dictionary<string, List<GeoPoint>>();
            #region 读入csv文件，整理原始数据并保存至trajSet中
            StreamReader fileReader = new StreamReader(csvFilePath);
            while (!fileReader.EndOfStream)
            {
                string rawStr = fileReader.ReadLine();
                string[] strs = rawStr.Split(',');
                DateTime date = DateTime.Parse(strs[0], new CultureInfo("fr-FR", false));//反正只有fr-FR才能正确解析，zh-CN或者en-US都不行= =
                DateTime baseline = new DateTime(date.Year, date.Month, date.Day);
                TimeSpan seconds = date - baseline;
                int time = (int)seconds.TotalSeconds;
                double lat = Double.Parse(strs[4]);
                double lon = Double.Parse(strs[3]);
                if (!trajSet.ContainsKey(strs[1]))
                {
                    trajSet.Add(strs[1], new List<GeoPoint>());
                }
                if (trajSet[strs[1]].Count == 0 || trajSet[strs[1]].Last<GeoPoint>().Time <= time)
                {
                    trajSet[strs[1]].Add(new GeoPoint(lat, lon, time, strs[6]));
                }
                else
                {
                    Console.WriteLine("时间出错！");
                    Console.Read();
                }
            }
            fileReader.Close();
            #endregion
            #region 输出trajSet中所有数据到txt中，按照time,lat,lon的格式
            int fileIndex = 0;
            foreach (KeyValuePair<string, List<GeoPoint>> pair in trajSet)
            {
                StreamWriter fileWriter = new StreamWriter(@trajsDirectory + @"\input_" + fileIndex + ".txt");
                foreach (var geoPoint in pair.Value)
                {
                    fileWriter.WriteLine(geoPoint.Time + "," + geoPoint.Lat + "," + geoPoint.Lon);
                }
                fileWriter.Close();
                ++fileIndex;
            }
            #endregion
            Console.WriteLine("所有轨迹文件输出完毕！按任意键退出……");
            Console.Read();
            return;
        }
    }
}
