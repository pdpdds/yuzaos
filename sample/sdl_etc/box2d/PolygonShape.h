#pragma once
#include <vector>

class Point {
public:
    Point() {};
    Point(int x, int y) { this->x = x; this->y = y; };
    int x, y;
};

class PolygonShape {
public:
    PolygonShape(std::vector<Point> vertices);
    ~PolygonShape();
    Point GetCenter(void);
    Point* GetVertices(void);
    int GetNumberOfVertices(void);
private:
    Point* vertices;
    Point center;
    int length;
};