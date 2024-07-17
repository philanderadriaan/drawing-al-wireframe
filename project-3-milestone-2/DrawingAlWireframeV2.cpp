

#include "Angel.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define MAX_VERT 21372
vec4 data[MAX_VERT];
vec4 colors[MAX_VERT];
int dataSize = 0;  // how many vertices

mat4 CTM;

// shader variable locations
GLuint programID;
GLuint bufferID;
GLuint vPositionLoc;
GLuint vColorLoc;
GLuint ctmID;

float yRot = 0.0;

vec4 original_data[MAX_VERT];
point4 unshaded_colors[MAX_VERT];
int triangle_indexes[MAX_VERT];
vec4 vertices[3619];

#define NONE -1
#define LEFT_FOOT 0
#define RIGHT_FOOT 1
#define LEFT_LOWER_LEG 2
#define RIGHT_LOWER_LEG 3
#define LEFT_UPPER_LEG 4
#define RIGHT_UPPER_LEG 5
#define LEFT_LOWER_ARM 6
#define RIGHT_LOWER_ARM 7
#define LEFT_UPPER_ARM 8
#define RIGHT_UPPER_ARM 9
#define TORSO 10
#define NUMBER_OF_GROUPS 11

int moving_group = NONE;

vec4 group_origins[NUMBER_OF_GROUPS];
vec4 group_inverses[NUMBER_OF_GROUPS];
mat4 group_transformations[NUMBER_OF_GROUPS];
int group_indexes[NUMBER_OF_GROUPS][MAX_VERT];
int group_count[NUMBER_OF_GROUPS];
int group_dependencies[NUMBER_OF_GROUPS];

int parseVertString(char * verticesString, int v[])
{
	char token[20];
	int vCount = 0;
	int t = 0;
	for (int s = 0; verticesString[s] != '\n'; s++)
	{
		if (verticesString[s] != ' ')
		{
			token[t++] = verticesString[s];
		}
		else if (t > 0)
		{
			token[t++] = 0;
			v[vCount++] = atoi(token);
			t = 0;
		}
	}

	if (t > 0)
	{
		token[t++] = 0;
		v[vCount++] = atoi(token);
	}
	return vCount;
}

