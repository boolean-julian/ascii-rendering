#include "prototypes.h"

Point newPoint(double x, double y, double z) {
	Point pt;
	pt.x = x;
	pt.y = y;
	pt.z = z;

	return pt;
}

Point scalePoint(double lambda, Point pt) {
	Point result;
	result.x = lambda*pt.x;
	result.y = lambda*pt.y;
	result.z = lambda*pt.z;

	return result;
}

Point addPoints(Point a, Point b) {
	return newPoint(a.x+b.x, a.y+b.y, a.z+b.z);
}

Point subtractPoints(Point a, Point b) {
	return addPoints(a, scalePoint(-1, b));
}

void printPoint(Point pt) {
	printf("(%f, %f, %f)\n", pt.x, pt.y, pt.z);
}

double normPoint(Point pt) {
	return sqrt(pt.x*pt.x + pt.y*pt.y + pt.z*pt.z);
}

double innerPoints(Point a, Point b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

Point normalizePoint(Point pt) {
	return scalePoint(1./normPoint(pt), pt);
}

Axis newAxis(Point origin, Point direction) {
	Axis ax;
	ax.origin = origin;
	ax.direction = normalizePoint(direction);

	return ax;
}

Point rotatePoint(Point pt, Axis axis, double theta) {
	if (theta == 0) return pt;

	pt = subtractPoints(pt, axis.origin);

	Point result;
	double cost = cos(theta);
	double sint = sin(theta);

	Point di = axis.direction;

	result.x    =   pt.x * (cost                    + di.x * di.x * (1-cost))
				+   pt.y * (di.x * di.y * (1-cost)  - di.z * sint)
				+   pt.z * (di.x * di.z * (1-cost)  + di.y * sint);

	result.y    =   pt.x * (di.y * di.x * (1-cost)  + di.z * sint)
				+   pt.y * (cost                    + di.y * di.y * (1-cost))
				+   pt.z * (di.y * di.z * (1-cost)  - di.x * sint);

	result.z    =   pt.x * (di.z * di.x * (1-cost)  - di.y * sint)
				+   pt.y * (di.z * di.y * (1-cost)  + di.x * sint)
				+   pt.z * (cost                    + di.z * di.z * (1-cost));
	
	return addPoints(result, axis.origin);
}

Point crossProduct(Point a, Point b) {
	Point c;

	c.x = a.y*b.z - a.z*b.y;
	c.y = a.z*b.x - a.x*b.z;
	c.z = a.x*b.y - a.y*b.x;

	return c;
}

void printProjection(Projection pr) {
	printf("[%d, %d],\n", pr.i, pr.j);
}

Projection projectPoint(Point pt, Point normal, Screen screen) {
	Projection pr;
	pr.luminance = innerPoints(screen.lightsource, normal);

	pr.i = round(screen.height/2  - screen.distance * pt.y/pt.z);
	pr.j = round(screen.width/2 + screen.distance * pt.x/pt.z);

	if (pr.i < 0 || pr.i >= screen.height || pr.j < 0 || pr.j >= screen.width) {
		pr.i = 0;
		pr.j = 0;
	}

	if (pr.luminance > screen.luminance[pr.i][pr.j]) {
		screen.luminance[pr.i][pr.j] = pr.luminance;

		int luminance_index = 11 * pr.luminance;
		screen.c[pr.i][pr.j] = ".,-~:;=!*#$@"[luminance_index];
	}

	return pr;
}

void clearScreen(Screen screen) {
	printf("\x1b[H");
	for (int k = 0; k < screen.height; k++) {
		for (int j = 0; j < screen.width; j++) {
			screen.c[k][j] = ' ';
			screen.luminance[k][j] = 0;
		}
	}
}

Screen newScreen(int width, int height, int distance, Point lightsource) {
	Screen screen;
	screen.width = width;
	screen.height = height;
	screen.distance = distance;

	screen.lightsource = normalizePoint(lightsource);

	screen.c = (char**) malloc(sizeof(char*)*screen.height);	
	screen.luminance = (double**) malloc(sizeof(double*)*screen.height);
	for (int k = 0; k < screen.height; k++) {
		screen.luminance[k] = (double*) malloc(sizeof(double)*screen.width);
		screen.c[k] = (char*) malloc(sizeof(char)*screen.width);
		for (int j = 0; j < screen.width; j++) {
			screen.luminance[k][j] = 0;
			screen.c[k][j] = ' ';
		}
	}

	const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
	write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);

	clearScreen(screen);

	return screen;
}

void printScreen(Screen scr) {
	for (int i = 0; i < scr.height; i++) {
		for (int j = 0; j < scr.width; j++) {
			putchar(scr.c[i][j]);
			putchar(' ');
		}
		putchar('\n');
	}
}

