#version 150

#extension GL_EXT_texture_array : enable

precision highp float;

uniform sampler2DArray texture;

in float light;
in vec3 texCoord;
in float screenDistance;

out vec4 color;

void main()
{
	color = texture2DArray(texture, texCoord);
	color.rgb *= light;
	color = mix(color, vec4(.4,.7,.9,1.0), screenDistance);
}
