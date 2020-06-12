#version 450 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT{
	vec2 uv;
	vec3 normal;
}gs_in[];

uniform uint time;
uniform mat4x4 projMat;

out vec2 texCoords;
out vec3 fColor;

vec3 getNormal(){
	vec3 a = (gl_in[0].gl_Position - gl_in[1].gl_Position).xyz;
	vec3 b = (gl_in[2].gl_Position - gl_in[1].gl_Position).xyz;

	return normalize(cross(a, b));
}

vec4 explode(vec4 pos, vec3 normal){
	float t = time / 1000.0;
	vec3 dirLen = normal * ((sin(t) + 1.0) / 2.0) * 2;
	return pos + vec4(dirLen, 0);
}

void GenerateLine(int index)
{
    gl_Position = projMat * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projMat * (gl_in[index].gl_Position + 
                                vec4(gs_in[index].normal, 0.0));
    EmitVertex();
    EndPrimitive();
}

void main()
{
	fColor = vec3(1, 0, 0);
    GenerateLine(0); // first vertex normal
	fColor = vec3(1, 1, 0);
    GenerateLine(1); // second vertex normal
	fColor = vec3(0, 0, 1);
    GenerateLine(2); // third vertex normal
}  