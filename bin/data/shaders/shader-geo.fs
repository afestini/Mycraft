#version 150
#extension GL_EXT_texture_array : enable

precision highp float;

uniform sampler2DArray texture;

in float light;
in vec3 texCoord;

out vec4 color;

void main()
{
	color = texture2DArray(texture, texCoord);
	color.rgb *= light;
}
