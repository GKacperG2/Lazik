// -------------------------------------------------------------------
// Plik: TraktorTekstury.c (przyk³adowa nazwa)
// Kod do rysowania traktora z dodanymi wspó³rzêdnymi teksturowania
// -------------------------------------------------------------------
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
extern unsigned int texture[2];

// -------------------------------------------------------------------
// Funkcja: prostopadloscian
// Rysuje obiekt w kszta³cie prostopad³oœcianu, z pêtlami po 0.1
// W ka¿dej pêtli zosta³y dodane wspó³rzêdne tekstur (glTexCoord2f).
// -------------------------------------------------------------------
void prostopadloscian(double a, double b, double offsetX, double offsetY, double offsetZ)
{
    // 1. Pierwsza œciana
    //    z w [0..20], x w [-10..95], y = 0
    for (float z = 0.0f; z <= 20.0f; z += 0.1f)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for (float x = -10.0f; x <= 95.0f; x += 0.1f)
        {
            glColor3f(0.8f, 0.0f, 1.0f);

            float u1 = (x + 10.0f) / 105.0f;   // x idzie od -10 do 95 (rozpiêtoœæ 105)
            float v1 = z / 20.0f;            // z idzie od 0 do 20 (rozpiêtoœæ 20)

            glTexCoord2f(u1, v1);
            glVertex3d(x + offsetX, offsetY, z + offsetZ);

            float v2 = (z + 0.1f) / 20.0f;
            glTexCoord2f(u1, v2);
            glVertex3d(x + offsetX, offsetY, (z + 0.1) + offsetZ);
        }
        glEnd();
    }

    // 2. Druga œciana
    //    z w [0..20], x w [-10..95], tutaj rysujemy w p³aszczyŸnie "Z=offsetZ?" -> glVertex(..., z+offsetY, 0+offsetZ)
    for (float z = 0.0f; z <= 20.0f; z += 0.1f)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glColor3f(1.0f, 1.0f, 0.0f);
        for (float x = -10.0f; x <= 95.0f; x += 0.1f)
        {
            float u1 = (x + 10.0f) / 105.0f;
            float v1 = z / 20.0f;

            glTexCoord2f(u1, v1);
            glVertex3d(x + offsetX, z + offsetY, offsetZ);

            float v2 = (z + 0.1f) / 20.0f;
            glTexCoord2f(u1, v2);
            glVertex3d(x + offsetX, (z + 0.1f) + offsetY, offsetZ);
        }
        glEnd();
    }

    // 3. Trzecia œciana
    //    z w [0..20], x w [0..20], ale wierzcho³ek ma X = -10 + offsetX
    //    Z kolei w glVertex: -10 + offsetX jest sta³e dla X w pêtli, roœnie "x" po osi, wchodzi jako "z" w 3D
    for (float z = 0.0f; z <= 20.0f; z += 0.1f)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glColor3d(0.8, 0.0, 0.0);
        for (float x = 0.0f; x <= 20.0f; x += 0.1f)
        {
            float u1 = x / 20.0f;     // x w [0..20]
            float v1 = z / 20.0f;     // z w [0..20]

            glTexCoord2f(u1, v1);
            glVertex3d(-10.0 + offsetX, z + offsetY, x + offsetZ);

            float v2 = (z + 0.1f) / 20.0f;
            glTexCoord2f(u1, v2);
            glVertex3d(-10.0 + offsetX, (z + 0.1f) + offsetY, x + offsetZ);
        }
        glEnd();
    }

    // 4. Czwarta œciana
    //    z w [0..20], x w [-10..95], y = 20
    for (float z = 0.0f; z <= 20.0f; z += 0.1f)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glColor3f(1.0f, 0.0f, 0.0f);
        for (float x = -10.0f; x <= 95.0f; x += 0.1f)
        {
            float u1 = (x + 10.0f) / 105.0f;
            float v1 = z / 20.0f;

            glTexCoord2f(u1, v1);
            glVertex3d(x + offsetX, 20.0 + offsetY, z + offsetZ);

            float v2 = (z + 0.1f) / 20.0f;
            glTexCoord2f(u1, v2);
            glVertex3d(x + offsetX, 20.0 + offsetY, (z + 0.1f) + offsetZ);
        }
        glEnd();
    }

    // 5. Pi¹ta œciana
    //    x w [-10..95], y w [0..20], z = 20
    for (float x = -10.0f; x <= 95.0f; x += 0.1f)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glColor3f(0.0f, 1.0f, 0.0f);
        for (float y = 0.0f; y <= 20.0f; y += 0.1f)
        {
            float u1 = (x + 10.0f) / 105.0f;  // x w [-10..95]
            float v1 = y / 20.0f;            // y w [0..20]

            glTexCoord2f(u1, v1);
            glVertex3d(x + offsetX, y + offsetY, 20.0 + offsetZ);

            float u2 = ((x + 0.1f) + 10.0f) / 105.0f;
            glTexCoord2f(u2, v1);
            glVertex3d((x + 0.1f) + offsetX, y + offsetY, 20.0 + offsetZ);
        }
        glEnd();
    }

    // 6. Szósta œciana
    //    z w [0..20], x w [0..20], "pionowo" przy X = 95 + offsetX
    for (float z = 0.0f; z <= 20.0f; z += 0.1f)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glColor3d(0.8, 0.0, 0.0);
        for (float x = 0.0f; x <= 20.0f; x += 0.1f)
        {
            float u1 = x / 20.0f;
            float v1 = z / 20.0f;

            glTexCoord2f(u1, v1);
            glVertex3d(95.0 + offsetX, z + offsetY, x + offsetZ);

            float v2 = (z + 0.1f) / 20.0f;
            glTexCoord2f(u1, v2);
            glVertex3d(95.0 + offsetX, (z + 0.1f) + offsetY, x + offsetZ);
        }
        glEnd();
    }
}