Surface newSurface(int nu, int nv) {
	Surface surface;

	surface.nu = nu;
	surface.nv = nv;

	surface.points = (Point**) malloc(sizeof(Point*) * nu);
	surface.normals = (Point**) malloc(sizeof(Point*) * nu);
	for (int k = 0; k < nu; k++) {
		surface.points[k] = (Point*) malloc(sizeof(Point) * nv);
		surface.normals[k] = (Point*) malloc(sizeof(Point) * nv);
	}

	return surface;
}

Surface flipSurfaceNormal(Surface surface) {
	Surface flipped = newSurface(surface.nu, surface.nv);

	for (int ku = 0; ku < surface.nu; ku++) {
		for (int kv = 0; kv < surface.nv; kv++) {
			flipped.points[ku][kv] = surface.points[ku][kv];
			flipped.normals[ku][kv] = scalePoint(-1, surface.normals[ku][kv]);
		}
	}

	return flipped;
}

Surface rotateSurface(Surface surface, Axis axis, double theta) {
	Surface rotated = newSurface(surface.nu, surface.nv);
	Axis normalAxis = newAxis(newPoint(0,0,0), axis.direction);

	for (int ku = 0; ku < surface.nu; ku++) {
		for (int kv = 0; kv < surface.nv; kv++) {
			rotated.points[ku][kv] = rotatePoint(surface.points[ku][kv], axis, theta);
			rotated.normals[ku][kv] = normalizePoint(rotatePoint(surface.normals[ku][kv], normalAxis, theta));
		}
	}

	return rotated;
}

void projectSurface(Surface surface, Screen screen) {
	for (int ku = 0; ku < surface.nu; ku++) {
		for (int kv = 0; kv < surface.nv; kv++) {
			projectPoint(surface.points[ku][kv], surface.normals[ku][kv], screen);
		}
	}
}

Surface getQuadrilateralSurface(Point* vertices, int nu, int nv) {
	Surface surface = newSurface(nu, nv);

	Point normal = crossProduct(
		subtractPoints(vertices[1], vertices[0]),
		subtractPoints(vertices[2], vertices[0])
	);

	for (int ku = 0; ku < surface.nu; ku++) {
		for (int kv = 0; kv < surface.nv; kv++) {
			double tu = (double)ku/surface.nu;
			double tv = (double)kv/surface.nv;

			Point p0 = addPoints(scalePoint(1-tu, vertices[0]), scalePoint(tu, vertices[1]));
			Point p1 = addPoints(scalePoint(1-tu, vertices[2]), scalePoint(tu, vertices[3]));

			surface.points[ku][kv] = addPoints(scalePoint(1-tv, p0), scalePoint(tv, p1));
			surface.normals[ku][kv] = normal;
		}
	}

	return surface;
}

Surface getTorusSurface(double r1, double r2, Point center, int nu, int nv) {
	Surface surface = newSurface(nu, nv);

	for (int ku = 0; ku < surface.nu; ku++) {
		for (int kv = 0; kv < surface.nv; kv++) {
			double tu = 2*PI*(double)ku/surface.nu;
			double tv = 2*PI*(double)kv/surface.nv;

			surface.points[ku][kv] = newPoint(
				(r2 + r1*cos(tu))*cos(tv),
				(r2 + r1*cos(tu))*sin(tv),
				r1*sin(tu)
			);

			surface.points[ku][kv] = addPoints(
				surface.points[ku][kv],
				center
			);

			surface.normals[ku][kv] = normalizePoint(newPoint(
				cos(tv)*cos(tu),
				sin(tv)*cos(tu),
				sin(tu)
			));
		}
	}

	return surface;
}