void readFile()
{
	char fileName[50];
	char str1[300];
	char str2[300];
	FILE *input;
	float x, y, z;
	float r, g, b;

	int numVertices = 1;   // .obj input file numbers the first vertex starting at index 1

	int facetCount = 0;

	char mtl_data[400][100];
	int mtl_count = 0;

	printf("Enter the name of the input file (no blanks): ");
	gets(fileName);

	input = fopen(fileName, "r+");
	if (input == NULL)
	{
		perror ("Error opening file");
	}
	else
	{
		int current_group;
		while(feof(input)== 0)
		{
			fscanf(input,"%s",str1);
			if (strcmp(str1,"v")==0)
			{  // lines starting with v  =  a vertex 
				fscanf(input,"%f %f %f", &x, &y, &z);
				vertices[numVertices++] = vec4(x,y,z,1);
			}
			else if (strcmp(str1,"f")==0)
			{  // lines starting with f = a facet
				facetCount++;
				// Code upgrade
				// The number of vertices that follow "f" is variable
				// so here is some code to gather all the integers into an array
				char verticesString[200];
				fgets(verticesString, 200, input);
				int v[50];
				int vCount = parseVertString(verticesString,v);
				for (int i = 0; i < vCount - 2; i++)
				{
					int v1 = v[0];
					int v2 = v[i + 1];
					int v3 = v[i + 2];
					if (dataSize+3 > MAX_VERT)
					{
						printf("Arrays are not big enough!\n");
						system("PAUSE");
						exit(1);
					}
					for (int j = 0; j < 3; j++)
					{
						group_indexes[current_group][group_count[current_group]] = dataSize + j;
						group_count[current_group]++;
					}
					// filling in 3 vertices to form a triangle
					// also 3 colors, one for each vertex
					// corresponding colors and vertices are at the same index in each array 
					unshaded_colors[dataSize] = color4(r,g,b,1);
					triangle_indexes[dataSize] = v1;
					data[dataSize++] = vertices[v1];
					unshaded_colors[dataSize] = color4(r,g,b,1);
					triangle_indexes[dataSize] = v2;
					data[dataSize++] = vertices[v2];
					unshaded_colors[dataSize] = color4(r,g,b,1);
					triangle_indexes[dataSize] = v3;
					data[dataSize++] = vertices[v3];
				}
			}
			else if (strcmp(str1,"mtllib")==0)
			{
				char mtl_name[100];
				fscanf(input,"%s", mtl_name);
				FILE* mtl_input = fopen(mtl_name, "r+");
				if (mtl_input == NULL)
				{
					perror ("Error opening mtllib");
					getchar();
				}
				else
				{
					do
					{
						fscanf(mtl_input,"%s", mtl_data[mtl_count]);
						mtl_count++;
					}
					while(feof(mtl_input) == 0);
				}
			}
			else if (strcmp(str1,"g")==0)
			{
				char group[100];
				fscanf(input,"%s", group);
				for (int i = 0; i < mtl_count; i++)
				{
					if (strcmp(group, mtl_data[i])==0)
					{
						i += 5;
						r = atof(mtl_data[i + 1]);
						g = atof(mtl_data[i + 2]);
						b = atof(mtl_data[i + 3]);
					}
				}
				int temp_group;
				if (strcmp(group,"shoe1L") == 0 || strcmp(group,"shoe2L") == 0 || strcmp(group,"shoe3L") == 0 || strcmp(group,"shoe4L") == 0)
				{
					temp_group = LEFT_FOOT;
				}
				else if (strcmp(group,"shoe1R") == 0 || strcmp(group,"shoe2R") == 0 || strcmp(group,"shoe3R") == 0 || strcmp(group,"shoe4R") == 0)
				{
					temp_group = RIGHT_FOOT;
				}
				else if (strcmp(group,"lowlegL") == 0)
				{
					temp_group = LEFT_LOWER_LEG;
				}
				else if (strcmp(group,"lowlegR") == 0)
				{
					temp_group = RIGHT_LOWER_LEG;
				}
				else if (strcmp(group,"uplegL") == 0)
				{
					temp_group = LEFT_UPPER_LEG;
				}
				else if (strcmp(group,"uplegR") == 0)
				{
					temp_group = RIGHT_UPPER_LEG;
				}
				else if (strcmp(group,"lowarmL") == 0 || strcmp(group,"handL") == 0 || strcmp(group,"sleaveL") == 0)
				{
					temp_group = LEFT_LOWER_ARM;
				}
				else if (strcmp(group,"lowarmR") == 0 || strcmp(group,"handR") == 0 || strcmp(group,"sleaveR") == 0)
				{
					temp_group = RIGHT_LOWER_ARM;
				}
				else if (strcmp(group,"uparmL") == 0)
				{
					temp_group = LEFT_UPPER_ARM;
				}
				else if (strcmp(group,"uparmR") == 0)
				{
					temp_group = RIGHT_UPPER_ARM;
				}
				else
				{
					temp_group = TORSO;
				}
				current_group = temp_group;
			}
			else
			{
				// line began with something else - ignore for now
				fscanf(input, "%[^\n]%*c", str2);
				//printf("Junk line : %s %s\n", str1, str2);
			}
		}
	}

	for (int i = 0; i < dataSize; i++)
	{
		original_data[i] = data[i];
	}
	printf("Facet count = %d\n",facetCount);
	//glutPostRedisplay();
}

void init()
{
	readFile();

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );

	// Create and initialize a buffer object
	/* set up vertex buffer object */
	glGenBuffers(1, &bufferID);
	glBindBuffer(GL_ARRAY_BUFFER, bufferID);

	//  2nd param - size of the buffer is in BYTES,  buffer is big enough to hold both vertices and colors (so * 2)    
	glBufferData(GL_ARRAY_BUFFER, sizeof(point4)*dataSize*2, NULL, GL_STATIC_DRAW);
	//  first info in buffer is vertices,  starting at byte offset 0,  then size of vertices data in bytes, then location of vertices data
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(point4)*dataSize, data );
	//  next info in buffer is colors, starting where vertices ended,  same size
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(point4)*dataSize, sizeof(point4)*dataSize, colors);




	// Load shaders and use the resulting shader program
	programID = InitShader("vshaderSD3.glsl", "fshaderSD3.glsl");
	glUseProgram(programID);

	vPositionLoc = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPositionLoc);
	// last parameter is starting byte offset for vertices (vPosition)
	glVertexAttribPointer(vPositionLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);   

	vColorLoc = glGetAttribLocation(programID, "vColor");
	glEnableVertexAttribArray(vColorLoc);
	// last parameter is starting byte offset for colors (vColor)
	glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4)*dataSize));

	ctmID = glGetUniformLocation(programID, "CTM");

	glClearColor(1.0, 1.0, 1.0, 1.0); /* white background */
}

