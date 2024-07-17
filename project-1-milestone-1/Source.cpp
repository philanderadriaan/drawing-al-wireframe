
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "math.h"
#include <string.h>
char* file_name = new char[];
FILE* input;
unsigned int width, height, size;
float rgb[3] = {1, 1, 1};
float points[6];
float* pixels;
int* graph;
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
int translate(float location, int size)
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
	int x_1 = translate(points[0], width);
	int y_1 = translate(points[1], height);
	int x_2 = translate(points[2], width);
	int y_2 = translate(points[3], height);
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
void setLinePoints()
{
	fscanf(input,"%f", &points[0]);
	fscanf(input,"%f", &points[1]);
	fscanf(input,"%f", &points[2]);
	fscanf(input,"%f", &points[3]);
}
void setTrianglePoints()
{
	setLinePoints();
	fscanf(input,"%f", &points[4]);
	fscanf(input,"%f", &points[5]);
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
	swap(&points[0], &points[4]);
	swap(&points[1], &points[5]);
	drawLine();
	swap(&points[2], &points[4]);
	swap(&points[3], &points[5]);
	drawLine();
}
void readData()
{
	input = fopen(file_name, "r+");
	char text[100];
	do
	{
		fscanf(input,"%s", text);
		if (strcmp (text, "RGB") == 0)
		{
			setColor();
		}
		else if (strcmp (text, "LINE") == 0)
		{
			setLinePoints();
			drawLine();
			displayPixel();
		}
		else if (strcmp (text, "TRI") == 0)
		{
			setTrianglePoints();
			drawTriangle();
			fillTriangle();
			displayPixel();
		}
	}
	while(feof(input) == 0);
	fclose(input);
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
		fillBackground();
		readData();

		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
		glutInitWindowSize(width, height);
		glutCreateWindow("OpenGL glDrawPixels demo - simplified by JM");
		glutDisplayFunc(display);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glutMainLoop();
	}
}