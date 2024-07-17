#version 150

in vec4 vPosition;
in vec4 vColor;
out vec4 color;

uniform mat4 CTM;

void main() 
{
   gl_Position = CTM*vPosition;
   color = vColor;
} 
