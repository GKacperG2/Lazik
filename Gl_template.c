// gl_template.c
#define _CRT_SECURE_NO_WARNINGS
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

#define glRGB(x, y, z) glColor3ub((GLubyte)x, (GLubyte)y, (GLubyte)z)

HPALETTE hPalette = NULL;

float tractorX = 10.0f;
float tractorZ = 25.0f;
float tractorAngle = 0.0f;
float tractorSpeed = 0.5f;
float tractorRotSpeed = 2.0f;

static LPCTSTR lpszAppName = TEXT("GL Template");
static HINSTANCE hInstance;

static GLfloat eyeX = 0.0f, eyeY = 5.0f, eyeZ = 30.0f;
static GLfloat centerX = 0.0f, centerY = 0.0f, centerZ = 0.0f;
static GLfloat upX = 0.0f, upY = 1.0f, upZ = 0.0f;
static float speed = 0.5f;

static float yaw = -90.0f;
static float pitch = 0.0f;
static float sensitivity = 2.0f;

static float tractorX = 10.0f;      // Początkowa pozycja traktora X
static float tractorZ = 25.0f;      // Początkowa pozycja traktora Z
static float tractorAngle = 0.0f;   // Kąt obrotu traktora (w stopniach)
static float tractorSpeed = 0.5f;   // Prędkość jazdy przód/tył
static float tractorRotSpeed = 2.0f; // Prędkość obrotu w lewo/prawo

static GLsizei lastHeight;
static GLsizei lastWidth;

BITMAPINFOHEADER bitmapInfoHeader;
unsigned char* bitmapData = NULL;
unsigned int texture[2];

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void SetDCPixelFormat(HDC hDC);
HPALETTE GetOpenGLPalette(HDC hDC);
unsigned char* LoadBitmapFile(const char* filename, BITMAPINFOHEADER* bitmapInfoHeader);

void updateCameraDirection();
void ChangeSize(GLsizei w, GLsizei h);
void SetupRC();
void RenderScene();
void drawBuilding(float posX, float posZ);
void drawFlatTerrain();
void drawTree(float posX, float posZ);
void drawTractor();



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
        0, 0, 0, 0, 0, 0,
        0, 0,
        0, 0, 0, 0, 0,
        32,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    nPixelFormat = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, nPixelFormat, &pfd);
}

HPALETTE GetOpenGLPalette(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE* pPal;
    int nPixelFormat = GetPixelFormat(hDC);
    DescribePixelFormat(hDC, nPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    if (!(pfd.dwFlags & PFD_NEED_PALETTE))
        return NULL;

    int nColors = 1 << pfd.cColorBits;
    pPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + nColors * sizeof(PALETTEENTRY));
    if (!pPal) return NULL;

    pPal->palVersion = 0x300;
    pPal->palNumEntries = (WORD)nColors;

    BYTE RedRange = (BYTE)((1 << pfd.cRedBits) - 1);
    BYTE GreenRange = (BYTE)((1 << pfd.cGreenBits) - 1);
    BYTE BlueRange = (BYTE)((1 << pfd.cBlueBits) - 1);

    for (int i = 0; i < nColors; i++)
    {
        pPal->palPalEntry[i].peRed = (BYTE)((i >> pfd.cRedShift) & RedRange);
        pPal->palPalEntry[i].peRed = (BYTE)((double)pPal->palPalEntry[i].peRed * 255.0 / RedRange);

        pPal->palPalEntry[i].peGreen = (BYTE)((i >> pfd.cGreenShift) & GreenRange);
        pPal->palPalEntry[i].peGreen = (BYTE)((double)pPal->palPalEntry[i].peGreen * 255.0 / GreenRange);

        pPal->palPalEntry[i].peBlue = (BYTE)((i >> pfd.cBlueShift) & BlueRange);
        pPal->palPalEntry[i].peBlue = (BYTE)((double)pPal->palPalEntry[i].peBlue * 255.0 / BlueRange);

        pPal->palPalEntry[i].peFlags = 0;
    }

    HPALETTE hRetPal = CreatePalette(pPal);
    SelectPalette(hDC, hRetPal, FALSE);
    RealizePalette(hDC);
    free(pPal);

    return hRetPal;
}

