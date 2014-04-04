/* 
 * Last Updated at [2014/2/10 11:13] by wuhao
 */
#include "MapDrawer.h"


MapDrawer::MapDrawer()
{
	//GDI+ initializaton
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

MapDrawer::~MapDrawer()
{
	//GDI+ 收尾工作
	delete bm;
	delete bmData;
	GdiplusShutdown(gdiplusToken);
}

//////////////////////////////////////////////////////////////////////////
///public part
//////////////////////////////////////////////////////////////////////////
void MapDrawer::setArea(double _minLat, double _maxLat, double _minLon, double _maxLon)
{
	minLat = _minLat;
	maxLat = _maxLat;
	minLon = _minLon;
	maxLon = _maxLon;
}

/*void MapDrawer::setArea(Map& map)
{
	minLat = map.minLat;
	minLon = map.minLon;
	maxLat = map.maxLat;
	maxLon = map.maxLon;
}*/

void MapDrawer::setResolution(int width)
{
	r_width = width;
	double height = (maxLat - minLat) / (maxLon - minLon) * width;
	r_height = (int)height;
}

void MapDrawer::setResolution(int width, int height)
{
	r_width = width;
	r_height = height;
}

void MapDrawer::newBitmap()
{
	bm = new Bitmap(r_width, r_height, PixelFormat32bppARGB);
}

void MapDrawer::lockBits()
{
	bmData = new BitmapData;
	bm->LockBits(new Rect(0, 0, r_width, r_height),
		ImageLockModeRead | ImageLockModeWrite, PixelFormat32bppARGB, bmData);
}

void MapDrawer::unlockBits()
{
	bm->UnlockBits(bmData);
}

void MapDrawer::drawPoint(Color color, int x, int y)
{
	if (x >= r_width || y >= r_height || x < 0 || y < 0)
	{
		printf("drawing pt(%d, %d) error, bitmap resolution is %d * %d\n",
			x, y, r_width, r_height);
		system("pause");
		exit(0);
	}
	byte* row = (byte*)bmData->Scan0 + (y * bmData->Stride);
	int x0 = x * 4;
	int x1 = x0 + 1;
	int x2 = x1 + 1;
	int x3 = x2 + 1;
	row[x0] = color.GetB();
	row[x1] = color.GetG();
	row[x2] = color.GetR();
	row[x3] = color.GetA();
}

void MapDrawer::drawPoint(Color color, double lat, double lon)
{
	if (inArea(lat, lon))
	{
		Point pt = geoToScreen(lat, lon);
		drawPoint(color, pt.X, pt.Y);
	}
}

void MapDrawer::drawBigPoint(Color color, double lat, double lon)
{
	if (inArea(lat, lon))
	{
		Point pt = geoToScreen(lat, lon);
		if (pt.X >= 1)
			drawPoint(color, pt.X - 1, pt.Y);
		if (pt.X <= r_width - 2)
			drawPoint(color, pt.X + 1, pt.Y);
		if (pt.Y >= 1)
			drawPoint(color, pt.X, pt.Y - 1);
		if (pt.Y <= r_height - 2)
			drawPoint(color, pt.X, pt.Y + 1);
		drawPoint(color, pt.X, pt.Y);
	}
}

void MapDrawer::drawBigPoint(Color color, int x, int y)
{
	Point pt = Point(x, y);
	if (pt.X >= 1)
		drawPoint(color, pt.X - 1, pt.Y);
	if (pt.X <= r_width - 2)
		drawPoint(color, pt.X + 1, pt.Y);
	if (pt.Y >= 1)
		drawPoint(color, pt.X, pt.Y - 1);
	if (pt.Y <= r_height - 2)
		drawPoint(color, pt.X, pt.Y + 1);
	drawPoint(color, pt.X, pt.Y);
}

void MapDrawer::drawLine(Color color, int x1, int y1, int x2, int y2)
{
	if (x1 == r_width)
		x1 = r_width;
	if (x1 == -1)
		x1 = 0;
	if (x2 == r_width)
		x2 = r_width;
	if (x2 == -1)
		x2 = 0;
	if (abs(x1 - x2) >= abs(y1 - y2))
		bresenhamDrawLine_x(color, x1, y1, x2, y2);
	else
		bresenhamDrawLine_y(color, x1, y1, x2, y2);
}

void MapDrawer::drawLine(Color color, double lat1, double lon1, double lat2, double lon2)
{
	if (inArea(lat1, lon1) && inArea(lat2, lon2))
	{
		Point pt1, pt2;
		pt1 = geoToScreen(lat1, lon1);
		pt2 = geoToScreen(lat2, lon2);
		drawLine(color, pt1.X, pt1.Y, pt2.X, pt2.Y);
	}
}

void MapDrawer::drawBoldLine(Color color, double lat1, double lon1, double lat2, double lon2)
{
	if (inArea(lat1, lon1) && inArea(lat2, lon2))
	{
		Point pt1, pt2, pt3, pt4, pt5, pt6;
		pt1 = geoToScreen(lat1, lon1);
		pt2 = geoToScreen(lat2, lon2);
		drawLine(color, pt1.X, pt1.Y, pt2.X, pt2.Y);
		pt3.X = pt1.X + 1;
		pt5.X = pt1.X - 1;
		pt5.Y = pt3.Y = pt1.Y;
		pt4.X = pt2.X + 1;
		pt6.X = pt2.X - 1;
		pt6.Y = pt4.Y = pt2.Y;
		drawLine(color, pt3.X, pt3.Y, pt4.X, pt4.Y);
		drawLine(color, pt5.X, pt5.Y, pt6.X, pt6.Y);
		pt3.Y = pt1.Y + 1;
		pt5.Y = pt1.Y - 1;
		pt5.X = pt3.X = pt1.X;
		pt4.Y = pt2.Y + 1;
		pt6.Y = pt2.Y - 1;
		pt6.X = pt4.X = pt2.X;
		drawLine(color, pt3.X, pt3.Y, pt4.X, pt4.Y);
		drawLine(color, pt5.X, pt5.Y, pt6.X, pt6.Y);
	}
}

Color randomColor()
{
	int r = int(((double)rand()) / RAND_MAX * 255);
	int g = int(((double)rand()) / RAND_MAX * 255);
	int b = int(((double)rand()) / RAND_MAX * 255);
	Color color((byte)r, (byte)g, (byte)b);
	return color;
}

void MapDrawer::drawMap(Color color, std::string mapFilePath)
{
	/************************************************************************/
	/* OpenStreetMap格式说明
	/* WA_EdgeGeometry文件
	/* id^^Highway^1^起始端点纬度^起始端点经度[^中间点1纬度^中间点1经度^中间点2纬度^中间点2经度.....]^结束端点纬度^结束端点经度*/
	/************************************************************************/

	std::ifstream ifs(mapFilePath);
	if (!ifs)
	{
		std::cout << "open " + mapFilePath + " error!\n" ;
		return;
	}
	std::string strLine;
	while (getline(ifs, strLine))
	{
		std::vector<std::string> substrs;
		split(strLine, "^", substrs);
		double lat1, lon1, lat2, lon2;
		lat2 = atof(substrs[3].c_str());
		lon2 = atof(substrs[4].c_str());
		Color color_ = randomColor();
		for (int i = 3; i < substrs.size() - 3; i+=2)
		{
			lat1 = lat2;
			lon1 = lon2;
			lat2 = atof(substrs[i + 2].c_str());
			lon2 = atof(substrs[i + 3].c_str());
			drawLine(color_, lat1, lon1, lat2, lon2);
			drawBigPoint(Color::Black, lat1, lon1);
			drawBigPoint(Color::Black, lat2, lon2);
		}
	}
	ifs.close();
}

void MapDrawer::saveBitmap(std::string fileName)
{

	CLSID pngClsid;
	GetEncoderClsid(L"image/png", &pngClsid);
	bm->Save(CharToWchar(fileName.c_str()), &pngClsid, NULL);
}

bool MapDrawer::inArea(double lat, double lon)
{
	return (lat > minLat && lat < maxLat && lon > minLon && lon < maxLon);
}

//北半球
void MapDrawer::zoomIn(int upperLeft_x, int upperLeft_y, int width, int height, int newR_width)
{
	double scaleY = (maxLat - minLat) / r_height;
	double scaleX = (maxLon - minLon) / r_width;
	double newMinLat, newMaxLat, newMinLon, newMaxLon;
	newMaxLat = maxLat - upperLeft_y * scaleY;
	newMinLat = newMaxLat - height * scaleY;
	newMinLon = minLon + upperLeft_x * scaleX;
	newMaxLon = newMinLon + width * scaleX;
	minLat = newMinLat;
	maxLat = newMaxLat;
	minLon = newMinLon;
	maxLon = newMaxLon;
	setResolution(newR_width);
}

Point MapDrawer::geoToScreen(double lat, double lon)
{
	int x = (lon - minLon) / ((maxLon - minLon)  / (double)r_width);
	int y = (maxLat -lat) / ((maxLat - minLat) / (double)r_height); //屏幕Y轴是向下递增的
	Point pt(x, y);
	return pt;
}

std::pair<double, double> MapDrawer::screenToGeo(int screenX, int screenY)
{
	double scale = r_width / (maxLon - minLon);
	double lat = maxLat - (double)screenY / (double)r_height * (maxLat - minLat);
	double lon = (double)screenX / (double)r_width * (maxLon - minLon) + minLon;
	return std::make_pair(lat, lon);
}

//////////////////////////////////////////////////////////////////////////
///private part
//////////////////////////////////////////////////////////////////////////
int MapDrawer::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

void MapDrawer::bresenhamDrawLine_x(Color color, int x1, int y1, int x2, int y2)
{
	int _x1, _y1, _x2, _y2;
	if (x1 < x2)
	{
		_x1 = x1;
		_y1 = y1;
		_x2 = x2;
		_y2 = y2;
	}
	else
	{
		_x1 = x2;
		_y1 = y2;
		_x2 = x1;
		_y2 = y1;
	}
	int x, y, dx, dy, e;
	dx = _x2 - _x1;
	dy = abs(_y2 - _y1);
	e = -dx;
	x = _x1;
	y = _y1;
	for (int i = 0; i <= dx; i++)
	{
		drawPoint(color, x, y);
		x++;
		e += 2 * dy;
		if (e >= 0)
		{
			if (_y1 > _y2)
				y--;
			else
				y++;
			e -= 2 * dx;
		}
	}
}

void MapDrawer::bresenhamDrawLine_y(Color color, int x1, int y1, int x2, int y2)
{
	int _x1, _y1, _x2, _y2;
	if (y1 < y2)
	{
		_x1 = x1;
		_y1 = y1;
		_x2 = x2;
		_y2 = y2;
	}
	else
	{
		_x1 = x2;
		_y1 = y2;
		_x2 = x1;
		_y2 = y1;
	}
	int x, y, dx, dy, e;
	dy = _y2 - _y1;
	dx = abs(_x2 - _x1);
	e = -dy;
	y = _y1;
	x = _x1;
	for (int i = 0; i <= dy; i++)
	{
		drawPoint(color, x, y);
		y++;
		e += 2 * dx;
		if (e >= 0)
		{
			if (_x1 > _x2)
				x--;
			else
				x++;
			e -= 2 * dy;
		}
	}
}

wchar_t* MapDrawer::CharToWchar(const char* c)
{
	int len = MultiByteToWideChar(CP_ACP, 0, c, strlen(c), NULL, 0);
	wchar_t* m_wchar = new wchar_t[len + 1];
	MultiByteToWideChar(CP_ACP, 0, c, strlen(c), m_wchar, len);
	m_wchar[len] = '\0';
	return m_wchar;
}

void MapDrawer::split(const std::string& src, const std::string& separator, std::vector<std::string>& dest)
{
	std::string str = src;
	std::string substring;
	std::string::size_type start = 0, index;
	do
	{
		index = str.find_first_of(separator, start);
		if (index != std::string::npos)
		{
			substring = str.substr(start, index - start);
			dest.push_back(substring);
			start = str.find_first_not_of(separator, index);
			if (start == std::string::npos) return;
		}
	} while (index != std::string::npos);
	//the last token
	substring = str.substr(start);
	dest.push_back(substring);
}