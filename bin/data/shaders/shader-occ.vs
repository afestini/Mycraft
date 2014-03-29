#version 150

in ivec4 vertex;

flat out ivec4 pos;

void main(void) 
{
	pos = vertex;
}