unsigned char* LoadBitmapFile(const char* filename, BITMAPINFOHEADER* bitmapInfoHeader)
{
    FILE* filePtr = fopen(filename, "rb");
    if (!filePtr)
    {
        MessageBoxA(NULL, "Nie można otworzyć pliku BMP!", "Błąd", MB_OK | MB_ICONERROR);
        return NULL;
    }

    BITMAPFILEHEADER bitmapFileHeader;
    fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

    if (bitmapFileHeader.bfType != 0x4D42)
    {
        fclose(filePtr);
        MessageBoxA(NULL, "To nie jest prawidłowy plik BMP!", "Błąd", MB_OK | MB_ICONERROR);
        return NULL;
    }

    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

    unsigned char* bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);
    if (!bitmapImage)
    {
        fclose(filePtr);
        return NULL;
    }

    fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);
    fclose(filePtr);

    for (int i = 0; i < (int)bitmapInfoHeader->biSizeImage; i += 3)
    {
        unsigned char temp = bitmapImage[i];
        bitmapImage[i] = bitmapImage[i + 2];
        bitmapImage[i + 2] = temp;
    }

    return bitmapImage;
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    WNDCLASS wc = { 0 };
    HWND hWnd;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "GLTemplate";

    if (!RegisterClass(&wc)) return FALSE;

    hWnd = CreateWindow("GLTemplate", "GL Template", WS_OVERLAPPEDWINDOW,
        50, 50, 800, 600, NULL, NULL, hInst, NULL);

    if (!hWnd) return FALSE;

    ShowWindow(hWnd, nCmdShow);
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
    switch (message)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'T': case 'G': case 'F': case 'H':
            updateTractorPosition((char)wParam);
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
        }
        updateCameraDirection();
        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_SIZE:
        ChangeSize(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_PAINT:
        RenderScene();
        SwapBuffers(GetDC(hWnd));
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void updateCameraDirection()
{
    float radYaw = yaw * M_PI / 180.0f;
    float radPitch = pitch * M_PI / 180.0f;

    centerX = eyeX + cosf(radYaw) * cosf(radPitch);
    centerY = eyeY + sinf(radPitch);
    centerZ = eyeZ + sinf(radYaw) * cosf(radPitch);
}

void RenderScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);

    // Teren
    drawFlatTerrain();

    // Obiekty
    drawBuilding(10.0f, 10.0f);
    drawTree(-10.0f, 5.0f);

    // Traktor
    glEnable(GL_TEXTURE_2D);
    drawTractor();
    glDisable(GL_TEXTURE_2D);
}

void updateTractorPosition(char key)
{
    float angleRad = tractorAngle * M_PI / 180.0f;
    switch (key)
    {
    case 'T': // Forward
        tractorX += cosf(angleRad) * tractorSpeed;
        tractorZ += sinf(angleRad) * tractorSpeed;
        break;
    case 'G': // Backward
        tractorX -= cosf(angleRad) * tractorSpeed;
        tractorZ -= sinf(angleRad) * tractorSpeed;
        break;
    case 'F': // Rotate Left
        tractorAngle += tractorRotSpeed;
        break;
    case 'H': // Rotate Right
        tractorAngle -= tractorRotSpeed;
        break;
    }
}

void SetupRC()
{
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    glShadeModel(GL_SMOOTH);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
}

void drawBuilding(float posX, float posZ)
{
    float size = 5.0f;

    glPushMatrix();
    glTranslatef(posX, 0.0f, posZ);
    glColor3f(0.7f, 0.7f, 0.7f);

    glBegin(GL_QUADS);
    // Ściany budynku
    glVertex3f(-size, 0.0f, -size);
    glVertex3f(size, 0.0f, -size);
    glVertex3f(size, size, -size);
    glVertex3f(-size, size, -size);

    glVertex3f(-size, 0.0f, size);
    glVertex3f(size, 0.0f, size);
    glVertex3f(size, size, size);
    glVertex3f(-size, size, size);

    glVertex3f(-size, size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, size, size);
    glVertex3f(-size, size, size);
    glEnd();

    glPopMatrix();
}

void drawFlatTerrain()
{
    glColor3f(0.2f, 0.8f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-100.0f, 0.0f, -100.0f);
    glVertex3f(100.0f, 0.0f, -100.0f);
    glVertex3f(100.0f, 0.0f, 100.0f);
    glVertex3f(-100.0f, 0.0f, 100.0f);
    glEnd();
}

void drawTree(float posX, float posZ)
{
    glPushMatrix();
    glTranslatef(posX, 0.0f, posZ);

    // Pień drzewa
    glColor3f(0.5f, 0.3f, 0.1f);
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);

    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(quad, 0.5, 0.5, 5.0, 16, 16);
    glPopMatrix();

    // Korona drzewa
    glColor3f(0.0f, 0.8f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 5.0f, 0.0f);
    glRotatef(-90, 1, 0, 0);
    gluCylinder(quad, 3.0, 0.0, 4.0, 16, 16);
    glPopMatrix();

    gluDeleteQuadric(quad);
    glPopMatrix();
}


void RenderScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);

    drawFlatTerrain();
    drawBuilding(10.0f, 10.0f);
    drawTree(-10.0f, 5.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    drawTractor();
    glDisable(GL_TEXTURE_2D);
}


void ChangeSize(GLsizei w, GLsizei h)
{
    if (h == 0) h = 1;
    GLfloat aspect = (GLfloat)w / (GLfloat)h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, aspect, 1.0f, 400.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

