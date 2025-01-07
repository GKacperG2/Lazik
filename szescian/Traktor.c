// Gl_template.c 
#define  _CRT_SECURE_NO_WARNINGS
#define M_PI 3.141592653589793238462643

#ifdef _MSC_VER
#  pragma comment(lib, "opengl32.lib")
#  pragma comment(lib, "glu32.lib")
#endif

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <math.h>
#include <stdio.h>


#define glRGB(x, y, z)	glColor3ub((GLubyte)x, (GLubyte)y, (GLubyte)z)
#define BITMAP_ID 0x4D42
#define GL_PI 3.14f


void prostopadloscian(double a, double b, double offsetX, double offsetY, double offsetZ) {
	for (float z = 0; z <= 20; z += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		for (float x = -10; x <= 20 + 75; x += 0.1) {
			glColor3f(0.8, 0.0, 1);
			glVertex3d(x + offsetX, 0 + offsetY, z + offsetZ);
			glVertex3d(x + offsetX, 0 + offsetY, z + 0.1 + offsetZ);
		}
		glEnd();
	}

	for (float z = 0; z <= 20; z += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		glColor3f(1.0, 1.0, 0.0);
		for (float x = -10; x <= 20 + 75; x += 0.1) {
			glVertex3d(x + offsetX, z + offsetY, 0 + offsetZ);
			glVertex3d(x + offsetX, z + 0.1 + offsetY, 0 + offsetZ);
		}
		glEnd();
	}

	for (float z = 0; z <= 20; z += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		glColor3d(0.8, 0.0, 0);
		for (float x = 0; x <= 20; x += 0.1) {
			glVertex3d(-10 + offsetX, z + offsetY, x + offsetZ);
			glVertex3d(-10 + offsetX, z + 0.1 + offsetY, x + offsetZ);
		}
		glEnd();
	}

	for (float z = 0; z <= 20; z += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		glColor3f(1.0, 0.0, 0.0);
		for (float x = -10; x <= 20 + 75; x += 0.1) {
			glVertex3d(x + offsetX, 20 + offsetY, z + offsetZ);
			glVertex3d(x + offsetX, 20 + offsetY, z + 0.1 + offsetZ);
		}
		glEnd();
	}

	for (float x = -10; x <= 20 + 75; x += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		glColor3f(0.0, 1.0, 0.0);
		for (float y = 0; y <= 20; y += 0.1) {
			glVertex3d(x + offsetX, y + offsetY, 20 + offsetZ);
			glVertex3d(x + 0.1 + offsetX, y + offsetY, 20 + offsetZ);
		}
		glEnd();
	}

	for (float z = 0; z <= 20; z += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		glColor3d(0.8, 0.0, 0);
		for (float x = 0; x <= 20; x += 0.1) {
			glVertex3d(95 + offsetX, z + offsetY, x + offsetZ);
			glVertex3d(95 + offsetX, z + 0.1 + offsetY, x + offsetZ);
		}
		glEnd();
	}
}
void prostopadloscian1(double a, double b, double offsetX, double offsetY, double offsetZ) {
	for (float z = 0; z <= 20; z += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		for (float x = 0; x <= 20; x += 0.1) {
			glColor3f(0.8, 0.0, 1);
			glVertex3d(x + offsetX, 0 + offsetY, z + offsetZ);
			glVertex3d(x + offsetX, 0 + offsetY, z + 0.1 + offsetZ);
		}
		glEnd();
	}

	for (float z = 0; z <= 20; z += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		glColor3f(1.0, 1.0, 0.0);
		for (float x = 0; x <= 20; x += 0.1) {
			glVertex3d(x + offsetX, z + offsetY, 0 + offsetZ);
			glVertex3d(x + offsetX, z + 0.1 + offsetY, 0 + offsetZ);
		}
		glEnd();
	}

	for (float z = 0; z <= 20; z += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		glColor3d(0.8, 0.0, 0);
		for (float x = 0; x <= 20; x += 0.1) {
			glVertex3d(0 + offsetX, z + offsetY, x + offsetZ);
			glVertex3d(0 + offsetX, z + 0.1 + offsetY, x + offsetZ);
		}
		glEnd();
	}

	for (float z = 0; z <= 20; z += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		glColor3f(1.0, 0.0, 0.0);
		for (float x = 0; x <= 20; x += 0.1) {
			glVertex3d(x + offsetX, 20 + offsetY, z + offsetZ);
			glVertex3d(x + offsetX, 20 + offsetY, z + 0.1 + offsetZ);
		}
		glEnd();
	}

	for (float x = 0; x <= 20; x += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		glColor3f(0.0, 1.0, 0.0);
		for (float y = 0; y <= 20; y += 0.1) {
			glVertex3d(x + offsetX, y + offsetY, 20 + offsetZ);
			glVertex3d(x + 0.1 + offsetX, y + offsetY, 20 + offsetZ);
		}
		glEnd();
	}

	for (float z = 0; z <= 20; z += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		glColor3d(0.8, 0.0, 0);
		for (float x = 0; x <= 20; x += 0.1) {
			glVertex3d(20 + offsetX, z + offsetY, x + offsetZ);
			glVertex3d(20 + offsetX, z + 0.1 + offsetY, x + offsetZ);
		}
		glEnd();
	}
}
void felga(double r, double h, double offsetX, double offsetY, double offsetZ) {
	const double PI = 3.14;
	double x, y, alpha;

	// Podstawa dolna
	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0.8, 1.0, 0.0);
	glVertex3d(offsetX, offsetY, offsetZ); // Œrodek podstawy dolnej
	for (alpha = 0; alpha <= 2 * PI; alpha += PI / 128.0) {
		x = r * sin(alpha) + offsetX;
		y = r * cos(alpha) + offsetY;
		glVertex3d(x, y, offsetZ);
	}
	glEnd();

	// Œciany boczne
	glBegin(GL_QUAD_STRIP);
	for (alpha = 0.0; alpha <= 2 * PI; alpha += PI / 128.0) {
		x = r * sin(alpha) + offsetX;
		y = r * cos(alpha) + offsetY;
		glVertex3d(x, y, offsetZ);         // Punkt na dolnej podstawie
		glVertex3d(x, y, offsetZ + h);     // Punkt na górnej podstawie
	}
	glEnd();

	// Podstawa górna
	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0.8, 1.0, 0.0);
	glVertex3d(offsetX, offsetY, offsetZ + h); // Œrodek podstawy górnej
	for (alpha = 0; alpha <= 2 * PI; alpha += PI / 128.0) {
		x = r * sin(alpha) + offsetX;
		y = r * cos(alpha) + offsetY;
		glVertex3d(x, y, offsetZ + h);
	}
	glEnd();
}

