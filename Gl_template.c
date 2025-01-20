#define _CRT_SECURE_NO_WARNINGS
#define M_PI 3.141592653589793238462643

#ifdef _MSC_VER
#  pragma comment(lib, "opengl32.lib")
#  pragma comment(lib, "glu32.lib")
#endif
#include <AntTweakBar.h>
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>     // Do rand(), srand()
#include <time.h>       // Do time(NULL)
#include "resource.h"   // tu są IDR_MENU, itp.

// -------------------------------------------------------------
// Fabuła (w skrócie):
// Gra przenosi nas na rozległe gospodarstwo, gdzie kierujemy
// traktorem. W losowych miejscach na terenie pojawiają się
// skrzynki/bloki (np. "ładunki do zebrania"). Zadaniem gracza jest
// objechanie pola i zebranie wszystkich bloczków poprzez przejechanie
// traktorem w ich pobliżu. Każdy zebrany bloczek znika z planszy
// i zwiększa liczbę punktów. Celem jest zebranie jak największej
// liczby bloczków.
//
// Sterowanie:
// - Kamera: W, S, A, D (ruch przód, tył, lewo, prawo), Q/E (góra/dół)
// - Obrót kamery: strzałki (← → → yaw, ↑ ↓ → pitch)
// - Ruch traktora: T (przód), G (tył), F (skręt w lewo), H (skręt w prawo)
//
// Miłej zabawy!
//
// -------------------------------------------------------------

// Pomocnicza definicja koloru w trybie RGB (niekoniecznie potrzebna)
#define glRGB(x, y, z)	glColor3ub((GLubyte)x, (GLubyte)y, (GLubyte)z)

// --- STAŁE i ZMIENNE GLOBALNE ---
HPALETTE hPalette = NULL;

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

// --- Tekstury i BMP ---
BITMAPINFOHEADER bitmapInfoHeader;
unsigned char* bitmapData = NULL;
unsigned int  texture[2];

// --- Bloczki do zbierania (pozycje + zebrane/niezebrane) ---
#define NUM_BLOCKS 50  // liczba losowych bloczków
static float blockPositions[NUM_BLOCKS][2];  // X, Z dla każdego bloczka
static int   blockCollected[NUM_BLOCKS];     // 0 - nie, 1 - tak
static int   score = 0;                     // liczba zebranych bloczków

// --- Deklaracje funkcji ---
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL    CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void SetDCPixelFormat(HDC hDC);
HPALETTE GetOpenGLPalette(HDC hDC);

unsigned char* LoadBitmapFile(const char* filename, BITMAPINFOHEADER* bitmapInfoHeader);

void updateCameraDirection();
void ChangeSize(GLsizei w, GLsizei h);
void SetupRC();
void RenderScene();

// Rysowanie obiektów
void drawBuilding(float posX, float posZ);
void drawFlatTerrain();
void drawTree(float posX, float posZ);
void drawBlocks();          // NOWA funkcja do rysowania bloczków
void checkBlockCollisions(); // NOWA funkcja do sprawdzania kolizji z bloczkami

// Zewnętrzna funkcja rysująca traktor (z pliku „TraktorTekstury.c”)
void drawTractor();

// ---------------------------------------------------------
// FUNKCJA wczytująca plik BMP (BGR → RGB) do pamięci
// ---------------------------------------------------------
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

    // Sprawdzenie "BM" (0x4D42)
    if (bitmapFileHeader.bfType != 0x4D42)
    {
        fclose(filePtr);
        MessageBoxA(NULL, "To nie jest prawidłowy plik BMP!", "Błąd", MB_OK | MB_ICONERROR);
        return NULL;
    }

    // Odczyt nagłówka BITMAPINFOHEADER
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

    // Przesuwamy się do początku danych bitmapy
    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

    // Rezerwacja pamięci na piksele
    unsigned char* bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);
    if (!bitmapImage)
    {
        fclose(filePtr);
        return NULL;
    }

    // Wczytujemy same piksele
    fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);
    fclose(filePtr);

    // Pliki BMP są w formacie BGR, a OpenGL domyślnie oczekuje RGB
    // Trzeba więc zamienić kolejność kanałów.
    for (int i = 0; i < (int)bitmapInfoHeader->biSizeImage; i += 3)
    {
        unsigned char temp = bitmapImage[i];
        bitmapImage[i] = bitmapImage[i + 2];
        bitmapImage[i + 2] = temp;
    }

    return bitmapImage;
}

