/* 
 * Last Updated at [2014/3/4 15:09] by wuhao
 */

/*
 *	//ʹ�÷���
 *	MapDrawer md;
 *	md.setArea(minLat, maxLat, minLon, maxLon); //��������Χ
 *	md.setResolution(size); //ͼƬ���,��λΪ����,�߶�����Ӧ
 *	md.newBitmap(); //������ͼƬ,Ĭ�ϱ���Ϊ͸��
 *	md.lockBits(); //�κλ�ͼ����ǰ����������
 *	drawsth(); //���л�ͼ����Ӧ�ü������
 * 	md.unlockBits(); //���л�ͼ������ɺ���ϴ˾����ύ����
 *  md.saveBitmap("out.png"); //���ΪpngͼƬ
 *
 *  //zoomingʹ�÷���
 *  size = 15000;
 *  md.setArea(minLat, maxLat, minLon, maxLon)
 *  md.setResolution(size);
 *  size = 3000;
 *  md.zoomIn(8900, 7150, 400, 300, size); //��15000��ȵ�ͼƬ�о۽���(8900,7150)Ϊ���Ͻǣ�400*300��С�����У�������ͼ����Ϊ3000
 *  md.newBitmap();
 *  ......
 */
#pragma once
#include <string>
#include <vector>
#include <Windows.h>  
#include <tchar.h>  
#include <stdio.h>  
#include <gdiplus.h>
#include <io.h>
#include <fstream>
#include <iostream>
using namespace Gdiplus;
#pragma comment(lib,"gdiplus.lib")


class MapDrawer
{
public:
	double minLat;
	double maxLat;
	double minLon;
	double maxLon;
	int r_width; //�������ؿ��
	int r_height; //�������ظ߶�

	MapDrawer();
	~MapDrawer();

	void drawPoint(Color color, int x, int y); //����Ļ(x,y)��һ�����ص�
	void drawPoint(Color color, double lat, double lon); //�ڵ�������(lat,lon)��Ӧ��ͼ�ϻ�һ�����ص�
	void drawBigPoint(Color color, int x, int y); //����Ļ(x,y)��һ��ʮ�ֵ�
	void drawBigPoint(Color color, double lat, double lon); //�ڵ�������(lat,lon)��Ӧ��ͼ�ϻ�һ��ʮ�ֵ�
	void drawLine(Color color, int x1, int y1, int x2, int y2); 
	void drawLine(Color color, double lat1, double lon1, double lat2, double lon2);
	void drawBoldLine(Color color, double lat1, double lon1, double lat2, double lon2); //��һ������
	void drawMap(Color color, std::string mapFilePath); //����ͼ��mapFilePathΪ��ͼ�ļ�·������OSM��׼��ʽ

	void newBitmap(); //�½���������ͼǰ����ã��������趨��area��resolution
	void lockBits(); //��ͼǰ�����ȵ������������newBitmap()����֮�����
	void unlockBits(); //���л�ͼ������������������������ύ����
	void saveBitmap(std::string fileName); //����ΪpngͼƬ��fileNameΪ�ļ�·��
	void setArea(double _minLat, double _maxLat, double _minLon, double _maxLon); //���ù켣��Ч�����ڴ˷�Χ�ڵĹ켣���ᱻ���ڻ�����
//	void setArea(Map& map); //������Ч������map����һ��
	void setResolution(int width); //���û�����ȣ��߶��ɾ�γ�����򰴱����Զ�����	
	void setResolution(int width, int height); //�Զ��廭�������Ƽ�ʹ��
	bool inArea(double lat, double lon); //�жϹ켣��γ���Ƿ���ָ����ͼ������
	void zoomIn(int upperLeft_x, int upperLeft_y, int width, int height, int newR_width); //��ֻ�ʺ����ڱ����򣡡��Ŵ��������Ͻ���������;��γ���Ϊ�������룬newR_widthΪ�»������
	
	Point geoToScreen(double lat, double lon); //����������ת������Ļ����,��������ΪGdiplus::Point
	std::pair<double, double> screenToGeo(int screenX, int screenY); //����Ļ����ת���ɵ�������,����pair��firstΪlat,secondΪlon

private:
	ULONG_PTR gdiplusToken;
	Bitmap* bm;
	BitmapData* bmData;
	
	void MapDrawer::bresenhamDrawLine_x(Color color, int x1, int y1, int x2, int y2);
	void MapDrawer::bresenhamDrawLine_y(Color color, int x1, int y1, int x2, int y2);
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	wchar_t* CharToWchar(const char* c);
	
	void split(const std::string& src, const std::string& separator, std::vector<std::string>& dest);
};