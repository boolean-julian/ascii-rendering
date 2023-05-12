#include "types.h"

void printPoint(Point pt);
Point newPoint(double x, double y, double z);
Point scalePoint(double lambda, Point pt);
Point addPoints(Point a, Point b);
Point subtractPoints(Point a, Point b);
double normPoint(Point pt);
Point normalizePoint(Point pt);
Axis newAxis(Point origin, Point direction);
Point rotatePoint(Point pt, Axis axis, double theta);