int main() {
	// Initialize screen
	int width = 400;
	int height = 160;
	int distance = 80;

	Point lightsource = newPoint(0, 0.5, -1);
	Screen screen = newScreen(width, height, distance, lightsource);

	// Rotation axis for animation
	Point axOrigin = newPoint(0,0,8);
	Point axDirect = newPoint(1.2,0.5,0.2);
	Axis axis = newAxis(axOrigin, axDirect);

	// Definition of cube faces via vertices
	double cubedx = 1;
	double cubedy = 1;
	double cubedz = 1;

	Point vertices0[4];
	vertices0[0] = newPoint(-cubedx, -cubedy, axOrigin.z - cubedz);
	vertices0[1] = newPoint(-cubedx, +cubedy, axOrigin.z - cubedz);
	vertices0[2] = newPoint(+cubedx, -cubedy, axOrigin.z - cubedz);
	vertices0[3] = newPoint(+cubedx, +cubedy, axOrigin.z - cubedz);
	
	Point vertices1[4];
	vertices1[0] = newPoint(-cubedx, +cubedy, axOrigin.z - cubedz);
	vertices1[1] = newPoint(-cubedx, +cubedy, axOrigin.z + cubedz);
	vertices1[2] = newPoint(+cubedx, +cubedy, axOrigin.z - cubedz);
	vertices1[3] = newPoint(+cubedx, +cubedy, axOrigin.z + cubedz);

	Point vertices2[4];
	vertices2[0] = newPoint(-cubedx, -cubedy, axOrigin.z + cubedz);
	vertices2[1] = newPoint(-cubedx, +cubedy, axOrigin.z + cubedz);
	vertices2[2] = newPoint(+cubedx, -cubedy, axOrigin.z + cubedz);
	vertices2[3] = newPoint(+cubedx, +cubedy, axOrigin.z + cubedz);

	Point vertices3[4];
	vertices3[0] = newPoint(-cubedx, -cubedy, axOrigin.z - cubedz);
	vertices3[1] = newPoint(-cubedx, -cubedy, axOrigin.z + cubedz);
	vertices3[2] = newPoint(+cubedx, -cubedy, axOrigin.z - cubedz);
	vertices3[3] = newPoint(+cubedx, -cubedy, axOrigin.z + cubedz);

	Point vertices4[4];
	vertices4[0] = newPoint(-cubedx, +cubedy, axOrigin.z - cubedz);
	vertices4[1] = newPoint(-cubedx, +cubedy, axOrigin.z + cubedz);
	vertices4[2] = newPoint(-cubedx, -cubedy, axOrigin.z - cubedz);
	vertices4[3] = newPoint(-cubedx, -cubedy, axOrigin.z + cubedz);

	Point vertices5[4];
	vertices5[0] = newPoint(+cubedx, +cubedy, axOrigin.z - cubedz);
	vertices5[1] = newPoint(+cubedx, +cubedy, axOrigin.z + cubedz);
	vertices5[2] = newPoint(+cubedx, -cubedy, axOrigin.z - cubedz);
	vertices5[3] = newPoint(+cubedx, -cubedy, axOrigin.z + cubedz);

	// Make surfaces from vertices
	Surface quad0 = getQuadrilateralSurface(vertices0, 50, 50);
	Surface quad1 = getQuadrilateralSurface(vertices1, 50, 50);
	Surface quad2 = getQuadrilateralSurface(vertices2, 50, 50);
	Surface quad3 = getQuadrilateralSurface(vertices3, 50, 50);
	Surface quad4 = getQuadrilateralSurface(vertices4, 50, 50);
	Surface quad5 = getQuadrilateralSurface(vertices5, 50, 50);

	// Offset rotation, looks really cool!
	double offsetAngle = PI/4;
	Axis offsetAxis = newAxis(
		axOrigin,
		newPoint(1,1,0)
	);
	quad0 = rotateSurface(quad0, offsetAxis, offsetAngle);
	quad1 = rotateSurface(quad1, offsetAxis, offsetAngle);
	quad2 = rotateSurface(quad2, offsetAxis, offsetAngle);
	quad3 = rotateSurface(quad3, offsetAxis, offsetAngle);
	quad4 = rotateSurface(quad4, offsetAxis, offsetAngle);
	quad5 = rotateSurface(quad5, offsetAxis, offsetAngle);

	// Surface normal flipping
	quad2 = flipSurfaceNormal(quad2);
	quad3 = flipSurfaceNormal(quad3);
	quad4 = flipSurfaceNormal(quad4);

	// We also prep a torus
	Surface torus = getTorusSurface(1, 4, axOrigin, 50, 100);

	int numberOfFrames = 400;
	for (int i = 0; i <= numberOfFrames; i++) {
		double phi = 4*PI*(double)i/numberOfFrames;
		
		Surface rotated0 = rotateSurface(quad0, axis, phi);
		Surface rotated1 = rotateSurface(quad1, axis, phi);
		Surface rotated2 = rotateSurface(quad2, axis, phi);
		Surface rotated3 = rotateSurface(quad3, axis, phi);
		Surface rotated4 = rotateSurface(quad4, axis, phi);
		Surface rotated5 = rotateSurface(quad5, axis, phi);

		projectSurface(rotated0, screen);
		projectSurface(rotated1, screen);
		projectSurface(rotated2, screen);
		projectSurface(rotated3, screen);
		projectSurface(rotated4, screen);
		projectSurface(rotated5, screen);
		
		Surface rotatedTorus = rotateSurface(torus, axis, phi);
		projectSurface(rotatedTorus, screen);

		printScreen(screen);
		Sleep(10);
		clearScreen(screen);
	}

	return 0;
}