// -------------------------------------------------------------------
// Funkcja: prostopadloscian1
// Bardzo podobna do powy¿szej – tutaj x, z w [0..20], a w pêtlach
// te¿ dodano glTexCoord2f(...).
// -------------------------------------------------------------------
void prostopadloscian1(double a, double b, double offsetX, double offsetY, double offsetZ)
{
    // 1. Pierwsza œciana
    for (float z = 0.0f; z <= 20.0f; z += 0.1f)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for (float x = 0.0f; x <= 20.0f; x += 0.1f)
        {
            glColor3f(0.8f, 0.0f, 1.0f);

            float u1 = x / 20.0f;
            float v1 = z / 20.0f;

            glTexCoord2f(u1, v1);
            glVertex3d(x + offsetX, offsetY, z + offsetZ);

            float v2 = (z + 0.1f) / 20.0f;
            glTexCoord2f(u1, v2);
            glVertex3d(x + offsetX, offsetY, (z + 0.1f) + offsetZ);
        }
        glEnd();
    }

    // 2. Druga œciana
    for (float z = 0.0f; z <= 20.0f; z += 0.1f)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glColor3f(1.0f, 1.0f, 0.0f);
        for (float x = 0.0f; x <= 20.0f; x += 0.1f)
        {
            float u1 = x / 20.0f;
            float v1 = z / 20.0f;

            glTexCoord2f(u1, v1);
            glVertex3d(x + offsetX, z + offsetY, offsetZ);

            float v2 = (z + 0.1f) / 20.0f;
            glTexCoord2f(u1, v2);
            glVertex3d(x + offsetX, (z + 0.1f) + offsetY, offsetZ);
        }
        glEnd();
    }

    // 3. Trzecia œciana
    for (float z = 0.0f; z <= 20.0f; z += 0.1f)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glColor3d(0.8, 0.0, 0.0);
        for (float x = 0.0f; x <= 20.0f; x += 0.1f)
        {
            float u1 =  x / 20.0f;
            float v1 =  z / 20.0f;

            glTexCoord2f(u1, v1);
            glVertex3d(offsetX, z + offsetY, x + offsetZ);

            float v2 = (z + 0.1f) / 20.0f;
            glTexCoord2f(u1, v2);
            glVertex3d(offsetX, (z + 0.1f) + offsetY, x + offsetZ);
        }
        glEnd();
    }

    // 4. Czwarta œciana
    for (float z = 0.0f; z <= 20.0f; z += 0.1f)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glColor3f(1.0f, 0.0f, 0.0f);
        for (float x = 0.0f; x <= 20.0f; x += 0.1f)
        {
            float u1 = x / 20.0f;
            float v1 = z / 20.0f;

            glTexCoord2f(u1, v1);
            glVertex3d(x + offsetX, 20.0 + offsetY, z + offsetZ);

            float v2 = (z + 0.1f) / 20.0f;
            glTexCoord2f(u1, v2);
            glVertex3d(x + offsetX, 20.0 + offsetY, (z + 0.1f) + offsetZ);
        }
        glEnd();
    }

    // 5. Pi¹ta œciana
    for (float x = 0.0f; x <= 20.0f; x += 0.1f)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glColor3f(0.0f, 1.0f, 0.0f);
        for (float y = 0.0f; y <= 20.0f; y += 0.1f)
        {
            float u1 = x / 20.0f;
            float v1 = y / 20.0f;

            glTexCoord2f(u1, v1);
            glVertex3d(x + offsetX, y + offsetY, 20.0 + offsetZ);

            float u2 = (x + 0.1f) / 20.0f;
            glTexCoord2f(u2, v1);
            glVertex3d((x + 0.1f) + offsetX, y + offsetY, 20.0 + offsetZ);
        }
        glEnd();
    }

    // 6. Szósta œciana
    for (float z = 0.0f; z <= 20.0f; z += 0.1f)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glColor3d(0.8, 0.0, 0.0);
        for (float x = 0.0f; x <= 20.0f; x += 0.1f)
        {
            float u1 = x / 20.0f;
            float v1 = z / 20.0f;

            glTexCoord2f(u1, v1);
            glVertex3d(20.0 + offsetX, z + offsetY, x + offsetZ);

            float v2 = (z + 0.1f) / 20.0f;
            glTexCoord2f(u1, v2);
            glVertex3d(20.0 + offsetX, (z + 0.1f) + offsetY, x + offsetZ);
        }
        glEnd();
    }
}

