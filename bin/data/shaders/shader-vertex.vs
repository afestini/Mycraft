#version 150

#extension GL_EXT_gpu_shader4 : enable

uniform mat4 mvp;
uniform ivec3 chunkPos;
uniform float sunlightVals[16];
uniform float blocklightVals[16];
uniform float playerLight;
uniform float fogDensity;

in vec3 vertex;
in vec3 texcoord;
in float lights;

out float light;
out vec3 texCoord;
out float screenDistance;

void main(void)
{
	vec4 pos = mvp * vec4((vertex/255.0) + chunkPos, 1.0);
	
	texCoord = texcoord;
	texCoord.xy /= 255.0;

	float sunlight = sunlightVals[int(lights) & 0xF];
	float blocklight = blocklightVals[int(lights) >> 4];
	light = max(sunlight, blocklight);
	screenDistance = dot(pos,pos);
	light = max(light, .7 * pow(playerLight, screenDistance) );

	float fogExp = screenDistance*fogDensity;
	screenDistance = clamp(1.0-exp2(-fogExp*1.44269), 0.0, 1.0);

	gl_Position = pos;
}
