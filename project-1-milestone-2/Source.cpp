
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string.h>
#include <iostream>
using namespace std;
#include "math.h"
#include "mat.h"
#include "vec.h"

char* file_name = new char[];
FILE* input;

unsigned int width, height, size;
float rgb[3] = {1, 1, 1};
float* pixels;
int* graph;

vec4 point[8];

mat4 ctm;
mat4 key_rotate;

void putPixel(int x, int y)
{
	if (0 <= x && x < width && 0 <= y && y < height)
{	
		graph[y * width + x] = 1;
	}
	else
	{
		printf("Pixel out of bounds: %d %d\n", x, y);
	}
}

void resetGraph()
{
	graph = new int[size];
	for(int i = 0; i < size; i++)
	{
		graph[i] = 0;
	}
}

void displayPixel()
{
	for(int i = 0; i < size; i++)
	{
		if(graph[i] == 1)
		{
			pixels[i * 3] = rgb[0];
			pixels[i * 3 + 1] = rgb[1];
			pixels[i * 3 + 2] = rgb[2];
		}
	}
	resetGraph();
}

void transformPoint()
{
	for (int i = 0; i < 8; i++)
	{
		point[i] = key_rotate * ctm * point[i];
	}
}

int getCoordinates(float location, int size)
{
	size = size / 2;
	location = location * size + size;
	if (location > 0)
	{
		location = location - 1;
	}
	return location;
}