// -------------------------------------------------------------------
// Funkcja: felga
// Rysuje walec z otworem (podobny do walca), z dwiema podstawami
// i œciank¹ boczn¹. Dodano glTexCoord2f(...).
// -------------------------------------------------------------------
void felga(double r, double h, double offsetX, double offsetY, double offsetZ)
{
    const double PI = 3.14;
    double x, y, alpha;

    // Podstawa dolna (TRIANGLE_FAN)
    glBegin(GL_TRIANGLE_FAN);
    glColor3d(0.8, 1.0, 0.0);

    // Œrodek podstawy dolnej
    glTexCoord2f(0.5f, 0.5f);
    glVertex3d(offsetX, offsetY, offsetZ);

    // Obrze¿e
    for (alpha = 0; alpha <= 2 * PI + 0.01; alpha += PI / 128.0)
    {
        x = r * sin(alpha) + offsetX;
        y = r * cos(alpha) + offsetY;

        float u = 0.5f + 0.5f * (float)sin(alpha);
        float v = 0.5f + 0.5f * (float)cos(alpha);

        glTexCoord2f(u, v);
        glVertex3d(x, y, offsetZ);
    }
    glEnd();

    // Œciany boczne (QUAD_STRIP)
    glBegin(GL_QUAD_STRIP);
    for (alpha = 0.0; alpha <= 2 * PI + 0.01; alpha += PI / 128.0)
    {
        x = r * sin(alpha) + offsetX;
        y = r * cos(alpha) + offsetY;

        // u w [0..1] wzd³u¿ obwodu
        float u = (float)(alpha / (2.0 * PI));

        // Dó³ -> v=0, Góra -> v=1
        glColor3f(1.0f, 1.0f, 1.0f);

        glTexCoord2f(u, 0.0f);
        glVertex3d(x, y, offsetZ);

        glTexCoord2f(u, 1.0f);
        glVertex3d(x, y, offsetZ + h);
    }
    glEnd();

    // Podstawa górna (TRIANGLE_FAN)
    glBegin(GL_TRIANGLE_FAN);
    glColor3d(0.8, 1.0, 0.0);

    // Œrodek podstawy górnej
    glTexCoord2f(0.5f, 0.5f);
    glVertex3d(offsetX, offsetY, offsetZ + h);

    // Obrze¿e
    for (alpha = 0; alpha <= 2 * PI + 0.01; alpha += PI / 128.0)
    {
        x = r * sin(alpha) + offsetX;
        y = r * cos(alpha) + offsetY;

        float u = 0.5f + 0.5f * (float)sin(alpha);
        float v = 0.5f + 0.5f * (float)cos(alpha);

        glTexCoord2f(u, v);
        glVertex3d(x, y, offsetZ + h);
    }
    glEnd();
}

