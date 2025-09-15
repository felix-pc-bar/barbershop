#pragma once
#include "engTools.h"

class Point2d
{
public:
	Point2d();
	Point2d(const Vertex3d& from3d);
	Point2d(int xin, int yin);
	friend Point2d operator+(Point2d& p1, Point2d& p2);
	int x, y;
};

class Rect2d
{
public:
	//int minX, minY, maxX, maxY;
	Point2d min, max;
	Rect2d(int lX, int lY, int hX, int hY);
	Rect2d(Point2d v1, Point2d v2, Point2d v3);
};

Point2d operator+(const Point2d& p1, const Point2d& p2);
Point2d operator-(const Point2d& p1, const Point2d& p2);
Point2d operator*(const Point2d& p1, const Point2d& p2);
bool operator==(const Point2d& p1, const Point2d& p2);

int edgeSideSS(const Point2d& origin, const Point2d& reference, const Point2d& test);

bool isTriangleOnScreen(const Point2d& p1, const Point2d& p2, const Point2d& p3, int screenWidth, int screenHeight);


