#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <windows.h>

#define PI 3.141592653589793

typedef struct Point Point;
struct Point {
	double x;
	double y;
	double z;
};

typedef struct Projection Projection;
struct Projection {
	int i;
	int j;
	double luminance;
};

typedef struct Screen Screen;
struct Screen {
	int distance;
	int height;
	int width;

	Point lightsource;

	char** c;
	double** luminance;
};

typedef struct Axis Axis;
struct Axis {
	Point origin;
	Point direction;
};

typedef struct Surface Surface;
struct Surface {
	int nu;
	int nv;

	Point** points;
	Point** normals;
};