// Gl_template.c
#define  _CRT_SECURE_NO_WARNINGS
#define M_PI 3.141592653589793238462643

#ifdef _MSC_VER
#  pragma comment(lib, "opengl32.lib")  
#  pragma comment(lib, "glu32.lib")     
#endif

#include <windows.h>            
#include <gl\gl.h>              
#include <gl\glu.h>             
#include <math.h>				
#include <stdio.h>
#include "resource.h"           

#define glRGB(x, y, z)	glColor3ub((GLubyte)x, (GLubyte)y, (GLubyte)z)
#define BITMAP_ID 0x4D42		
#define GL_PI 3.14

HPALETTE hPalette = NULL;

static LPCTSTR lpszAppName = "GL Template";
static HINSTANCE hInstance;

static GLfloat eyeX = 0.0f, eyeY = 5.0f, eyeZ = 30.0f;
static GLfloat centerX = 0.0f, centerY = 0.0f, centerZ = 0.0f;
static GLfloat upX = 0.0f, upY = 1.0f, upZ = 0.0f;
static float speed = 0.5f;

static float tractorAcceleration = 0.0f;
static float maxSpeed = 1.0f;   // maksymalna prêdkoœæ w przód
static float friction = 0.01f;  // tarcie
static float turnSpeed = 2.0f;


static float yaw = -90.0f;
static float pitch = 0.0f;
static float sensitivity = 2.0f;

static GLsizei lastHeight;
static GLsizei lastWidth;

BITMAPINFOHEADER	bitmapInfoHeader;
unsigned char* bitmapData;
unsigned int		texture[2];

// Zmienne dla traktora
static float tractorX = 0.0f;
static float tractorZ = 0.0f;
static float tractorY = 0.0f;
static float tractorYaw = 0.0f;
static float tractorPitch = 0.0f;
static float tractorRoll = 0.0f;
static float tractorSpeedForward = 0.0f;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL APIENTRY AboutDlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
void SetDCPixelFormat(HDC hDC);

// Funkcje pomocnicze
void ReduceToUnit(float vector[3])
{
	float length;
	length = (float)sqrt((vector[0] * vector[0]) +
		(vector[1] * vector[1]) +
		(vector[2] * vector[2]));
	if (length == 0.0f)
		length = 1.0f;
	vector[0] /= length;
	vector[1] /= length;
	vector[2] /= length;
}

void calcNormal(float v[3][3], float out[3])
{
	float v1[3], v2[3];
	static const int x = 0;
	static const int y = 1;
	static const int z = 2;

	v1[x] = v[0][x] - v[1][x];
	v1[y] = v[0][y] - v[1][y];
	v1[z] = v[0][z] - v[1][z];

	v2[x] = v[1][x] - v[2][x];
	v2[y] = v[1][y] - v[2][y];
	v2[z] = v[1][z] - v[2][z];

	out[x] = v1[y] * v2[z] - v1[z] * v2[y];
	out[y] = v1[z] * v2[x] - v1[x] * v2[z];
	out[z] = v1[x] * v2[y] - v1[y] * v2[x];

	ReduceToUnit(out);
}

void updateCameraDirection() {
	float frontX = cosf(yaw * M_PI / 180.0f) * cosf(pitch * M_PI / 180.0f);
	float frontY = sinf(pitch * M_PI / 180.0f);
	float frontZ = sinf(yaw * M_PI / 180.0f) * cosf(pitch * M_PI / 180.0f);

	centerX = eyeX + frontX;
	centerY = eyeY + frontY;
	centerZ = eyeZ + frontZ;
}

