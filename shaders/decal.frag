#version 450 core

layout (location = 0) out vec4 fColor;

uniform mat4x4 projMat;
uniform mat4x4 orthoMat;
uniform mat4x4 viewMat;

uniform samplerCube skybox;

uniform sampler2D tex;
uniform sampler2D projTex;


//----in vars
in Vertex {
	vec2 uv;
	vec3 skyboxUV;
	vec3 worldNormal;
	vec3 worldPos;
	vec3 camPosWorld;
	mat4x4 worldMat;
	mat3x3 tangentToModelMat;
};

void main()
{
	vec4 projPos = (projMat * viewMat * vec4(worldPos, 1));
	vec2 projUV = projPos.xy / projPos.w;
	projUV = (projUV + 1.0) / 2.0;
	projUV.y = 1.0 - projUV.y;

	fColor = texture(projTex, projUV);
	//fColor = vec4(0, 1, 0, 1);
}