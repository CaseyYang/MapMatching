using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSVToTXT
{
    /// <summary>
    /// 坐标点类
    /// </summary>
    public class GeoPoint
    {
        /// <summary>
        /// 坐标点纬度
        /// </summary>
        private double lat;
        public double Lat
        {
            get { return lat; }
        }
        /// <summary>
        /// 坐标点经度
        /// </summary>
        private double lon;
        public double Lon
        {
            get { return lon; }
        }
        /// <summary>
        /// 坐标点时间戳
        /// </summary>
        private double time;
        public double Time
        {
            get { return time; }
        }
        /// <summary>
        /// 出租车状态
        /// </summary>
        private string cabStatus;
        public string CabStatus
        {
            get { return cabStatus; }
        }

        public GeoPoint()
        {
            lat = lon = time = -1;
            cabStatus = "";
        }
        public GeoPoint(double lat, double lon, double time, string cabStatus)
        {
            this.lat = lat;
            this.lon = lon;
            this.time = time;
            this.cabStatus = cabStatus;
        }

        public override string ToString()
        {
            return "lat=" + lat + ",lon=" + lon + ",time=" + time + ",cabStatus=" + cabStatus;
        }
    }
}