void ChangeSize(GLsizei w, GLsizei h)
{
	GLfloat fAspect;
	if (h == 0)
		h = 1;

	lastWidth = w;
	lastHeight = h;

	fAspect = (GLfloat)w / (GLfloat)h;

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, fAspect, 1.0, 400.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void SetupRC()
{
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glColor3f(0.0, 0.0, 0.0);
}

unsigned char* LoadBitmapFile(char* filename, BITMAPINFOHEADER* bitmapInfoHeader)
{
	FILE* filePtr;
	BITMAPFILEHEADER	bitmapFileHeader;
	unsigned char* bitmapImage;
	int					imageIdx = 0;
	unsigned char		tempRGB;

	filePtr = fopen(filename, "rb");
	if (filePtr == NULL)
		return NULL;

	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

	if (bitmapFileHeader.bfType != BITMAP_ID)
	{
		fclose(filePtr);
		return NULL;
	}

	fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);
	if (!bitmapImage)
	{
		free(bitmapImage);
		fclose(filePtr);
		return NULL;
	}

	fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);
	if (bitmapImage == NULL)
	{
		fclose(filePtr);
		return NULL;
	}

	for (imageIdx = 0; imageIdx < bitmapInfoHeader->biSizeImage; imageIdx += 3)
	{
		tempRGB = bitmapImage[imageIdx];
		bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
		bitmapImage[imageIdx + 2] = tempRGB;
	}

	fclose(filePtr);
	return bitmapImage;
}

// Delikatnie faluj¹cy teren
float getTerrainHeight(float x, float z) {
	// Delikatne falowanie: ma³a amplituda 0.5, czêstotliwoœæ mniejsza 0.05
	return -5.0f + sinf(x * 0.05f) * cosf(z * 0.05f) * 0.5f;
}

// Deklaracje funkcji rysuj¹cych
void prostopadloscian(double a, double b, double offsetX, double offsetY, double offsetZ);
void prostopadloscian1(double a, double b, double offsetX, double offsetY, double offsetZ);
void felga(double r, double h, double offsetX, double offsetY, double offsetZ);
void walec(double r, double h, double offsetX, double offsetY, double offsetZ);

void prostopadloscian(double a, double b, double offsetX, double offsetY, double offsetZ) {
	// Kod jak poprzednio, bez zmian
	for (float z = 0; z <= 20; z += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		for (float x = -10; x <= 20 + 75; x += 0.1) {
			glColor3f(0.8f, 0.0f, 1.0f);
			glVertex3d(x + offsetX, 0 + offsetY, z + offsetZ);
			glVertex3d(x + offsetX, 0 + offsetY, z + 0.1 + offsetZ);
		}
		glEnd();
	}

	// Reszta tak jak by³o...
	for (float z = 0; z <= 20; z += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		glColor3f(1.0f, 1.0f, 0.0f);
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
		glColor3f(1.0f, 0.0f, 0.0f);
		for (float x = -10; x <= 20 + 75; x += 0.1) {
			glVertex3d(x + offsetX, 20 + offsetY, z + offsetZ);
			glVertex3d(x + offsetX, 20 + offsetY, z + 0.1 + offsetZ);
		}
		glEnd();
	}

	for (float x = -10; x <= 20 + 75; x += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		glColor3f(0.0f, 1.0f, 0.0f);
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
	// Jak poprzednio
	for (float z = 0; z <= 20; z += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		for (float x = 0; x <= 20; x += 0.1) {
			glColor3f(0.8f, 0.0f, 1.0f);
			glVertex3d(x + offsetX, 0 + offsetY, z + offsetZ);
			glVertex3d(x + offsetX, 0 + offsetY, z + 0.1 + offsetZ);
		}
		glEnd();
	}

	// Reszta bez zmian...
	for (float z = 0; z <= 20; z += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		glColor3f(1.0f, 1.0f, 0.0f);
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
		glColor3f(1.0f, 0.0f, 0.0f);
		for (float x = 0; x <= 20; x += 0.1) {
			glVertex3d(x + offsetX, 20 + offsetY, z + offsetZ);
			glVertex3d(x + offsetX, 20 + offsetY, z + 0.1 + offsetZ);
		}
		glEnd();
	}

	for (float x = 0; x <= 20; x += 0.1) {
		glBegin(GL_TRIANGLE_STRIP);
		glColor3f(0.0f, 1.0f, 0.0f);
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

	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0.8, 1.0, 0.0);
	glVertex3d(offsetX, offsetY, offsetZ);
	for (alpha = 0; alpha <= 2 * PI; alpha += PI / 128.0) {
		x = r * sin(alpha) + offsetX;
		y = r * cos(alpha) + offsetY;
		glVertex3d(x, y, offsetZ);
	}
	glEnd();

	glBegin(GL_QUAD_STRIP);
	for (alpha = 0.0; alpha <= 2 * PI; alpha += PI / 128.0) {
		x = r * sin(alpha) + offsetX;
		y = r * cos(alpha) + offsetY;
		glVertex3d(x, y, offsetZ);
		glVertex3d(x, y, offsetZ + h);
	}
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0.8, 1.0, 0.0);
	glVertex3d(offsetX, offsetY, offsetZ + h);
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

	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0.8, 0.0, 0.0);
	glVertex3d(offsetX, offsetY, offsetZ);
	for (alpha = 0; alpha <= 2 * PI; alpha += PI / 128.0) {
		x = r * sin(alpha) + offsetX;
		y = r * cos(alpha) + offsetY;
		glVertex3d(x, y, offsetZ);
	}
	glEnd();

	glBegin(GL_QUAD_STRIP);
	for (alpha = 0.0; alpha <= 2 * PI; alpha += PI / 128.0) {
		x = r * sin(alpha) + offsetX;
		y = r * cos(alpha) + offsetY;
		glVertex3d(x, y, offsetZ);
		glVertex3d(x, y, offsetZ + h);
	}
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0.8, 0.0, 0.0);
	glVertex3d(offsetX, offsetY, offsetZ + h);
	for (alpha = 0; alpha <= 2 * PI; alpha += PI / 128.0) {
		x = r * sin(alpha) + offsetX;
		y = r * cos(alpha) + offsetY;
		glVertex3d(x, y, offsetZ + h);
	}
	glEnd();
}

