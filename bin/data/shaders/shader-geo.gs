#version 150

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 mvp;
uniform mat4 faceCoords[6];
uniform float playerLight;
uniform ivec3 chunkPos;
uniform float sunlightVals[16];
uniform float blocklightVals[16];

flat in ivec4 position[];
flat in ivec4 lights[];

out vec3 texCoord;
out float light;
	
void EmitFaceVertex(vec4 pos, float lightValue, vec3 uvt)
{
	gl_Position = mvp * pos;
	float screenDistance = dot(gl_Position,gl_Position);
	light = max(lightValue, .7 * pow(playerLight, screenDistance) );
	texCoord = uvt;
	EmitVertex();
}

void main() 
{
	ivec4 pos = position[0];
	int textureID = pos.w;
	int face = pos.z;

	pos = ivec4(chunkPos, 0) + ivec4(pos.x >> 4, pos.y, pos.x & 0xF, 1);
	
	vec4 lightVal;
	for (int i=0; i<4; ++i)
	{
		float sunlight = sunlightVals[lights[0][i] & 0xF];
		float blocklight = blocklightVals[lights[0][i] >> 4];
		lightVal[i] = max(sunlight, blocklight);
	}

	if (abs(lightVal[0] - lightVal[2]) > abs(lightVal[1] - lightVal[3]))
	{
		EmitFaceVertex( pos + faceCoords[face][0], lightVal[0], vec3(0, 0, textureID) );
		EmitFaceVertex( pos + faceCoords[face][1], lightVal[1], vec3(0, 1, textureID) );
		EmitFaceVertex( pos + faceCoords[face][2], lightVal[3], vec3(1, 0, textureID) );
		EmitFaceVertex( pos + faceCoords[face][3], lightVal[2], vec3(1, 1, textureID) );
	}
	else
	{
		EmitFaceVertex( pos + faceCoords[face][2], lightVal[3], vec3(1, 0, textureID) );
		EmitFaceVertex( pos + faceCoords[face][0], lightVal[0], vec3(0, 0, textureID) );
		EmitFaceVertex( pos + faceCoords[face][3], lightVal[2], vec3(1, 1, textureID) );
		EmitFaceVertex( pos + faceCoords[face][1], lightVal[1], vec3(0, 1, textureID) );
	}
	EndPrimitive();
}
