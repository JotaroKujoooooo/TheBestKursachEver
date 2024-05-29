#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;

	
	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����




//������ ���������� ����
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//������� ���� �� ���������, � ����� ��� ����
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId; 
GLuint texId2;
GLuint texId3;
GLuint texId4;

//����������� ����� ������ ��������
void initRender(OpenGL *ogl)
{
	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);
	

	//������ ����������� ���������  (R G B)
	RGBTRIPLE *texarray;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("flag.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//���������� �� ��� ��������
	glGenTextures(1, &texId);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	RGBTRIPLE* texarray2;
	char* texCharArray2;
	int texW2, texH2;
	OpenGL::LoadBMP("max.bmp", &texW2, &texH2, &texarray2);
	OpenGL::RGBtoChar(texarray2, texW2, texH2, &texCharArray2);



	//���������� �� ��� ��������
	glGenTextures(1, &texId2);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId2);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW2, texH2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray2);

	//�������� ������
	free(texCharArray2);
	free(texarray2);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	RGBTRIPLE* texarray3;
	char* texCharArray3;
	int texW3, texH3;
	OpenGL::LoadBMP("gads.bmp", &texW3, &texH3, &texarray3);
	OpenGL::RGBtoChar(texarray3, texW3, texH3, &texCharArray3);



	//���������� �� ��� ��������
	glGenTextures(1, &texId3);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId3);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW3, texH3, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray3);

	//�������� ������
	free(texCharArray3);
	free(texarray3);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);



	RGBTRIPLE* texarray4;
	char* texCharArray4;
	int texW4, texH4;
	OpenGL::LoadBMP("gosl.bmp", &texW4, &texH4, &texarray4);
	OpenGL::RGBtoChar(texarray4, texW4, texH4, &texCharArray4);



	//���������� �� ��� ��������
	glGenTextures(1, &texId4);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId4);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW4, texH4, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray4);

	//�������� ������
	free(texCharArray4);
	free(texarray4);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH); 


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}



double getNx(double* A, double* B, double* C)
{
	return -((B[1] - A[1]) * (C[2] - B[2]) - (B[2] - A[2]) * (C[1] - B[1]));
}
double getNy(double* A, double* B, double* C)
{
	return -(-(B[0] - A[0]) * (C[2] - B[2]) + (B[2] - A[2]) * (C[0] - B[0]));
}
double getNz(double* A, double* B, double* C)
{
	return ((B[0] - A[0]) * (C[1] - B[1]) - (B[1] - A[1]) * (C[0] - B[0]));
}

void norma(double v1[], double v2[], double v11[], double& norm_x, double& norm_y, double& norm_z)
{
	double dlin;
	double ax, ay, az, bx, by, bz;
	ax = v2[0] - v1[0];
	ay = v2[1] - v1[1];
	az = v2[2] - v1[2];
	bx = v11[0] - v1[0];
	by = v11[1] - v1[1];
	bz = v11[2] - v1[2];
	norm_x = (ay * bz - by * az);
	norm_y = (bx * az - ax * bz);
	norm_z = (ax * by - bx * ay);
	dlin = sqrt(pow(norm_x, 2) + pow(norm_y, 2) + pow(norm_z, 2));
	norm_x /= dlin;
	norm_y /= dlin;
	norm_z /= dlin;
	//norm_x = -norm_x;
	//norm_y = -norm_y;
	//norm_z = -norm_z;
}