void drawTerrain(int size, float scale) {
	const float halfSize = size / 2.0f;

	// Wype³nienie terenu
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.2f, 0.8f, 0.2f);
	for (int z = 0; z < size; ++z) {
		glBegin(GL_TRIANGLE_STRIP);
		for (int x = 0; x <= size; ++x) {
			float worldX = (x - halfSize) * scale;
			float worldZ = (z - halfSize) * scale;
			float worldZNext = ((z + 1) - halfSize) * scale;

			float height1 = getTerrainHeight(worldX, worldZ);
			float height2 = getTerrainHeight(worldX, worldZNext);

			glVertex3f(worldX, height1, worldZ);
			glVertex3f(worldX, height2, worldZNext);
		}
		glEnd();
	}

	// Siatka terenu
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(0.0f, 0.0f, 0.0f);
	for (int z = 0; z < size; ++z) {
		glBegin(GL_TRIANGLE_STRIP);
		for (int x = 0; x <= size; ++x) {
			float worldX = (x - halfSize) * scale;
			float worldZ = (z - halfSize) * scale;
			float worldZNext = ((z + 1) - halfSize) * scale;

			float height1 = getTerrainHeight(worldX, worldZ);
			float height2 = getTerrainHeight(worldX, worldZNext);

			glVertex3f(worldX, height1, worldZ);
			glVertex3f(worldX, height2, worldZNext);
		}
		glEnd();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void drawTractor() {
	glPushMatrix();

	glTranslatef(tractorX, tractorY, tractorZ);
	glRotatef(tractorYaw, 0.0f, 1.0f, 0.0f);
	glRotatef(tractorRoll * 180.0f / M_PI, 0.0f, 0.0f, 1.0f);
	glRotatef(tractorPitch * 180.0f / M_PI, 1.0f, 0.0f, 0.0f);

	glScalef(0.1f, 0.1f, 0.1f);

	prostopadloscian1(10, 10, 0, 20, 20);
	prostopadloscian(10, 10, 0, 0, 20);

	walec(10, 10, 0, 0, 0);
	walec(10, 10, 0, 0, 50);
	walec(2.5, 40, 0, 0, 10);
	walec(2.5, 40, 90, 0, 10);
	walec(8, 10, 90, 0, 0);
	walec(8, 10, 90, 0, 50);

	felga(8, 2, 0, 0, -2);
	felga(8, 2, 0, 0, 60);
	felga(6, 2, 90, 0, -2);
	felga(6, 2, 90, 0, 60);

	glPopMatrix();
}

void updateTractor() {
	// Zastosuj przyspieszenie do prêdkoœci
	tractorSpeedForward += tractorAcceleration;

	// Ogranicz prêdkoœæ maksymaln¹
	if (tractorSpeedForward > maxSpeed) tractorSpeedForward = maxSpeed;
	if (tractorSpeedForward < -0.3f) tractorSpeedForward = -0.3f; // cofanie wolniejsze

	// Tarcie: gdy nie wciskasz I/K w kolejnej klatce i acceleration = 0
	// prêdkoœæ powoli spada do zera
	if (fabs(tractorAcceleration) < 0.0001f) {
		if (tractorSpeedForward > 0.0f) {
			tractorSpeedForward -= friction;
			if (tractorSpeedForward < 0.0f) tractorSpeedForward = 0.0f;
		}
		else if (tractorSpeedForward < 0.0f) {
			tractorSpeedForward += friction;
			if (tractorSpeedForward > 0.0f) tractorSpeedForward = 0.0f;
		}
	}

	// Wyzeruj acceleration po ka¿dej klatce, ¿eby wymusiæ ponowne trzymanie klawisza
	// do przyspieszenia lub hamowania.
	tractorAcceleration = 0.0f;

	// Reszta kodu jak wczeœniej:
	float dirX = cosf(tractorYaw * M_PI / 180.0f);
	float dirZ = sinf(tractorYaw * M_PI / 180.0f);

	tractorX += dirX * tractorSpeedForward;
	tractorZ += dirZ * tractorSpeedForward;

	float baseHeight = 0.5f;
	float terrainHeight = getTerrainHeight(tractorX, tractorZ);
	tractorY = terrainHeight + baseHeight;

	float frontHeight = getTerrainHeight(tractorX + dirX, tractorZ + dirZ);
	float backHeight = getTerrainHeight(tractorX - dirX, tractorZ - dirZ);
	float dzForward = frontHeight - backHeight;
	tractorPitch = atan2f(dzForward, 2.0f);

	float rightX = cosf((tractorYaw + 90.0f) * M_PI / 180.0f);
	float rightZ = sinf((tractorYaw + 90.0f) * M_PI / 180.0f);

	float rightHeight = getTerrainHeight(tractorX + rightX, tractorZ + rightZ);
	float leftHeight = getTerrainHeight(tractorX - rightX, tractorZ - rightZ);

	float dzRight = rightHeight - leftHeight;
	tractorRoll = atan2f(dzRight, 2.0f);
}

// Funkcja rysuj¹ca prosty budynek (kostka)
void drawBuilding(float posX, float posZ) {
	float h = getTerrainHeight(posX, posZ);
	glPushMatrix();
	glTranslatef(posX, h, posZ);
	// Budynek: prosta kostka
	float size = 5.0f;
	glColor3f(0.7f, 0.7f, 0.7f);
	glBegin(GL_QUADS);
	// Góra
	glVertex3f(-size, size, -size);
	glVertex3f(size, size, -size);
	glVertex3f(size, size, size);
	glVertex3f(-size, size, size);
	// Dó³
	glVertex3f(-size, 0, -size);
	glVertex3f(size, 0, -size);
	glVertex3f(size, 0, size);
	glVertex3f(-size, 0, size);
	// Przód
	glVertex3f(-size, 0, size);
	glVertex3f(size, 0, size);
	glVertex3f(size, size, size);
	glVertex3f(-size, size, size);
	// Ty³
	glVertex3f(-size, 0, -size);
	glVertex3f(-size, size, -size);
	glVertex3f(size, size, -size);
	glVertex3f(size, 0, -size);
	// Lewo
	glVertex3f(-size, 0, -size);
	glVertex3f(-size, 0, size);
	glVertex3f(-size, size, size);
	glVertex3f(-size, size, -size);
	// Prawo
	glVertex3f(size, 0, -size);
	glVertex3f(size, 0, size);
	glVertex3f(size, size, size);
	glVertex3f(size, size, -size);
	glEnd();

	glPopMatrix();
}

// Funkcja rysuj¹ca drzewo: pieñ (walec) i korona (sto¿ek)
void drawTree(float posX, float posZ) {
	float h = getTerrainHeight(posX, posZ);
	glPushMatrix();
	glTranslatef(posX, h, posZ);

	// Rotacja tak, aby walec by³ pionowy wzd³u¿ osi Y:
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

	// Pieñ - walec
	glColor3f(0.5f, 0.3f, 0.1f);
	GLUquadric* quad = gluNewQuadric();
	gluQuadricNormals(quad, GLU_SMOOTH);
	gluCylinder(quad, 0.5f, 0.5f, 5.0f, 16, 16);

	// Korona - sto¿ek
	glTranslatef(0.0f, 0.0f, 5.0f); // przesuwamy wzd³u¿ nowej osi Z (dawnej Y)
	glColor3f(0.0f, 0.8f, 0.0f);
	gluCylinder(quad, 3.0f, 0.0f, 4.0f, 16, 16);

	gluDeleteQuadric(quad);
	glPopMatrix();
}

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);

	updateTractor();

	drawTerrain(300, 0.5f);

	// Rysowanie budynku w punkcie (10, 10)
	drawBuilding(10.0f, 10.0f);

	// Rysowanie drzewa w punkcie (-10, 5)
	drawTree(-10.0f, 5.0f);
	glTranslatef(10.0, 1.0, -10.0);
	drawTractor();

	glMatrixMode(GL_MODELVIEW);
	glFlush();
}


