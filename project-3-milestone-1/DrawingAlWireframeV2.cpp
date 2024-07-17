

#include "Angel.h"
#include <string>
#include <iostream>

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define MAX_VERT 30000
point4 data[MAX_VERT];
point4 colors[MAX_VERT];
int dataSize = 0;  // how many vertices

mat4 CTM;

// shader variable locations
GLuint programID;
GLuint bufferID;
GLuint vPositionLoc;
GLuint vColorLoc;
GLuint ctmID;

float yRot = 0.0;

point4 unshaded_colors[MAX_VERT];
int triangle_sequence[MAX_VERT];
vec4 vertices[4000];


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
					// filling in 3 vertices to form a triangle
					// also 3 colors, one for each vertex
					// corresponding colors and vertices are at the same index in each array 
					unshaded_colors[dataSize] = color4(r,g,b,1);
					triangle_sequence[dataSize] = v1;
					data[dataSize++] = vertices[v1];
					unshaded_colors[dataSize] = color4(r,g,b,1);
					triangle_sequence[dataSize] = v2;
					data[dataSize++] = vertices[v2];
					unshaded_colors[dataSize] = color4(r,g,b,1);
					triangle_sequence[dataSize] = v3;
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
				char body_part[100];
				fscanf(input,"%s", body_part);
				for (int i = 0; i < mtl_count; i++)
				{
					if (strcmp(body_part, mtl_data[i])==0)
					{
						i += 5;
						r = atof(mtl_data[i + 1]);
						g = atof(mtl_data[i + 2]);
						b = atof(mtl_data[i + 3]);
					}
				}
			}
			else
			{
				// line began with something else - ignore for now
				fscanf(input, "%[^\n]%*c", str2);
				//printf("Junk line : %s %s\n", str1, str2);
			}
		}
	}
	printf("Facet count = %d\n",facetCount);
	glutPostRedisplay();
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
	float normal_sum[4000];
	int normal_count[4000];
	for (int i = 0; i < 4000; i++)
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
			v[j] = triangle_sequence[triangle_index + j];
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
			v[j] = triangle_sequence[triangle_index + j];
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
	if(key == 100) {
		yRot += 1;
		if (yRot > 360.0 ) {
			yRot -= 360.0;
		}
	}
	if(key == 102) {
		yRot -= 1;
		if (yRot < -360.0 ) {
			yRot += 360.0;
		}
	}
	glutPostRedisplay();
}

void mykey(unsigned char key, int mousex, int mousey)
{
	float dr = 3.14159/180.0*5.0;
	if(key=='q'||key=='Q') exit(0);

	glutPostRedisplay();
}

void reshape(GLsizei ww, GLsizei hh)
{
	glViewport(0,0, ww, hh);
}

int main(int argc, char** argv)
{
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

