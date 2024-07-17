
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string.h>
#include <iostream>
using namespace std;
#include "math.h"
#include "mat.h"
#include "vec.h"

char data[300][30];
int word_count = 0;

unsigned int width;
unsigned int height;
unsigned int size;
int* graph;


float rgb[3];
float shaded[3];
float projection_data[6];
float* pixels;
float* depth;
float* previous_depth;

int is_frustum;
int is_first = 1;
int is_cube = 1;

vec4 scan_points[2];
vec4 line_points[2];
vec4 triangle_points[3];
vec4 cube_points[8];

vec4 light_direction;

mat4 camera;
mat4 projection;
mat4 ctm;
mat4 key_rotate;

float intensity = 1;

void putPixel(int x, int y, float z)
{
	if (0 <= x && x < width && 0 <= y)
	{
		graph[y * width + x] = 1;
		depth[y * width + x] = z;
	}
	else
	{
		printf("Pixel out of bounds: %d %d\n", x, y);
	}
}

void resetGraph()
{
	delete[] graph;
	graph = new int[size];
	delete[] depth;
	depth = new float[size];
	for (int i = 0; i < size; i++)
	{
		depth[i] = -3;
	}
}

void displayPixel()
{
	for(int i = 0; i < size; i++)
	{
		if(graph[i] == 1 && depth[i] >= previous_depth[i])
		{
			for (int j = 0; j < 3; j++)
			{
				pixels[i * 3 + j] = shaded[j];
			}
			previous_depth[i] = depth[i];
		}
	}
	resetGraph();
}

