using System;
using System.Collections.Generic;
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
            Console.WriteLine(args.Length);
            string csvFIlePath="";
            StreamReader fileReader = new StreamReader(csvFIlePath);
            while (!fileReader.EndOfStream)
            {
                string rawStr=fileReader.ReadLine();
                string[] strs = rawStr.Split(',');

            }
        }
    }
}
