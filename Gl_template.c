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
#include "resource.h"

#define glRGB(x, y, z)	glColor3ub((GLubyte)x, (GLubyte)y, (GLubyte)z)
#define BITMAP_ID 0x4D42
#define GL_PI 3.14f

HPALETTE hPalette = NULL;

static float tractorX = 10.0f;      // Pocz¹tkowa pozycja traktora X
static float tractorZ = 25.0f;      // Pocz¹tkowa pozycja traktora Z
static float tractorAngle = 0.0f;   // K¹t obrotu traktora (w stopniach)
static float tractorSpeed = 0.5f;   // Prêdkoœæ jazdy przód/ty³
static float tractorRotSpeed = 2.0f; // Prêdkoœæ obrotu w lewo/prawo


static LPCTSTR lpszAppName = TEXT("GL Template");
static HINSTANCE hInstance;

static GLfloat eyeX = 0.0f, eyeY = 5.0f, eyeZ = 30.0f;
static GLfloat centerX = 0.0f, centerY = 0.0f, centerZ = 0.0f;
static GLfloat upX = 0.0f, upY = 1.0f, upZ = 0.0f;
static float speed = 0.5f;

static float yaw = -90.0f;
static float pitch = 0.0f;
static float sensitivity = 2.0f;

static GLsizei lastHeight;
static GLsizei lastWidth;