// ---------------------------------------------------------
// GŁÓWNA FUNKCJA
// ---------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
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
        800, 600,   // nieco większe okno
        NULL,
        NULL,
        hInstance,
        NULL
    );

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

// ---------------------------------------------------------
// Okno: procedura
// ---------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HGLRC hRC;
    static HDC   hDC;

    switch (message)
    {
    case WM_CREATE:
    {
        hDC = GetDC(hWnd);
        SetDCPixelFormat(hDC);
        hPalette = GetOpenGLPalette(hDC);

        hRC = wglCreateContext(hDC);
        wglMakeCurrent(hDC, hRC);

        SetupRC();

        // Inicjalizacja generatora liczb losowych (dla bloczków)
        srand((unsigned)time(NULL));

        // Generujemy ID tekstur
        glGenTextures(2, texture);

        //---------------------------------
        // 1. Wczytaj do texture[0]
        //---------------------------------
        bitmapData = LoadBitmapFile("metal.bmp", &bitmapInfoHeader);
        if (bitmapData)
        {
            glBindTexture(GL_TEXTURE_2D, texture[0]);

            // Ustaw filtry na GL_LINEAR (lepsza jakość niż NEAREST)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            // Sposób zawijania
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

            // Stworzenie tekstury
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                bitmapInfoHeader.biWidth,
                bitmapInfoHeader.biHeight,
                0, GL_RGB, GL_UNSIGNED_BYTE, bitmapData);

            free(bitmapData);
            bitmapData = NULL;

            // Ustaw tryb nakładania na DECAL
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        }

        //---------------------------------
        // 2. Wczytaj do texture[1]
        //---------------------------------
        bitmapData = LoadBitmapFile("takiecos.bmp", &bitmapInfoHeader);
        if (bitmapData)
        {
            glBindTexture(GL_TEXTURE_2D, texture[1]);

            // Ustaw filtry na GL_LINEAR (lepsza jakość niż NEAREST)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            // Sposób zawijania
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

            // Stworzenie tekstury
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                bitmapInfoHeader.biWidth,
                bitmapInfoHeader.biHeight,
                0, GL_RGB, GL_UNSIGNED_BYTE, bitmapData);

            free(bitmapData);
            bitmapData = NULL;

            // Ustaw tryb nakładania na DECAL
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        }

        // Ustawiamy tryb mieszania koloru z teksturą
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        // Losowe położenia bloczków
        srand((unsigned)time(NULL));
        for (int i = 0; i < NUM_BLOCKS; i++)
        {
            blockPositions[i][0] = (float)(rand() % 180 - 90);  // X z przedziału -90..+89
            blockPositions[i][1] = (float)(rand() % 180 - 90);  // Z z przedziału -90..+89
            blockCollected[i] = 0;
        }


        return 0;
    }
    case WM_DESTROY:
        wglMakeCurrent(hDC, NULL);
        wglDeleteContext(hRC);
        if (hPalette)
            DeleteObject(hPalette);
        PostQuitMessage(0);
        break;

    case WM_SIZE:
        ChangeSize((GLsizei)LOWORD(lParam), (GLsizei)HIWORD(lParam));
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);

        // Rysujemy pełną scenę
        RenderScene();

        // Zamiana buforów
        SwapBuffers(hDC);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_KEYDOWN:
    {
        // Ruchy kamery + obrót (yaw, pitch)
        float frontX = centerX - eyeX;
        float frontY = centerY - eyeY;
        float frontZ = centerZ - eyeZ;
        float length = sqrtf(frontX * frontX + frontY * frontY + frontZ * frontZ);

        if (length != 0.0f)
        {
            frontX /= length;
            frontY /= length;
            frontZ /= length;
        }

        float rightX = cosf((yaw - 90.0f) * (float)M_PI / 180.0f);
        float rightZ = sinf((yaw - 90.0f) * (float)M_PI / 180.0f);

        switch (wParam)
        {
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

            // Traktor
        case 'T': // Do przodu
        {
            float angleRad = (tractorAngle + 90.0f) * (float)M_PI / 180.0f;
            tractorX += sinf(angleRad) * tractorSpeed;
            tractorZ += cosf(angleRad) * tractorSpeed;
            checkBlockCollisions(); // sprawdź czy zebraliśmy jakiś bloczek
        }
        break;

        case 'G': // Do tyłu
        {
            float angleRad = (tractorAngle + 90.0f) * (float)M_PI / 180.0f;
            tractorX -= sinf(angleRad) * tractorSpeed;
            tractorZ -= cosf(angleRad) * tractorSpeed;
            checkBlockCollisions(); // sprawdź czy zebraliśmy jakiś bloczek
        }
        break;
        case 'F': // Obrót w lewo
            tractorAngle += tractorRotSpeed;
            checkBlockCollisions();
            break;

        case 'H': // Obrót w prawo
            tractorAngle -= tractorRotSpeed;
            checkBlockCollisions();
            break;
        }

        updateCameraDirection();
        InvalidateRect(hWnd, NULL, FALSE);
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
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0L;
}

