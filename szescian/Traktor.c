// TraktorTekstury.c
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

#define glRGB(x, y, z) glColor3ub((GLubyte)x, (GLubyte)y, (GLubyte)z)
#define GL_PI 3.14f

extern unsigned int texture[2];
extern float tractorX, tractorZ, tractorAngle;

void prostopadloscian1(double a, double b, double offsetX, double offsetY, double offsetZ);
void walec(double r, double h, double offsetX, double offsetY, double offsetZ);
void felga(double r, double h, double offsetX, double offsetY, double offsetZ);

void drawTractor()
{
    glPushMatrix();
    glTranslatef(tractorX, 0.0f, tractorZ);
    glRotatef(tractorAngle, 0.0f, 1.0f, 0.0f);
    glScalef(0.1f, 0.1f, 0.1f);

    //-----------------------------------------
    // 1) Kabina i maska => tekstura[0]
    //-----------------------------------------
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Kabina
    prostopadloscian1(10, 10, 0, 20, 20);

    // Maska
    prostopadloscian1(10, 10, 0, 0, 20);

    //-----------------------------------------
    // 2) Ko³a i felgi => tekstura[1]
    //-----------------------------------------
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Przednie ko³a
    walec(10, 10, 0, 0, 0);
    walec(10, 10, 0, 0, 50);

    // Tylne ko³a
    walec(8, 10, 90, 0, 0);
    walec(8, 10, 90, 0, 50);

    // Felgi przednie
    felga(8, 2, 0, 0, -2);
    felga(8, 2, 0, 0, 60);

    // Felgi tylne
    felga(6, 2, 90, 0, -2);
    felga(6, 2, 90, 0, 60);

    glPopMatrix();
}

void prostopadloscian1(double a, double b, double offsetX, double offsetY, double offsetZ)
{
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.5f, 0.5f);

    // Front
    glTexCoord2f(0.0f, 0.0f); glVertex3f(offsetX, offsetY, offsetZ);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(offsetX + a, offsetY, offsetZ);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(offsetX + a, offsetY + b, offsetZ);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(offsetX, offsetY + b, offsetZ);

    // Back
    glTexCoord2f(0.0f, 0.0f); glVertex3f(offsetX, offsetY, offsetZ - a);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(offsetX + a, offsetY, offsetZ - a);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(offsetX + a, offsetY + b, offsetZ - a);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(offsetX, offsetY + b, offsetZ - a);

    // Top
    glTexCoord2f(0.0f, 0.0f); glVertex3f(offsetX, offsetY + b, offsetZ);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(offsetX + a, offsetY + b, offsetZ);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(offsetX + a, offsetY + b, offsetZ - a);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(offsetX, offsetY + b, offsetZ - a);

    glEnd();
}

void walec(double r, double h, double offsetX, double offsetY, double offsetZ)
{
    const int segments = 32;
    const float angleStep = 2.0f * M_PI / segments;

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++)
    {
        float angle = i * angleStep;
        float x = r * cosf(angle);
        float z = r * sinf(angle);

        glTexCoord2f(i / (float)segments, 0.0f);
        glVertex3f(x + offsetX, offsetY, z + offsetZ);

        glTexCoord2f(i / (float)segments, 1.0f);
        glVertex3f(x + offsetX, offsetY + h, z + offsetZ);
    }
    glEnd();
}

void felga(double r, double h, double offsetX, double offsetY, double offsetZ)
{
    walec(r, h, offsetX, offsetY, offsetZ);
    glBegin(GL_TRIANGLE_FAN);

    glTexCoord2f(0.5f, 0.5f);
    glVertex3f(offsetX, offsetY, offsetZ);

    for (int i = 0; i <= 32; i++)
    {
        float angle = i * 2.0f * M_PI / 32.0f;
        float x = r * cosf(angle);
        float z = r * sinf(angle);

        glTexCoord2f(0.5f + 0.5f * cosf(angle), 0.5f + 0.5f * sinf(angle));
        glVertex3f(x + offsetX, offsetY, z + offsetZ);
    }
    glEnd();
}