void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//��������������
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//��������� ���������
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//������ �����
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//���� ���� �������, ��� ����������� (����������� ���������)
	glShadeModel(GL_SMOOTH);
	//===================================
	//������� ���  


	//������ ��������� ���������� ��������
		//������ ��������� ���������� ��������
	double A[2] = { -4, -4 };
	double B[2] = { 4, -4 };
	double C[2] = { 4, 4 };
	double D[2] = { -4, 4 };

	//glBindTexture(GL_TEXTURE_2D, texId);

	//glColor3d(0.6, 0.6, 0.6);
	//glBegin(GL_QUADS);

	//glNormal3d(0, 0, 1);
	//glTexCoord2d(0, 0);
	//glVertex2dv(A);
	//glTexCoord2d(1, 0);
	//glVertex2dv(B);
	//glTexCoord2d(1, 1);
	//glVertex2dv(C);
	//glTexCoord2d(0, 1);
	//glVertex2dv(D);

	//glEnd();
	//����� ��������� ���������� ��������




	int i;
	double a;
	//���
	double norm[] = { 0,0,0 };
	double z0 = 0;
	double z1 = 10;
	double v1[] = { 0,0,z0 };
	double v2[] = { 4,1,z0 };
	double v3[] = { 4,5,z0 };
	double v4[] = { -1, 2, z0 };
	double v5[] = { -4, 5, z0 };
	double v6[] = { -2,1,z0 };
	double v7[] = { -5,-2,z0 };
	double v8[] = { -2,-7,z0 };

	//����
	double v11[] = { 0,0,z1 };
	double v21[] = { 4,1,z1 };
	double v31[] = { 4,5,z1 };
	double v41[] = { -1, 2, z1 };
	double v51[] = { -4, 5, z1 };
	double v61[] = { -2,1,z1 };
	double v71[] = { -5,-2,z1 };
	double v81[] = { -2,-7,z1 };



	glBegin(GL_POLYGON);
	glNormal3d(0, 0, -1);
	glColor3d(0.2, 0.7, 0.7);
	glVertex3dv(v1);
	glVertex3dv(v2);
	for (i = 59; i < 100; i++)
	{
		a = (double)i / 39.6 * 3.1415;
		glVertex3d(cos(a) * 2 + 4, sin(a) * 2 + 3, z0);
	}
	glVertex3dv(v3);
	glVertex3dv(v4);
	glVertex3dv(v5);
	glVertex3dv(v6);
	glVertex3dv(v7);
	glVertex3dv(v8);
	glEnd();



	glBegin(GL_QUADS);
	glColor3d(0.2, 0.5, 0.7);
	for (i = 59; i < 100; i++)
	{
		a = (double)i / 39.6 * 3.1415;
		double vv1[] = { cos(a) * 2 + 4, sin(a) * 2 + 3, z0 };
		if (i != 99)
		{
			a = (double)(i + 1) / 39.6 * 3.1415;
		}
		double vv2[] = { cos(a) * 2 + 4 ,sin(a) * 2 + 3, z0 };
		a = (double)i / 39.6 * 3.1415;
		double vv11[] = { cos(a) * 2 + 4, sin(a) * 2 + 3, z1 };
		norma(vv1, vv2, vv11, norm[0], norm[1], norm[2]);
		glNormal3dv(norm);
		glVertex3d(cos(a) * 2 + 4, sin(a) * 2 + 3, z0);
		glVertex3d(cos(a) * 2 + 4, sin(a) * 2 + 3, z1);
		if (i != 99)
		{
			a = (double)(i + 1) / 39.6 * 3.1415;
			glVertex3d(cos(a) * 2 + 4, sin(a) * 2 + 3, z1);
			glVertex3d(cos(a) * 2 + 4, sin(a) * 2 + 3, z0);
		}
		else
		{
			a = (double)50 / 39.6 * 3.1415;
		}

	}
	glEnd();



	//glEnd();


	glBegin(GL_QUADS);
	norma(v1, v2, v11, norm[0], norm[1], norm[2]);
	glNormal3dv(norm);
	glColor3d(0.2, 0.5, 0.7);
	glVertex3dv(v1);
	glVertex3d(v1[0], v1[1], z1);
	glVertex3d(v2[0], v2[1], z1);
	glVertex3dv(v2);
	glEnd();

	glBegin(GL_QUADS);
	norma(v3, v4, v31, norm[0], norm[1], norm[2]);
	glNormal3dv(norm);
	glColor3d(0.2, 0.5, 0.7);
	glVertex3dv(v3);
	glVertex3d(v3[0], v3[1], z1);
	glVertex3d(v4[0], v4[1], z1);
	glVertex3dv(v4);
	glEnd();




	glBegin(GL_QUADS);
	norma(v6, v7, v61, norm[0], norm[1], norm[2]);
	glNormal3dv(norm);
	glColor3d(0.2, 0.5, 0.7);
	glVertex3dv(v6);
	glVertex3d(v6[0], v6[1], z1);
	glVertex3d(v7[0], v7[1], z1);
	glVertex3dv(v7);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texId);

	glColor3d(0.2, 0.7, 0.5);
	glBegin(GL_POLYGON);

	glNormal3d(0, 0, 1);
	glTexCoord2d((0 + 5), (0 + 7));
	//glTexCoord2d(0, 0)
	glVertex3dv(v11);
	glTexCoord2d((4 + 5), (1 + 7));
	//glTexCoord2d(1, 0)
	glVertex3dv(v21);
	glTexCoord2d((4 + 5), (5 + 7));
	//glTexCoord2d(1, 1);
	glVertex3dv(v31);
	glTexCoord2d((-1 + 5), (2 + 7));
	//glTexCoord2d(0, 1);
	glVertex3dv(v41);
	glTexCoord2d((-4 + 5), (5 + 7));
	glVertex3dv(v51);
	glTexCoord2d((-2 + 5), (1 + 7));
	glVertex3dv(v61);
	glTexCoord2d((-5 + 5), (-2 + 7));
	glVertex3dv(v71);
	glTexCoord2d((-2 + 5), (-7 + 7));
	glVertex3dv(v81);

	glEnd();
	glBegin(GL_POLYGON);
	glColor3d(0.2, 0.7, 0.5);
	glNormal3d(0, 0, 1);
	for (i = 59; i < 100; i++)
	{
		a = (double)i / 39.6 * 3.1415;
		glTexCoord2d((cos(a) * 2 + 4), (sin(a) * 2 + 3));
		glVertex3d(cos(a) * 2 + 4, sin(a) * 2 + 3, z1);
	}
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texId2);

	glBegin(GL_QUADS);
	norma(v7, v8, v71, norm[0], norm[1], norm[2]);
	glNormal3dv(norm);
	glColor3d(0.2, 0.5, 0.7);
	glTexCoord2d(1, 0);
	glVertex3dv(v7);
	glTexCoord2d(1, 1);
	glVertex3d(v7[0], v7[1], z1);
	glTexCoord2d(0,1);
	glVertex3d(v8[0], v8[1], z1);
	glTexCoord2d(0, 0);
	glVertex3dv(v8);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texId3);

	glBegin(GL_QUADS);
	norma(v8, v1, v81, norm[0], norm[1], norm[2]);
	glNormal3dv(norm);
	glColor3d(0.2, 0.5, 0.7);
	glTexCoord2d(0,0);
	glVertex3dv(v8);
	glTexCoord2d(0,1);
	glVertex3d(v8[0], v8[1], z1);
	glTexCoord2d(1,1);
	glVertex3d(v1[0], v1[1], z1);
	glTexCoord2d(1,0);
	glVertex3dv(v1);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texId4);

	glBegin(GL_QUADS);
	norma(v4, v5, v41, norm[0], norm[1], norm[2]);
	glNormal3dv(norm);
	glColor3d(0.2, 0.5, 0.7);
	glTexCoord2d(0, 0);
	glVertex3dv(v4);
	glTexCoord2d(0, 1);
	glVertex3d(v4[0], v4[1], z1);
	glTexCoord2d(1, 1);
	glVertex3d(v5[0], v5[1], z1);
	glTexCoord2d(1, 0);
	glVertex3dv(v5);
	glEnd();


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_QUADS);
	norma(v5, v6, v51, norm[0], norm[1], norm[2]);
	glNormal3dv(norm);
	glColor4d(0.2, 0.5, 0.7, 0.9);
	glVertex3dv(v5);
	glVertex3d(v5[0], v5[1], z1);
	glVertex3d(v6[0], v6[1], z1);
	glVertex3dv(v6);
	glEnd();

	glDisable(GL_BLEND);
	//����� ��������� ���������� ��������


   //��������� ������ ������

	
	glMatrixMode(GL_PROJECTION);	//������ �������� ������� ��������. 
	                                //(���� ��������� ��������, ����� �� ������������.)
	glPushMatrix();   //��������� ������� ������� ������������� (������� ��������� ������������� ��������) � ���� 				    
	glLoadIdentity();	  //��������� ��������� �������
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //������� ����� ������������� ��������

	glMatrixMode(GL_MODELVIEW);		//������������� �� �����-��� �������
	glPushMatrix();			  //��������� ������� ������� � ���� (��������� ������, ����������)
	glLoadIdentity();		  //���������� �� � ������

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //������� ����� ��������� ��� ������� ������ � �������� ������.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - ���/���� �������" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "�����. �����: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "��������� ������: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //��������������� ������� �������� � �����-��� �������� �� �����.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}