BITMAPINFOHEADER	bitmapInfoHeader;
unsigned char* bitmapData;
unsigned int		texture[2];

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void SetDCPixelFormat(HDC hDC);

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
	const int x = 0;
	const int y = 1;
	const int z = 2;

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
	float frontX = cosf(yaw * (float)M_PI / 180.0f) * cosf(pitch * (float)M_PI / 180.0f);
	float frontY = sinf(pitch * (float)M_PI / 180.0f);
	float frontZ = sinf(yaw * (float)M_PI / 180.0f) * cosf(pitch * (float)M_PI / 180.0f);

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
	gluPerspective(60.0f, fAspect, 1.0f, 400.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void SetupRC()
{
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
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

	for (imageIdx = 0; imageIdx < (int)bitmapInfoHeader->biSizeImage; imageIdx += 3)
	{
		tempRGB = bitmapImage[imageIdx];
		bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
		bitmapImage[imageIdx + 2] = tempRGB;
	}

	fclose(filePtr);
	return bitmapImage;
}

// Funkcja rysuj¹ca p³aski teren (du¿y kwadrat)
void drawFlatTerrain() {
	glColor3f(0.2f, 0.8f, 0.2f);
	glBegin(GL_QUADS);
	glVertex3f(-100.0f, 0.0f, -100.0f);
	glVertex3f(100.0f, 0.0f, -100.0f);
	glVertex3f(100.0f, 0.0f, 100.0f);
	glVertex3f(-100.0f, 0.0f, 100.0f);
	glEnd();
}

// Funkcja rysuj¹ca budynek
void drawBuilding(float posX, float posZ) {
	float h = 0.0f; // teren jest p³aski
	glPushMatrix();
	glTranslatef(posX, h, posZ);

	float size = 5.0f;
	glColor3f(0.7f, 0.7f, 0.7f);
	glBegin(GL_QUADS);
	// Góra
	glVertex3f(-size, size, -size);
	glVertex3f(size, size, -size);
	glVertex3f(size, size, size);
	glVertex3f(-size, size, size);
	// Dó³
	glVertex3f(-size, 0.0f, -size);
	glVertex3f(size, 0.0f, -size);
	glVertex3f(size, 0.0f, size);
	glVertex3f(-size, 0.0f, size);
	// Przód
	glVertex3f(-size, 0.0f, size);
	glVertex3f(size, 0.0f, size);
	glVertex3f(size, size, size);
	glVertex3f(-size, size, size);
	// Ty³
	glVertex3f(-size, 0.0f, -size);
	glVertex3f(-size, size, -size);
	glVertex3f(size, size, -size);
	glVertex3f(size, 0.0f, -size);
	// Lewo
	glVertex3f(-size, 0.0f, -size);
	glVertex3f(-size, 0.0f, size);
	glVertex3f(-size, size, size);
	glVertex3f(-size, size, -size);
	// Prawo
	glVertex3f(size, 0.0f, -size);
	glVertex3f(size, 0.0f, size);
	glVertex3f(size, size, size);
	glVertex3f(size, size, -size);
	glEnd();

	glPopMatrix();
}

// Funkcja rysuj¹ca drzewo (pieñ pionowy)
void drawTree(float posX, float posZ) {
	float h = 0.0f; // p³aski teren
	glPushMatrix();
	glTranslatef(posX, h, posZ);

	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

	// Pieñ - walec
	glColor3f(0.5f, 0.3f, 0.1f);
	GLUquadric* quad = gluNewQuadric();
	gluQuadricNormals(quad, GLU_SMOOTH);
	gluCylinder(quad, 0.5f, 0.5f, 5.0f, 16, 16);

	// Korona - sto¿ek
	glTranslatef(0.0f, 0.0f, 5.0f);
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

	// Rysowanie p³askiego terenu
	drawFlatTerrain();

	// Rysowanie budynku
	drawBuilding(10.0f, 10.0f);

	// Rysowanie drzewa
	drawTree(-10.0f, 5.0f);

	glTranslatef(tractorX, 1.0f, tractorZ);

	// Obrót wokó³ osi Y (skala w stopniach)
	glRotatef(tractorAngle, 0.0f, 1.0f, 0.0f);
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
	pPal->palNumEntries = (WORD)nColors;

	RedRange = (BYTE)((1 << pfd.cRedBits) - 1);
	GreenRange = (BYTE)((1 << pfd.cGreenBits) - 1);
	BlueRange = (BYTE)((1 << pfd.cBlueBits) - 1);

	for (i = 0; i < nColors; i++)
	{
		pPal->palPalEntry[i].peRed = (BYTE)((i >> pfd.cRedShift) & RedRange);
		pPal->palPalEntry[i].peRed = (BYTE)((double)pPal->palPalEntry[i].peRed * 255.0 / RedRange);

		pPal->palPalEntry[i].peGreen = (BYTE)((i >> pfd.cGreenShift) & GreenRange);
		pPal->palPalEntry[i].peGreen = (BYTE)((double)pPal->palPalEntry[i].peGreen * 255.0 / GreenRange);

		pPal->palPalEntry[i].peBlue = (BYTE)((i >> pfd.cBlueShift) & BlueRange);
		pPal->palPalEntry[i].peBlue = (BYTE)((double)pPal->palPalEntry[i].peBlue * 255.0 / BlueRange);

		pPal->palPalEntry[i].peFlags = (BYTE)NULL;
	}

	hRetPal = CreatePalette(pPal);
	SelectPalette(hDC, hRetPal, FALSE);
	RealizePalette(hDC);
	free(pPal);

	return hRetPal;
}


int APIENTRY WinMain(HINSTANCE hInst,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
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

	return (int)msg.wParam;
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
		ChangeSize((GLsizei)LOWORD(lParam), (GLsizei)HIWORD(lParam));
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

		float rightX = cosf((yaw - 90.0f) * (float)M_PI / 180.0f);
		float rightZ = sinf((yaw - 90.0f) * (float)M_PI / 180.0f);

		switch (wParam) {
			// Kamera
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
		case 'T': // Do przodu
		{
			float angleRad = (tractorAngle + 90.0f) * (float)M_PI / 180.0f;
			tractorX += sinf(angleRad) * tractorSpeed;
			tractorZ += cosf(angleRad) * tractorSpeed;
		}
		break;

		case 'G': // Do ty³u
		{
			float angleRad = (tractorAngle + 90.0f) * (float)M_PI / 180.0f;
			tractorX -= sinf(angleRad) * tractorSpeed;
			tractorZ -= cosf(angleRad) * tractorSpeed;
		}
		break;
		case 'F': // Obrót w lewo
			tractorAngle += tractorRotSpeed;
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case 'H': // Obrót w prawo
			tractorAngle -= tractorRotSpeed;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}

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
		break;

	default:
		return (DefWindowProc(hWnd, message, wParam, lParam));
	}

	return 0L;
}

BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_OPENGL_VENDOR, TEXT("Vendor: "));
		SetDlgItemText(hDlg, IDC_OPENGL_RENDERER, TEXT("Renderer: "));
		SetDlgItemText(hDlg, IDC_OPENGL_VERSION, TEXT("Version: "));
		SetDlgItemText(hDlg, IDC_OPENGL_EXTENSIONS, TEXT("Extensions: "));

		SetDlgItemText(hDlg, IDC_GLU_VERSION, TEXT("GLU Version: "));
		SetDlgItemText(hDlg, IDC_GLU_EXTENSIONS, TEXT("GLU Extensions: "));
		return TRUE;

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
