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

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
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
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
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
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
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

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
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

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("flag.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
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



	//генерируем ИД для текстуры
	glGenTextures(1, &texId2);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId2);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW2, texH2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray2);

	//отчистка памяти
	free(texCharArray2);
	free(texarray2);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	RGBTRIPLE* texarray3;
	char* texCharArray3;
	int texW3, texH3;
	OpenGL::LoadBMP("gads.bmp", &texW3, &texH3, &texarray3);
	OpenGL::RGBtoChar(texarray3, texW3, texH3, &texCharArray3);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId3);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId3);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW3, texH3, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray3);

	//отчистка памяти
	free(texCharArray3);
	free(texarray3);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);



	RGBTRIPLE* texarray4;
	char* texCharArray4;
	int texW4, texH4;
	OpenGL::LoadBMP("gosl.bmp", &texW4, &texH4, &texarray4);
	OpenGL::RGBtoChar(texarray4, texW4, texH4, &texCharArray4);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId4);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId4);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW4, texH4, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray4);

	//отчистка памяти
	free(texCharArray4);
	free(texarray4);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

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


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


	//Начало рисования квадратика станкина
		//Начало рисования квадратика станкина
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
	//конец рисования квадратика станкина




	int i;
	double a;
	//низ
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

	//верх
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
	//конец рисования квадратика станкина


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}