void display()
{
	vec4 light_direction(0, 0, 1, 0);
	light_direction = RotateY(yRot) * light_direction;
	float normal_sum[3619];
	int normal_count[3619];
	for (int i = 0; i < 3619; i++)
	{
		normal_sum[i] = 0;
		normal_count[i] = 0;
	}
	int v[3];
	for (int i = 0; i < dataSize / 3; i++)
	{
		int triangle_index = i * 3;
		vec4 triangle_vertices[3];
		for (int j = 0; j < 3; j++)
		{
			v[j] = triangle_indexes[triangle_index + j];
			triangle_vertices[j] = vertices[v[j]];
		}
		float normal = dot(normalize(light_direction), normalize(cross(triangle_vertices[1] - triangle_vertices[0], triangle_vertices[2] - triangle_vertices[0])));
		for (int j = 0; j < 3; j++)
		{
			normal_sum[v[j]] = normal_sum[v[j]] + normal;
			normal_count[v[j]]++;
		}
	}
	for (int i = 0; i < dataSize / 3; i++)
	{
		int triangle_index = i * 3;
		for (int j = 0; j < 3; j++)
		{
			v[j] = triangle_indexes[triangle_index + j];
			float normal_average = normal_sum[v[j]] / normal_count[v[j]];
			float red = unshaded_colors[triangle_index + j].x * normal_average;
			float green = unshaded_colors[triangle_index + j].y * normal_average;
			float blue = unshaded_colors[triangle_index + j].z * normal_average;
			colors[triangle_index + j] = color4(red, green, blue,1);
		}
	}
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(point4)*dataSize, sizeof(point4)*dataSize, colors);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
	CTM = RotateY(yRot)*Scale(0.3,0.3,-0.3);
	glUniformMatrix4fv(ctmID, 1, GL_TRUE, CTM); // send CTM to shader
	glDrawArrays(GL_TRIANGLES, 0, dataSize); 
	glutSwapBuffers();
}



void myspecialkey(int key, int mousex, int mousey)
{
	int xRot = 0;
	if(key == 100)
	{
		yRot += 1;
		if (yRot > 360.0 )
		{
			yRot -= 360.0;
		}
	}
	else if(key == 102)
	{
		yRot -= 1;
		if (yRot < -360.0 )
		{
			yRot += 360.0;
		}
	}
	else if(key ==  GLUT_KEY_UP)
	{
		xRot = 1;
	}
	else if(key ==  GLUT_KEY_DOWN)
	{
		xRot = -1;
	}
	if(moving_group >= 0 && (key ==  GLUT_KEY_UP || key ==  GLUT_KEY_DOWN))
	{
		group_transformations[moving_group] = Translate(group_inverses[moving_group]) * RotateX(xRot) * Translate(group_origins[moving_group]) * group_transformations[moving_group];
		for (int i = 0; i < NUMBER_OF_GROUPS; i++)
		{
			for (int j = 0; j < group_count[i]; j++)			
			{
				data[group_indexes[i][j]] = group_transformations[i] * original_data[group_indexes[i][j]];
				int k = group_dependencies[i];
				while (k >= 0)
				{
					data[group_indexes[i][j]] =  group_transformations[k] * data[group_indexes[i][j]];
					k = group_dependencies[k];
				}
			}
		}
	}
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(point4)*dataSize, data );
	glutPostRedisplay();
}