// ---------------------------------------------------------
// Dialog "About"
// ---------------------------------------------------------
BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        // Można tu wypisać np. glGetString(GL_VENDOR), GL_RENDERER, itp.
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

// ---------------------------------------------------------
// Ustawianie formatu pikseli
// ---------------------------------------------------------
void SetDCPixelFormat(HDC hDC)
{
    int nPixelFormat;
    static PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,    // double buffering
        PFD_TYPE_RGBA,
        24,                  // kolor 24-bit
        0,0,0,0,0,0,
        0,0,
        0,0,0,0,0,
        32,  // 24-bit głębia + 8-bit stencil?  (lub 32)
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0,0,0
    };

    nPixelFormat = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, nPixelFormat, &pfd);
}

// ---------------------------------------------------------
// Paleta (w starych kartach - raczej dziś nieużywane)
// ---------------------------------------------------------
HPALETTE GetOpenGLPalette(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE* pPal;
    int nPixelFormat = GetPixelFormat(hDC);
    DescribePixelFormat(hDC, nPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    // Jeżeli nie ma flagi PFD_NEED_PALETTE, to nie tworzymy
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

// ---------------------------------------------------------
// Funkcje pomocnicze
// ---------------------------------------------------------
void updateCameraDirection()
{
    float frontX = cosf(yaw * (float)M_PI / 180.0f) * cosf(pitch * (float)M_PI / 180.0f);
    float frontY = sinf(pitch * (float)M_PI / 180.0f);
    float frontZ = sinf(yaw * (float)M_PI / 180.0f) * cosf(pitch * (float)M_PI / 180.0f);

    centerX = eyeX + frontX;
    centerY = eyeY + frontY;
    centerZ = eyeZ + frontZ;
}

void ChangeSize(GLsizei w, GLsizei h)
{
    if (h == 0) h = 1;

    lastWidth = w;
    lastHeight = h;
    GLfloat fAspect = (GLfloat)w / (GLfloat)h;

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
    glShadeModel(GL_SMOOTH);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
}

// ---------------------------------------------------------
// Rysowanie płaskiego terenu
// ---------------------------------------------------------
void drawFlatTerrain()
{
    // Prosty kwadratowy teren (zielony)
    glColor3f(0.2f, 0.8f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-100.0f, 0.0f, -100.0f);
    glVertex3f(100.0f, 0.0f, -100.0f);
    glVertex3f(100.0f, 0.0f, 100.0f);
    glVertex3f(-100.0f, 0.0f, 100.0f);
    glEnd();
}

// ---------------------------------------------------------
// Rysowanie prostego budynku
// ---------------------------------------------------------
void drawBuilding(float posX, float posZ)
{
    float h = 0.0f;  // teren jest płaski
    float size = 5.0f;

    glPushMatrix();
    glTranslatef(posX, h, posZ);

    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
    // Góra
    glVertex3f(-size, size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, size, size);
    glVertex3f(-size, size, size);

    // Dół
    glVertex3f(-size, 0.0f, -size);
    glVertex3f(size, 0.0f, -size);
    glVertex3f(size, 0.0f, size);
    glVertex3f(-size, 0.0f, size);

    // Przód
    glVertex3f(-size, 0.0f, size);
    glVertex3f(size, 0.0f, size);
    glVertex3f(size, size, size);
    glVertex3f(-size, size, size);

    // Tył
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

// ---------------------------------------------------------
// Drzewo (pień + korona)
// ---------------------------------------------------------
void drawTree(float posX, float posZ)
{
    glPushMatrix();
    glTranslatef(posX, 0.0f, posZ);

    // Pień
    glColor3f(0.5f, 0.3f, 0.1f);
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);

    // Pień: walec
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(quad, 0.5, 0.5, 5.0, 16, 16);
    glPopMatrix();

    // Korona: stożek
    glColor3f(0.0f, 0.8f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 5.0f, 0.0f);
    glRotatef(-90, 1, 0, 0);
    gluCylinder(quad, 3.0, 0.0, 4.0, 16, 16);
    glPopMatrix();

    gluDeleteQuadric(quad);
    glPopMatrix();
}

// ---------------------------------------------------------
// Rysowanie bloczków do zbierania
// ---------------------------------------------------------
void drawBlocks()
{
    // Prosta forma bloczków: małe sześciany
    float halfSize = 1.0f;

    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (blockCollected[i] == 1)
            continue;  // pomijamy już zebrane bloczki

        float bx = blockPositions[i][0];
        float bz = blockPositions[i][1];

        glPushMatrix();
        glTranslatef(bx, 0.0f, bz);
        glColor3f(1.0f, 0.5f, 0.0f); // pomarańcz

        glBegin(GL_QUADS);
        // Góra
        glVertex3f(-halfSize, halfSize, -halfSize);
        glVertex3f(halfSize, halfSize, -halfSize);
        glVertex3f(halfSize, halfSize, halfSize);
        glVertex3f(-halfSize, halfSize, halfSize);

        // Dół
        glVertex3f(-halfSize, 0.0f, -halfSize);
        glVertex3f(halfSize, 0.0f, -halfSize);
        glVertex3f(halfSize, 0.0f, halfSize);
        glVertex3f(-halfSize, 0.0f, halfSize);

        // Przód
        glVertex3f(-halfSize, 0.0f, halfSize);
        glVertex3f(halfSize, 0.0f, halfSize);
        glVertex3f(halfSize, halfSize, halfSize);
        glVertex3f(-halfSize, halfSize, halfSize);

        // Tył
        glVertex3f(-halfSize, 0.0f, -halfSize);
        glVertex3f(-halfSize, halfSize, -halfSize);
        glVertex3f(halfSize, halfSize, -halfSize);
        glVertex3f(halfSize, 0.0f, -halfSize);

        // Lewo
        glVertex3f(-halfSize, 0.0f, -halfSize);
        glVertex3f(-halfSize, 0.0f, halfSize);
        glVertex3f(-halfSize, halfSize, halfSize);
        glVertex3f(-halfSize, halfSize, -halfSize);

        // Prawo
        glVertex3f(halfSize, 0.0f, -halfSize);
        glVertex3f(halfSize, 0.0f, halfSize);
        glVertex3f(halfSize, halfSize, halfSize);
        glVertex3f(halfSize, halfSize, -halfSize);
        glEnd();

        glPopMatrix();
    }
}

// ---------------------------------------------------------
// Sprawdzenie, czy traktor zebrał bloczek
// ---------------------------------------------------------
void checkBlockCollisions()
{
    // Prostą metodą jest sprawdzenie odległości (2D) pomiędzy
    // środkiem traktora (tractorX, tractorZ) a pozycją bloczka.
    // Jeśli jest < pewnego promienia, to uznajemy bloczek za zebrany.
    float collisionRadius = 6.0f;

    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (blockCollected[i] == 1) continue;

        float dx = tractorX - blockPositions[i][0];
        float dz = tractorZ - blockPositions[i][1];
        float dist = sqrtf(dx * dx + dz * dz);

        if (dist < collisionRadius)
        {
            // Zebraliśmy bloczek
            blockCollected[i] = 1;
            score++;
            // (Można tu np. dodać dźwięk, wyświetlenie napisu, itp.)
        }
    }
}