void SetDCPixelFormat(HDC hDC)
{
	int nPixelFormat;
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0,0,0,0,0,0,
		0,0,
		0,0,0,0,0,
		32,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0,0,0 };

	nPixelFormat = ChoosePixelFormat(hDC, &pfd);
	SetPixelFormat(hDC, nPixelFormat, &pfd);
}


HPALETTE GetOpenGLPalette(HDC hDC)
{
	HPALETTE hRetPal = NULL;
	PIXELFORMATDESCRIPTOR pfd;
	LOGPALETTE* pPal;
	int nPixelFormat;
	int nColors;
	int i;
	BYTE RedRange, GreenRange, BlueRange;

	nPixelFormat = GetPixelFormat(hDC);
	DescribePixelFormat(hDC, nPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	if (!(pfd.dwFlags & PFD_NEED_PALETTE))
		return NULL;

	nColors = 1 << pfd.cColorBits;

	pPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + nColors * sizeof(PALETTEENTRY));
	pPal->palVersion = 0x300;
	pPal->palNumEntries = nColors;

	RedRange = (1 << pfd.cRedBits) - 1;
	GreenRange = (1 << pfd.cGreenBits) - 1;
	BlueRange = (1 << pfd.cBlueBits) - 1;

	for (i = 0; i < nColors; i++)
	{
		pPal->palPalEntry[i].peRed = (i >> pfd.cRedShift) & RedRange;
		pPal->palPalEntry[i].peRed = (unsigned char)((double)pPal->palPalEntry[i].peRed * 255.0 / RedRange);

		pPal->palPalEntry[i].peGreen = (i >> pfd.cGreenShift) & GreenRange;
		pPal->palPalEntry[i].peGreen = (unsigned char)((double)pPal->palPalEntry[i].peGreen * 255.0 / GreenRange);

		pPal->palPalEntry[i].peBlue = (i >> pfd.cBlueShift) & BlueRange;
		pPal->palPalEntry[i].peBlue = (unsigned char)((double)pPal->palPalEntry[i].peBlue * 255.0 / BlueRange);

		pPal->palPalEntry[i].peFlags = (unsigned char)NULL;
	}

	hRetPal = CreatePalette(pPal);
	SelectPalette(hDC, hRetPal, FALSE);
	RealizePalette(hDC);
	free(pPal);

	return hRetPal;
}