// -------------------------------------------------------------------
// Funkcja: walec
// Klasyczny walec (pe³ny) – równie¿ z dodanymi wspó³rzêdnymi tekstur.
// -------------------------------------------------------------------
void walec(double r, double h, double offsetX, double offsetY, double offsetZ)
{
    const double PI = 3.14;
    double x, y, alpha;

    // Podstawa dolna
    glBegin(GL_TRIANGLE_FAN);
    glColor3d(0.8, 0.0, 0.0);

    glTexCoord2f(0.5f, 0.5f);
    glVertex3d(offsetX, offsetY, offsetZ);

    for (alpha = 0; alpha <= 2 * PI + 0.01; alpha += PI / 128.0)
    {
        x = r * sin(alpha) + offsetX;
        y = r * cos(alpha) + offsetY;

        float u = 0.5f + 0.5f * (float)sin(alpha);
        float v = 0.5f + 0.5f * (float)cos(alpha);

        glTexCoord2f(u, v);
        glVertex3d(x, y, offsetZ);
    }
    glEnd();

    // Œciany boczne
    glBegin(GL_QUAD_STRIP);
    for (alpha = 0.0; alpha <= 2 * PI + 0.01; alpha += PI / 128.0)
    {
        x = r * sin(alpha) + offsetX;
        y = r * cos(alpha) + offsetY;

        float u = (float)(alpha / (2.0 * PI));

        glTexCoord2f(u, 0.0f);
        glVertex3d(x, y, offsetZ);

        glTexCoord2f(u, 1.0f);
        glVertex3d(x, y, offsetZ + h);
    }
    glEnd();

    // Podstawa górna
    glBegin(GL_TRIANGLE_FAN);
    glColor3d(0.8, 0.0, 0.0);

    glTexCoord2f(0.5f, 0.5f);
    glVertex3d(offsetX, offsetY, offsetZ + h);

    for (alpha = 0; alpha <= 2 * PI + 0.01; alpha += PI / 128.0)
    {
        x = r * sin(alpha) + offsetX;
        y = r * cos(alpha) + offsetY;

        float u = 0.5f + 0.5f * (float)sin(alpha);
        float v = 0.5f + 0.5f * (float)cos(alpha);

        glTexCoord2f(u, v);
        glVertex3d(x, y, offsetZ + h);
    }
    glEnd();
}

// -------------------------------------------------------------------
// Funkcja: drawTractor
// G³ówna funkcja rysuj¹ca ca³y traktor ze wszystkich bry³
// (prostopad³oœciany, walce, felgi). Dodano tylko teksturowanie
// wewn¹trz poszczególnych bry³ – ca³a reszta logiki zostaje.
// -------------------------------------------------------------------
void drawTractor()
{
    glPushMatrix();
    glScalef(0.1f, 0.1f, 0.1f);

    glTranslatef(-42.5f, 1.0f, -29.0f);

    //-----------------------------------------
    // 1) Kabina i maska => tekstura[0]
    //-----------------------------------------
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Kabina
    prostopadloscian1(10, 10, 0, 20, 20);

    // Maska
    prostopadloscian(10, 10, 0, 0, 20);

    //-----------------------------------------
    // 2) Ko³a i felgi => tekstura[1]
    //-----------------------------------------
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Walce (np. elementy konstrukcji kó³)
    walec(10, 10, 0, 0, 0);
    walec(10, 10, 0, 0, 50);
    walec(2.5f, 40.0f, 0, 0, 10);
    walec(2.5f, 40.0f, 90, 0, 10);
    walec(8, 10, 90, 0, 0);
    walec(8, 10, 90, 0, 50);

    // Felgi (ko³a)
    felga(8, 2, 0, 0, -2);
    felga(8, 2, 0, 0, 60);
    felga(6, 2, 90, 0, -2);
    felga(6, 2, 90, 0, 60);

    glPopMatrix();
}