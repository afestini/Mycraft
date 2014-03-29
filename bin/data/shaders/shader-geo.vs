#version 150
#extension GL_EXT_gpu_shader4 : enable

in ivec4 vertex;
in ivec4 light;

flat out ivec4 position;
flat out ivec4 lights;

void main(void) 
{
	position = vertex;
	lights = light;
}
