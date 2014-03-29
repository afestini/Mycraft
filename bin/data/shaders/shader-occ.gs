#version 150

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

uniform mat4 mvp;

flat in ivec4 pos[];

void main() 
{
	ivec3 min = pos[0].xyz;
	ivec3 add = ivec3(16, pos[0].w, 16);
	
	vec4 lbr = mvp * vec4(min.x, min.y, min.z, 1.0);
	vec4 lbf = mvp * vec4(min.x, min.y, min.z + add.z, 1.0);
	vec4 ltr = mvp * vec4(min.x, min.y + add.y, min.z, 1.0);
	vec4 ltf = mvp * vec4(min.x, min.y + add.y, min.z + add.z, 1.0);
	
	vec4 rbr = mvp * vec4(min.x + add.x, min.y, min.z, 1.0);
	vec4 rbf = mvp * vec4(min.x + add.x, min.y, min.z + add.z, 1.0);
	vec4 rtr = mvp * vec4(min.x + add.x, min.y + add.y, min.z, 1.0);
	vec4 rtf = mvp * vec4(min.x + add.x, min.y + add.y, min.z + add.z, 1.0);
	
	gl_Position = ltr; EmitVertex();
	gl_Position = lbr; EmitVertex();
	gl_Position = ltf; EmitVertex();
	gl_Position = lbf; EmitVertex();
	EndPrimitive();
	
	gl_Position = rtf; EmitVertex();
	gl_Position = rbf; EmitVertex();
	gl_Position = rtr; EmitVertex();
	gl_Position = rbr; EmitVertex();
	EndPrimitive();
	
	gl_Position = ltf; EmitVertex();
	gl_Position = lbf; EmitVertex();
	gl_Position = rtf; EmitVertex();
	gl_Position = rbf; EmitVertex();
	EndPrimitive();
	
	gl_Position = rtr; EmitVertex();
	gl_Position = rbr; EmitVertex();
	gl_Position = ltr; EmitVertex();
	gl_Position = lbr; EmitVertex();
	EndPrimitive();
	
	gl_Position = ltr; EmitVertex();
	gl_Position = ltf; EmitVertex();
	gl_Position = rtr; EmitVertex();
	gl_Position = rtf; EmitVertex();
	EndPrimitive();
	
	gl_Position = rbr; EmitVertex();
	gl_Position = rbf; EmitVertex();
	gl_Position = lbr; EmitVertex();
	gl_Position = lbf; EmitVertex();
	EndPrimitive();
}