void walec(double r, double h, double offsetX, double offsetY, double offsetZ) {
	const double PI = 3.14;
	double x, y, alpha;

	// Podstawa dolna
	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0.8, 0.0, 0.0);
	glVertex3d(offsetX, offsetY, offsetZ); // Œrodek podstawy dolnej
	for (alpha = 0; alpha <= 2 * PI; alpha += PI / 128.0) {
		x = r * sin(alpha) + offsetX;
		y = r * cos(alpha) + offsetY;
		glVertex3d(x, y, offsetZ);
	}
	glEnd();

	// Œciany boczne
	glBegin(GL_QUAD_STRIP);
	for (alpha = 0.0; alpha <= 2 * PI; alpha += PI / 128.0) {
		x = r * sin(alpha) + offsetX;
		y = r * cos(alpha) + offsetY;
		glVertex3d(x, y, offsetZ);         // Punkt na dolnej podstawie
		glVertex3d(x, y, offsetZ + h);     // Punkt na górnej podstawie
	}
	glEnd();

	// Podstawa górna
	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0.8, 0.0, 0.0);
	glVertex3d(offsetX, offsetY, offsetZ + h); // Œrodek podstawy górnej
	for (alpha = 0; alpha <= 2 * PI; alpha += PI / 128.0) {
		x = r * sin(alpha) + offsetX;
		y = r * cos(alpha) + offsetY;
		glVertex3d(x, y, offsetZ + h);
	}
	glEnd();
}

void drawTractor() {
	glPushMatrix();

	glScalef(0.1f, 0.1f, 0.1f);

	prostopadloscian1(10, 10, 0, 20, 20);
	prostopadloscian(10, 10, 0, 0, 20);

	walec(10, 10, 0, 0, 0);
	walec(10, 10, 0, 0, 50);
	walec(2.5f, 40.0f, 0, 0, 10);
	walec(2.5f, 40.0f, 90, 0, 10);
	walec(10, 10, 90, 0, 0);
	walec(10, 10, 90, 0, 50);

	felga(8, 2, 0, 0, -2);
	felga(8, 2, 0, 0, 60);
	felga(8, 2, 90, 0, -2);
	felga(8, 2, 90, 0, 60);

	glPopMatrix();
}