void swap(int *a, int *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

void swap(float *a, float *b)
{
	float temp = *a;
	*a = *b;
	*b = temp;
}

void drawLine()
{
	int x_1 = getCoordinates(point[0].x, width);
	int y_1 = getCoordinates(point[0].y, height);
	int x_2 = getCoordinates(point[1].x, width);
	int y_2 = getCoordinates(point[1].y, height);
	int is_steep = abs(y_2 - y_1) > abs(x_2 - x_1);
	if(is_steep)
	{
		swap(&x_1, &y_1);
		swap(&x_2, &y_2);
	}
	if(x_1 > x_2)
	{
		swap(&x_1, &x_2);
		swap(&y_1, &y_2);
	}
	int d_y = abs(y_2 - y_1);
	int d_x = x_2 - x_1;
	int offset = d_x / 2;
	int y = y_1;
	for(int x = x_1; x <= x_2; x++)
	{
		if (is_steep)
		{
			putPixel(y,x);
		}
		else 
		{
			putPixel(x,y);
		}
		offset = offset - d_y;
		if (offset < 0)
		{
			offset = offset + d_x;
			if(y_1 < y_2)
			{
				y++;
			}
			else
			{
				y--;
			}
		}
	}
}

void fillBackground()
{
	rgb[0] = 1;
	rgb[1] = 1;
	rgb[2] = 1;
	for(int x = 0; x < width; x++)
	{
		for(int y = 0; y < height; y++)
		{
			putPixel(x, y);
		}
	}
	displayPixel();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawPixels(width,height,GL_RGB,GL_FLOAT,pixels);
	glutSwapBuffers();
}

void getFile()
{
	printf("Enter the name of the input file (no blanks): ");
	gets(file_name);
	input = fopen(file_name, "r+");
}

void setSize()
{
	input = fopen(file_name, "r+");
	char text[100];
	do
	{
		fscanf(input,"%s", text);
	}
	while(feof(input) == 0 && strcmp (text, "DIM") != 0);
	fscanf(input,"%d", &width);
	fscanf(input,"%d", &height);
	fclose(input);
	size = width * height;
	pixels = new float[size * 3];
	resetGraph();
}

void setColor()
{
	fscanf(input,"%f", &rgb[0]);
	fscanf(input,"%f", &rgb[1]);
	fscanf(input,"%f", &rgb[2]);
}

void setLinepoint()
{
	fscanf(input,"%f", &point[0].x);
	fscanf(input,"%f", &point[0].y);
	fscanf(input,"%f", &point[0].z);
	point[0].w = 1;
	fscanf(input,"%f", &point[1].x);
	fscanf(input,"%f", &point[1].y);
	fscanf(input,"%f", &point[1].z);
	point[1].w = 1;
}

void setTrianglepoint()
{
	setLinepoint();
	fscanf(input,"%f", &point[2].x);
	fscanf(input,"%f", &point[2].y);
	fscanf(input,"%f", &point[2].z);
	point[2].w = 1;
}

void fillTriangle()
{
	for(int y = 0; y < height; y++)
	{
		int min = INT_MAX;
		int max = INT_MIN;
		for(int x = 0; x < width; x++)
		{
			if(graph[y * width + x] == 1)
			{
				if(x < min)
				{
					min = x;
				}
				if(x > max)
				{
					max = x;
				}
			}
		}
		for(int x = min; x < max; x++)
		{
			if (0 <= x && x < width)
			{
				putPixel(x, y);
			}
		}
	}
}

void drawTriangle()
{
	drawLine();
	swap(&point[0].x, &point[2].x);
	swap(&point[0].y, &point[2].y);
	drawLine();
	swap(&point[1].x, &point[2].x);
	swap(&point[1].y, &point[2].y);
	drawLine();
}

void setScale()
{
	float scale[3];
	fscanf(input,"%f", &scale[0]);
	fscanf(input,"%f", &scale[1]);
	fscanf(input,"%f", &scale[2]);
	ctm = Scale(scale[0], scale[1], scale[2]) * ctm;
}

void setTranslate()
{
	float translate[3];
	fscanf(input,"%f", &translate[0]);
	fscanf(input,"%f", &translate[1]);
	fscanf(input,"%f", &translate[2]);
	ctm = Translate(translate[0], translate[1], translate[2]) * ctm;
}

void setUnitCube()
{
	vec4 corner_0(-0.5, -0.5, -0.5, 1);
	point[0] = corner_0;
	vec4 corner_1(-0.5, -0.5, 0.5, 1);
	point[1] = corner_1;
	vec4 corner_2(-0.5, 0.5, -0.5, 1);
	point[2] = corner_2;
	vec4 corner_3(-0.5, 0.5, 0.5, 1);
	point[3] = corner_3;
	vec4 corner_4(0.5, -0.5, -0.5, 1);
	point[4] = corner_4;
	vec4 corner_5(0.5, -0.5, 0.5, 1);
	point[5] = corner_5;
	vec4 corner_6(0.5, 0.5, -0.5, 1);
	point[6] = corner_6;
	vec4 corner_7(0.5, 0.5, 0.5, 1);
	point[7] = corner_7;
}

void drawCube()
{
	vec4 clone[8];
	for (int i = 0; i < 8; i++)
	{
		clone[i] = point[i];
	}
	drawLine();
	point[1] = clone[2];
	drawLine();
	point[1] = clone[4];
	drawLine();
	point[0] = clone[1];
	point[1] = clone[3];
	drawLine();
	point[1] = clone[5];
	drawLine();
	point[0] = clone[2];
	point[1] = clone[3];
	drawLine();
	point[1] = clone[6];
	drawLine();
	point[0] = clone[3];
	point[1] = clone[7];
	drawLine();
	point[0] = clone[4];
	point[1] = clone[5];
	drawLine();
	point[1] = clone[6];
	drawLine();
	point[0] = clone[5];
	point[1] = clone[7];
	drawLine();
	point[0] = clone[6];
	point[1] = clone[7];
	drawLine();
}

void resetCTM()
{
	mat4 identity;
	ctm = identity;
}

void readData()
{
	resetGraph();
	fillBackground();
	input = fopen(file_name, "r+");
	char text[100];
	do
	{
		fscanf(input,"%s", text);
		if (strcmp (text, "LOAD_IDENTITY_MATRIX") == 0)
		{
			resetCTM();
		}
		else if (strcmp (text, "TRANSLATE") == 0)
		{
			setTranslate();
		}
		else if (strcmp (text, "ROTATEX") == 0)
		{
			int rotate;
			fscanf(input,"%d", &rotate);
			ctm = RotateX(rotate) * ctm;
		}
		else if (strcmp (text, "ROTATEY") == 0)
		{
			int rotate;
			fscanf(input,"%d", &rotate);
			ctm = RotateY(rotate) * ctm;
		}
		else if (strcmp (text, "ROTATEZ") == 0)
		{
			int rotate;
			fscanf(input,"%d", &rotate);
			ctm = RotateZ(rotate) * ctm;
		}
		else if (strcmp (text, "SCALE") == 0)
		{
			setScale();
		}
		else if (strcmp (text, "WIREFRAME_CUBE") == 0)
		{
			setUnitCube();
			transformPoint();
			drawCube();
			displayPixel();
		}
		else if (strcmp (text, "LINE") == 0)
		{
			setLinepoint();
			transformPoint();
			drawLine();
			displayPixel();
		}
		else if (strcmp (text, "TRI") == 0)
		{
			setTrianglepoint();
			transformPoint();
			drawTriangle();
			fillTriangle();
			displayPixel();
		}
		else if (strcmp (text, "RGB") == 0)
		{
			setColor();
		}
	}
	while(feof(input) == 0);
	fclose(input);
}

void keyboard(int input, int a, int b)
{
	switch(input)
	{
	case GLUT_KEY_UP:
		key_rotate *= RotateX(-1);
		break;
	case GLUT_KEY_DOWN:
		key_rotate *= RotateX(1);
		break;
	case GLUT_KEY_LEFT:
		key_rotate *= RotateY(-1);
		break;
	case GLUT_KEY_RIGHT:
		key_rotate *= RotateY(1);
		break;
	}
	readData();
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	getFile();
	if (input == NULL)
	{
		perror ("Error opening file");
		getchar();
	}
	else
	{
		fclose(input);
		setSize();
		readData();

		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
		glutInitWindowSize(width, height);
		glutCreateWindow("OpenGL glDrawPixels demo - simplified by JM");
		glutDisplayFunc(display);
		glutSpecialFunc(keyboard);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glutMainLoop();

	}
}