void transform()
{
	if (is_frustum == 1)
	{
		projection = Frustum(projection_data[0], projection_data[1], projection_data[2], projection_data[3], projection_data[4], projection_data[5]);
	}
	else
	{
		projection = Ortho(projection_data[0], projection_data[1], projection_data[2], projection_data[3], projection_data[4], projection_data[5]);
	}
	for (int i = 0; i < 2; i++)
	{
		line_points[i] =  projection * camera * key_rotate * ctm * line_points[i];
		line_points[i] /= line_points[i].w;
	}
	for (int i = 0; i < 3; i++)
	{
		triangle_points[i] =  projection * camera * key_rotate * ctm * triangle_points[i];
		triangle_points[i] /= triangle_points[i].w;
	}
	for (int i = 0; i < 8; i++)
	{
		cube_points[i] =  projection * camera * key_rotate * ctm * cube_points[i];
		cube_points[i] /= cube_points[i].w;
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

float getDepth(float w_1, float z_1, float w_2, float z_2, float w)
{
	return (((w - w_1)  * (z_2 - z_1))/ (w_2 - w_1)) + z_1;
}

void drawLine()
{
	int x_1 = getCoordinates(line_points[0].x, width);
	int y_1 = getCoordinates(line_points[0].y, height);
	float z_1 = line_points[0].z;
	int x_2 = getCoordinates(line_points[1].x, width);
	int y_2 = getCoordinates(line_points[1].y, height);
	float z_2 = line_points[1].z;
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
		swap(&z_1, &z_2);
	}
	int d_y = abs(y_2 - y_1);
	int d_x = x_2 - x_1;
	int offset = d_x / 2;
	int y = y_1;
	for(int x = x_1; x <= x_2; x++)
	{
		if (is_steep)
		{
			putPixel(y,x, getDepth(x_1, z_1, x_2, z_2, x));
		}
		else 
		{
			putPixel(x,y, getDepth(x_1, z_1, x_2, z_2, x));
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


void scan()
{
	int x_1 = scan_points[0].x;
	int y_1 = scan_points[0].y;
	float z_1 = scan_points[0].z;
	int x_2 = scan_points[1].x;
	int y_2 = scan_points[1].y;
	float z_2 = scan_points[1].z;
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
		swap(&z_1, &z_2);
	}
	int d_y = abs(y_2 - y_1);
	int d_x = x_2 - x_1;
	int offset = d_x / 2;
	int y = y_1;
	for(int x = x_1; x <= x_2; x++)
	{
		if (is_steep)
		{
			putPixel(y,x, getDepth(x_1, z_1, x_2, z_2, x));
		}
		else 
		{
			putPixel(x,y, getDepth(x_1, z_1, x_2, z_2, x));
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

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawPixels(width,height,GL_RGB,GL_FLOAT,pixels);
	glutSwapBuffers();
}

void fillTriangle()
{
	for(int y = 0; y < height; y++)
	{
		int min = INT_MAX;
		int max = INT_MIN;
		int min_depth = -3;
		int max_depth = -3;
		for(int x = 0; x < width; x++)
		{
			if(graph[y * width + x] == 1)
			{
				if(x < min)
				{
					min = x;
					min_depth = depth[y * width + min];
				}
				if(x > max)
				{
					max = x;
					max_depth = depth[y * width + max];
				}
			}
		}
		if (0 <= min && 0 <= max && min < width && max < width)
		{
			scan_points[0] = vec4(min, y, min_depth, 0);
			scan_points[1] = vec4(max, y, max_depth, 0);
			scan();
		}
	}
	for (int i = 0; i < 3; i++)
	{
		shaded[i] = rgb[i] * dot(normalize(light_direction), normalize(cross(triangle_points[1] - triangle_points[0], triangle_points[2] - triangle_points[0])));
	}
}

void drawTriangle()
{
	line_points[0] = triangle_points[0];
	line_points[1] = triangle_points[1];
	drawLine();
	line_points[0] = triangle_points[1];
	line_points[1] = triangle_points[2];
	drawLine();
	line_points[0] = triangle_points[2];
	line_points[1] = triangle_points[0];
	drawLine();
}

void unitCube()
{
	vec4 corner_0(-0.5, -0.5, -0.5, 1);
	cube_points[0] = corner_0;
	vec4 corner_1(-0.5, -0.5, 0.5, 1);
	cube_points[1] = corner_1;
	vec4 corner_2(-0.5, 0.5, -0.5, 1);
	cube_points[2] = corner_2;
	vec4 corner_3(-0.5, 0.5, 0.5, 1);
	cube_points[3] = corner_3;
	vec4 corner_4(0.5, -0.5, -0.5, 1);
	cube_points[4] = corner_4;
	vec4 corner_5(0.5, -0.5, 0.5, 1);
	cube_points[5] = corner_5;
	vec4 corner_6(0.5, 0.5, -0.5, 1);
	cube_points[6] = corner_6;
	vec4 corner_7(0.5, 0.5, 0.5, 1);
	cube_points[7] = corner_7;
}

void readData()
{
	for (int i = 0; i < word_count; i++)
	{
		if (strcmp (data[i], "LIGHT_DIRECTION") == 0)
		{
			light_direction.x = atof(data[i + 1]);
			light_direction.y = atof(data[i + 2]);
			light_direction.z = atof(data[i + 3]);
			light_direction.w = 0;
		}
		else if (strcmp (data[i], "FRUSTUM") == 0)
		{
			if(is_first == 1)
			{
				for (int j = 0; j < 6; j++)
				{
					projection_data[j] = atof(data[i + j + 1]);
				}
				is_frustum = 1;
				is_first = 0;
			}
		}
		else if (strcmp (data[i], "ORTHO") == 0)
		{
			if(is_first == 1)
			{
				for (int j = 0; j < 6; j++)
				{
					projection_data[j] = atof(data[i + j + 1]);
				}
				is_frustum = 0;
				is_first = 0;
			}
		}
		else if (strcmp (data[i], "LOOKAT") == 0)
		{
			vec4 look[3];
			for (int j = 0; j < 3; j++)
			{
				look[j].x = atof(data[i + j * 3 + 1]);
				look[j].y = atof(data[i + j * 3 + 2]);
				look[j].z = atof(data[i + j * 3 + 3]);
			}
			camera = LookAt(look[0], look[1], look[2]);
		}
		else if (strcmp (data[i], "SOLID_CUBE") == 0)
		{
			is_cube = 1;
			unitCube();
			transform();
			triangle_points[0] = cube_points[0];
			triangle_points[1] = cube_points[3];
			triangle_points[2] = cube_points[1];
			drawTriangle();
			fillTriangle();
			displayPixel();
			triangle_points[0] = cube_points[0];
			triangle_points[1] = cube_points[2];
			triangle_points[2] = cube_points[3];
			drawTriangle();
			fillTriangle();
			displayPixel();
			triangle_points[0] = cube_points[4];
			triangle_points[1] = cube_points[7];
			triangle_points[2] = cube_points[5];
			drawTriangle();
			fillTriangle();
			displayPixel();
			triangle_points[0] = cube_points[4];
			triangle_points[1] = cube_points[6];
			triangle_points[2] = cube_points[7];
			drawTriangle();
			fillTriangle();
			displayPixel();
			triangle_points[0] = cube_points[0];
			triangle_points[1] = cube_points[5];
			triangle_points[2] = cube_points[1];
			drawTriangle();
			fillTriangle();
			displayPixel();
			triangle_points[0] = cube_points[0];
			triangle_points[1] = cube_points[4];
			triangle_points[2] = cube_points[5];
			drawTriangle();
			fillTriangle();
			displayPixel();
			triangle_points[0] = cube_points[2];
			triangle_points[1] = cube_points[7];
			triangle_points[2] = cube_points[3];
			drawTriangle();
			fillTriangle();
			displayPixel();
			triangle_points[0] = cube_points[2];
			triangle_points[1] = cube_points[6];
			triangle_points[2] = cube_points[7];
			drawTriangle();
			fillTriangle();
			displayPixel();
			triangle_points[0] = cube_points[0];
			triangle_points[1] = cube_points[6];
			triangle_points[2] = cube_points[2];
			drawTriangle();
			fillTriangle();
			displayPixel();
			triangle_points[0] = cube_points[0];
			triangle_points[1] = cube_points[4];
			triangle_points[2] = cube_points[6];
			drawTriangle();
			fillTriangle();
			displayPixel();
			triangle_points[0] = cube_points[1];
			triangle_points[1] = cube_points[7];
			triangle_points[2] = cube_points[3];
			drawTriangle();
			fillTriangle();
			displayPixel();
			triangle_points[0] = cube_points[1];
			triangle_points[1] = cube_points[5];
			triangle_points[2] = cube_points[7];
			drawTriangle();
			fillTriangle();
			displayPixel();
		}
		else if (strcmp (data[i], "LOAD_IDENTITY_MATRIX") == 0)
		{
			mat4 identity;
			ctm = identity;
		}
		else if (strcmp (data[i], "TRANSLATE") == 0)
		{
			float translate[3];
			for (int j = 0; j < 3; j++)
			{
				translate[j] = atof(data[i + j + 1]);
			}
			ctm = Translate(translate[0], translate[1], translate[2]) * ctm;
		}
		else if (strcmp (data[i], "ROTATEX") == 0)
		{
			int rotate;
			rotate = atoi(data[i + 1]);
			ctm = RotateX(rotate) * ctm;
		}
		else if (strcmp (data[i], "ROTATEY") == 0)
		{
			int rotate;
			rotate = atoi(data[i + 1]);
			ctm = RotateY(rotate) * ctm;
		}
		else if (strcmp (data[i], "ROTATEZ") == 0)
		{
			int rotate;
			rotate = atoi(data[i + 1]);
			ctm = RotateZ(rotate) * ctm;
		}
		else if (strcmp (data[i], "SCALE") == 0)
		{
			float scale[3];
			for (int j = 0; j < 3; j++)
			{
				scale[j] = atof(data[i + j + 1]);
			}
			ctm = Scale(scale[0], scale[1], scale[2]) * ctm;
		}
		else if (strcmp (data[i], "WIREFRAME_CUBE") == 0)
		{
			is_cube = 1;
			unitCube();
			transform();
			line_points[0] = cube_points[0];
			line_points[1] = cube_points[1];
			drawLine();
			line_points[0] = cube_points[0];
			line_points[1] = cube_points[2];
			drawLine();
			line_points[0] = cube_points[0];
			line_points[1] = cube_points[4];
			drawLine();
			line_points[0] = cube_points[1];
			line_points[1] = cube_points[3];
			drawLine();
			line_points[0] = cube_points[1];
			line_points[1] = cube_points[5];
			drawLine();
			line_points[0] = cube_points[2];
			line_points[1] = cube_points[3];
			drawLine();
			line_points[0] = cube_points[2];
			line_points[1] = cube_points[6];
			drawLine();
			line_points[0] = cube_points[3];
			line_points[1] = cube_points[7];
			drawLine();
			line_points[0] = cube_points[4];
			line_points[1] = cube_points[5];
			drawLine();
			line_points[0] = cube_points[4];
			line_points[1] = cube_points[6];
			drawLine();
			line_points[0] = cube_points[5];
			line_points[1] = cube_points[7];
			drawLine();
			line_points[0] = cube_points[6];
			line_points[1] = cube_points[7];
			drawLine();
			displayPixel();
		}
		else if (strcmp (data[i], "LINE") == 0)
		{
			is_cube = 0;
			for (int j = 0; j < 2; j++)
			{
				line_points[j].x = atof(data[i + j * 3 + 1]);
				line_points[j].y = atof(data[i + j * 3 + 2]);
				line_points[j].z = atof(data[i + j * 3 + 3]);
			}
			transform();
			drawLine();
			displayPixel();
		}
		else if (strcmp (data[i], "TRI") == 0)
		{
			is_cube = 0;
			for (int j = 0; j < 3; j++)
			{
				triangle_points[j].x = atof(data[i + j * 3 + 1]);
				triangle_points[j].y = atof(data[i + j * 3 + 2]);
				triangle_points[j].z = atof(data[i + j * 3 + 3]);
			}
			transform();
			drawTriangle();
			fillTriangle();
			displayPixel();
		}
		else if (strcmp(data[i], "RGB") == 0)
		{
			for (int j = 0; j < 3; j++)
			{
				rgb[j] = atof(data[i + j + 1]);
			}
		}
		else if (strcmp(data[i], "DIM") == 0)
		{
			width = atoi(data[i + 1]);
			height = atoi(data[i + 2]);
			size = width * height;
			resetGraph();
			delete[] previous_depth;
			previous_depth = new float[size];
			for (int i = 0; i < size; i++)
			{
				previous_depth[i] = -3;
			}
			delete[] pixels;
			pixels = new float[size * 3];
			shaded[0] = 1;
			shaded[1] = 1;
			shaded[2] = 1;
			for(int x = 0; x < width; x++)
			{
				for(int y = 0; y < height; y++)
				{
					putPixel(x, y, -2);
				}
			}
			displayPixel();
			resetGraph();
		}
	}
}

void keyboard(int key, int a, int b)
{
	switch(key)
	{
	case GLUT_KEY_UP:
		key_rotate *= RotateX(1);
		break;
	case GLUT_KEY_DOWN:
		key_rotate *= RotateX(-1);
		break;
	case GLUT_KEY_LEFT:
		key_rotate *= RotateY(1);
		break;
	case GLUT_KEY_RIGHT:
		key_rotate *= RotateY(-1);
		break;
	}
	glutPostRedisplay();
	readData();
}

void keyboard2(unsigned char key, int a, int b)
{
	switch(key)
	{
	case 'o':
		is_frustum = 0;
		break;
	case 'p':
		is_frustum = 1;
		break;
	case 'l':
		projection_data[0] *= 1.1;
		break;
	case 'r':
		projection_data[1] *= 1.1;
		break;
	case 'b':
		projection_data[2] *= 1.1;
		break;
	case 't':
		projection_data[3] *= 1.1;
		break;
	case 'n':
		projection_data[4] *= 1.1;
		break;
	case 'f':
		projection_data[5] *= 1.1;
		break;
	}
	glutPostRedisplay();
	readData();
}

int main(int argc, char** argv)
{
	printf("Enter the name of the input file (no blanks): ");
	char* file_name = new char[];
	gets(file_name);
	FILE* input = fopen(file_name, "r+");
	if (input == NULL)
	{
		perror ("Error opening file");
		getchar();
	}
	else
	{
		do
		{
			fscanf(input,"%s", data[word_count]);
			word_count++;
		}
		while(feof(input) == 0);
		readData();
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
		glutInitWindowSize(width, height);
		glutCreateWindow("OpenGL glDrawPixels demo - simplified by JM");
		glutDisplayFunc(display);
		glutKeyboardFunc(keyboard2);
		glutSpecialFunc(keyboard);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glutMainLoop();
	}
}