void mykey(unsigned char key, int mousex, int mousey)
{
	float dr = 3.14159/180.0*5.0;
	if(key == 'q' || key == 'Q')
	{
		exit(0);
	}
	else if(key == 'a')
	{
		moving_group = LEFT_FOOT;
	}
	else if(key == 'A')
	{
		moving_group = RIGHT_FOOT;
	}
	else if(key == 'k')
	{
		moving_group = LEFT_LOWER_LEG;
	}
	else if(key == 'K')
	{
		moving_group = RIGHT_LOWER_LEG;
	}
	else if(key == 'h')
	{
		moving_group = LEFT_UPPER_LEG;
	}
	else if(key == 'H')
	{
		moving_group = RIGHT_UPPER_LEG;
	}
	else if(key == 'e')
	{
		moving_group = LEFT_LOWER_ARM;
	}
	else if(key == 'E')
	{
		moving_group = RIGHT_LOWER_ARM;
	}
	else if(key == 's')
	{
		moving_group = LEFT_UPPER_ARM;
	}
	else if(key == 'S')
	{
		moving_group = RIGHT_UPPER_ARM;
	}
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(point4)*dataSize, data );
	glutPostRedisplay();
}

void reshape(GLsizei ww, GLsizei hh)
{
	glViewport(0,0, ww, hh);
}

int main(int argc, char** argv)
{
	vec4 left_foot_origin(-0.8, 2.9, -0.1, 0);
	vec4 right_foot_origin(0.8, 2.9, -0.1, 0);
	vec4 left_lower_leg_origin(-0.7, 2.1, -0.23, 0);
	vec4 right_lower_leg_origin(0.7, 2.1, -0.23, 0);
	vec4 left_upper_leg_origin(-0.55, 1.5, -0.15, 0);
	vec4 right_upper_leg_origin(0.55, 1.5, -0.15, 0);
	vec4 left_lower_arm_origin(-2, -0.06, 0.30, 0);
	vec4 right_lower_arm_origin(2, -0.06, 0.30, 0);
	vec4 left_upper_arm_origin (-1.3, -0.35, 0.32, 0);
	vec4 right_upper_arm_origin(1.3, -0.35, 0.32, 0);
	vec4 torso_origin(0, 0, 0, 0);

	group_origins[LEFT_FOOT] = left_foot_origin;
	group_origins[RIGHT_FOOT] = right_foot_origin;
	group_origins[LEFT_LOWER_LEG] = left_lower_leg_origin;
	group_origins[RIGHT_LOWER_LEG] = right_lower_leg_origin;
	group_origins[LEFT_UPPER_LEG] = left_upper_leg_origin;
	group_origins[RIGHT_UPPER_LEG] = right_upper_leg_origin;
	group_origins[LEFT_LOWER_ARM] = left_lower_arm_origin;
	group_origins[RIGHT_LOWER_ARM] = right_lower_arm_origin;
	group_origins[LEFT_UPPER_ARM] = left_upper_arm_origin;
	group_origins[RIGHT_UPPER_ARM] = right_upper_arm_origin;
	group_origins[TORSO] = torso_origin;

	group_dependencies[LEFT_FOOT] = LEFT_LOWER_LEG;
	group_dependencies[RIGHT_FOOT] = RIGHT_LOWER_LEG;
	group_dependencies[LEFT_LOWER_LEG] = LEFT_UPPER_LEG;
	group_dependencies[RIGHT_LOWER_LEG] = RIGHT_UPPER_LEG;
	group_dependencies[LEFT_UPPER_LEG] = NONE;
	group_dependencies[RIGHT_UPPER_LEG] = NONE;
	group_dependencies[LEFT_LOWER_ARM] = LEFT_UPPER_ARM;
	group_dependencies[RIGHT_LOWER_ARM] = RIGHT_UPPER_ARM;
	group_dependencies[LEFT_UPPER_ARM] = NONE;
	group_dependencies[RIGHT_UPPER_ARM] = NONE;
	group_dependencies[TORSO] = NONE;

	for (int i = 0; i < NUMBER_OF_GROUPS; i++)
	{
		vec4 inverse(0, 0, 0, 0);
		inverse.x = -group_origins[i].x;
		inverse.y = -group_origins[i].y;
		inverse.z = -group_origins[i].z;
		group_inverses[i] = inverse;

		mat4 transformation;
		group_transformations[i] = transformation;

		group_count[i] = 0;
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);

	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutCreateWindow("Detective Al Capone - With Shaders");

	glewExperimental = GL_TRUE;  // new in version 2 of this file.  probably can't hurt - might help
	glewInit();
	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(mykey);
	glutSpecialFunc(myspecialkey);

	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
	return 0;
}