int APIENTRY WinMain(HINSTANCE       hInst,
	HINSTANCE       hPrevInstance,
	LPSTR           lpCmdLine,
	int             nCmdShow)
{
	MSG     msg;
	WNDCLASS wc;
	HWND    hWnd;

	hInstance = hInst;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
	wc.lpszClassName = lpszAppName;

	if (RegisterClass(&wc) == 0)
		return FALSE;

	hWnd = CreateWindow(
		lpszAppName,
		lpszAppName,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		50, 50,
		400, 400,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (hWnd == NULL)
		return FALSE;

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HGLRC hRC;
	static HDC hDC;

	switch (message)
	{
	case WM_CREATE:
		hDC = GetDC(hWnd);
		SetDCPixelFormat(hDC);
		hPalette = GetOpenGLPalette(hDC);
		hRC = wglCreateContext(hDC);
		wglMakeCurrent(hDC, hRC);
		SetupRC();
		glGenTextures(2, &texture[0]);

		bitmapData = LoadBitmapFile("Bitmapy\\checker.bmp", &bitmapInfoHeader);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmapInfoHeader.biWidth,
			bitmapInfoHeader.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmapData);
		if (bitmapData)
			free(bitmapData);

		bitmapData = LoadBitmapFile("Bitmapy\\crate.bmp", &bitmapInfoHeader);
		glBindTexture(GL_TEXTURE_2D, texture[1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmapInfoHeader.biWidth,
			bitmapInfoHeader.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmapData);
		if (bitmapData)
			free(bitmapData);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		break;

	case WM_DESTROY:
		wglMakeCurrent(hDC, NULL);
		wglDeleteContext(hRC);
		if (hPalette != NULL)
			DeleteObject(hPalette);
		PostQuitMessage(0);
		break;

	case WM_SIZE:
		ChangeSize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_PAINT:
	{
		RenderScene();
		SwapBuffers(hDC);
		ValidateRect(hWnd, NULL);
	}
	break;

	case WM_QUERYNEWPALETTE:
		if (hPalette)
		{
			int nRet;
			SelectPalette(hDC, hPalette, FALSE);
			nRet = RealizePalette(hDC);
			InvalidateRect(hWnd, NULL, FALSE);
			return nRet;
		}
		break;

	case WM_KEYDOWN:
	{
		float frontX = centerX - eyeX;
		float frontY = centerY - eyeY;
		float frontZ = centerZ - eyeZ;
		float length = sqrtf(frontX * frontX + frontY * frontY + frontZ * frontZ);
		if (length != 0.0f) {
			frontX /= length;
			frontY /= length;
			frontZ /= length;
		}

		float rightX = cosf((yaw - 90.0f) * M_PI / 180.0f);
		float rightY = 0.0f;
		float rightZ = sinf((yaw - 90.0f) * M_PI / 180.0f);

		switch (wParam) {
		case 'W':
			eyeX += frontX * speed;
			eyeY += frontY * speed;
			eyeZ += frontZ * speed;
			break;
		case 'S':
			eyeX -= frontX * speed;
			eyeY -= frontY * speed;
			eyeZ -= frontZ * speed;
			break;
		case 'A':
			eyeX -= rightX * speed;
			eyeZ -= rightZ * speed;
			break;
		case 'D':
			eyeX += rightX * speed;
			eyeZ += rightZ * speed;
			break;
		case 'Q':
			eyeY += speed;
			break;
		case 'E':
			eyeY -= speed;
			break;
		case VK_LEFT:
			yaw -= sensitivity;
			break;
		case VK_RIGHT:
			yaw += sensitivity;
			break;
		case VK_UP:
			pitch += sensitivity;
			if (pitch > 89.0f) pitch = 89.0f;
			break;
		case VK_DOWN:
			pitch -= sensitivity;
			if (pitch < -89.0f) pitch = -89.0f;
			break;

			// Sterowanie traktorem
		case 'I':
			// przyspieszenie w przód
			tractorAcceleration += 0.05f;
			if (tractorAcceleration > 0.2f) tractorAcceleration = 0.2f; // ogranicz przyspieszenie
			break;
		case 'K':
			// przyspieszenie w ty³ (lub hamowanie)
			tractorAcceleration -= 0.05f;
			if (tractorAcceleration < -0.1f) tractorAcceleration = -0.1f; // ogranicz prêdkoœæ do ty³u
			break;
		case 'J':
			// skrêæ w lewo
			// skrêcanie zale¿ne od prêdkoœci: im szybciej jedziesz, tym mniejszy skuteczny skrêt
			// im wolniejsza jazda, tym ³atwiej skrêciæ
		{
			float speedFactor = 1.0f - (fabs(tractorSpeedForward) / maxSpeed);
			if (speedFactor < 0.3f) speedFactor = 0.3f; // minimalny skrêt
			tractorYaw -= turnSpeed * speedFactor;
		}
		break;
		case 'L':
			// skrêæ w prawo
		{
			float speedFactor = 1.0f - (fabs(tractorSpeedForward) / maxSpeed);
			if (speedFactor < 0.3f) speedFactor = 0.3f;
			tractorYaw += turnSpeed * speedFactor;
		}
		break;

		updateCameraDirection();
		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;

	case WM_PALETTECHANGED:
		if ((hPalette != NULL) && ((HWND)wParam != hWnd))
		{
			SelectPalette(hDC, hPalette, FALSE);
			RealizePalette(hDC);
			UpdateColors(hDC);
			return 0;
		}
		break;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_FILE_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_HELP_ABOUT:
			DialogBox(hInstance,
				MAKEINTRESOURCE(IDD_DIALOG_ABOUT),
				hWnd,
				AboutDlgProc);
			break;
		}
	}
	break;

	default:
		return (DefWindowProc(hWnd, message, wParam, lParam));
	}

	return (0L);
}

BOOL APIENTRY AboutDlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		int i;
		GLenum glError;
		SetDlgItemText(hDlg, IDC_OPENGL_VENDOR, (LPCSTR)glGetString(GL_VENDOR));
		SetDlgItemText(hDlg, IDC_OPENGL_RENDERER, (LPCSTR)glGetString(GL_RENDERER));
		SetDlgItemText(hDlg, IDC_OPENGL_VERSION, (LPCSTR)glGetString(GL_VERSION));
		SetDlgItemText(hDlg, IDC_OPENGL_EXTENSIONS, (LPCSTR)glGetString(GL_EXTENSIONS));
		SetDlgItemText(hDlg, IDC_GLU_VERSION, (LPCSTR)gluGetString(GLU_VERSION));
		SetDlgItemText(hDlg, IDC_GLU_EXTENSIONS, (LPCSTR)gluGetString(GLU_EXTENSIONS));

		i = 0;
		do {
			glError = glGetError();
			SetDlgItemText(hDlg, IDC_ERROR1 + i, (LPCSTR)gluErrorString(glError));
			i++;
		} while (i < 6 && glError != GL_NO_ERROR);

		return (TRUE);
	}
	break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
			EndDialog(hDlg, TRUE);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, TRUE);
		break;
	}
	return FALSE;
}