// ---------------------------------------------------------
// Render całej sceny
// ---------------------------------------------------------
void RenderScene()
{
    // 1) Wyczyszczenie buforów
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2) Ustaw tryb macierzy MODELVIEW i zresetuj macierz
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 3) Ustawienie kamery
    gluLookAt(eyeX, eyeY, eyeZ,
        centerX, centerY, centerZ,
        upX, upY, upZ);

    // 4) Rysowanie sceny 3D

    // a) Teren
    drawFlatTerrain();

    // b) Przykładowe obiekty (budynek i drzewo)
    drawBuilding(10.0f, 10.0f);
    drawTree(-10.0f, 5.0f);

    // c) Rysowanie bloczków (do zebrania)
    drawBlocks();

    // d) Włącz teksturowanie i narysuj traktor
    glEnable(GL_TEXTURE_2D);

    glPushMatrix();
    {
        glTranslatef(tractorX, 1.0f, tractorZ);
        glRotatef(tractorAngle, 0.0f, 1.0f, 0.0f);

        // Ustaw aktualną teksturę (np. kabina/maska)
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        glColor3f(1.0f, 1.0f, 1.0f);

        // Rysowanie traktora
        drawTractor();
    }
    glPopMatrix();

    // e) Sprawdzenie kolizji (czy traktor zebrał któryś bloczek)
    checkBlockCollisions();

    // f) Wyłącz teksturowanie (jeśli chcesz rysować obiekty bez tekstur dalej)
    glDisable(GL_TEXTURE_2D);

    // 5) Rysowanie nakładki 2D (HUD, tekst itp.) w trybie ortho
    //    Zapamiętujemy bieżący tryb projekcji i macierzy, żeby potem przywrócić.

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    {
        // Ustaw rzut ortograficzny na rozmiar okna
        glLoadIdentity();
        gluOrtho2D(0, lastWidth, 0, lastHeight);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        {
            glLoadIdentity();

            // PRZYKŁAD 1: (Pseudo-kod z GLUT, jeśli mamy glut)
            /*
            glColor3f(0, 0, 0);
            glRasterPos2i(10, lastHeight - 20);
            char buf[64];
            sprintf(buf, "Zebrane bloczki: %d / %d", score, NUM_BLOCKS);
            for (char *c = buf; *c; c++)
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
            */

            // PRZYKŁAD 2: (Windows GDI - może migotać w double-buffer)
            HDC hDC = wglGetCurrentDC();
            SetBkMode(hDC, TRANSPARENT);
            SetTextColor(hDC, RGB(0, 0, 0));
            char text[64];
            sprintf(text, "Zebrane bloczki: %d / %d", score, NUM_BLOCKS);
            TextOutA(hDC, 10, 10, text, strlen(text));
        }
        // Przywracamy macierz MODELVIEW
        glPopMatrix();
    }
    // Przywracamy macierz PROJECTION
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Na koniec wracamy do trybu MODELVIEW (dobry nawyk)
    glMatrixMode(GL_MODELVIEW);
}
