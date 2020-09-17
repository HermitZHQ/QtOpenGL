#version 450 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 4) out vec3 gAlbedo2;

uniform sampler2D tex;
uniform sampler2D normalMap;

uniform mat4x4 viewMat;
uniform mat4x4 projMat;

//----in vars
in Vertex {
	vec2 uv;
	vec3 skyboxUV;
	vec3 worldNormal;
	vec3 worldPos;
	vec3 viewPos;
	mat4x4 worldMat;
	vec3 camPosWorld;
	mat3x3 tangentToModelMat;
};

void main()
{
	gPosition = worldPos;
	gPosition = viewPos;// view space for SSAO

	// Get "normal" from the normalmap
	vec3 normal = texture(normalMap, uv).rgb;
	normal = normal * 2 - 1;
	normal = normalize(tangentToModelMat * normal);
	//normal = normalize((viewMat * vec4(normal, 1)).xyz);// View space for SSAO

	// This is very important, you can't use viewMat * worldNormal
	// you must use inverse&transpose
	// ��ʾ�㣺�������ת�ñ���Ҫʹ�ã��������Ĵ���ģ�����һ��Ҫʹ����ת����ת��
	// ���ӳ���Ⱦ��ȡ�����Ч���Ļ���Ҳ����ֻ����viewMat���棬�����ǳ�����ת�õ��棬��������
	// ʹ������ע�͵ĵط���˵���ǳ���Ҫ�����ݴ��
	normal = normalize((transpose(inverse(viewMat)) * vec4(normal, 0)).xyz);
	// �������ʹ��worldNormal�Ļ�������������͹�У�����ƽ��ķ���ǳ���Բ�����������ܼ�
	//normal = normalize((transpose(inverse(viewMat)) * vec4(worldNormal, 0)).xyz); 

	// for test, decompose the normal in deferred rendering fragment
	//normal = normalize((transpose(inverse(viewMat)) * vec4(1, 0, 0, 0)).xyz); 

	gNormal = normal;

	gAlbedo = texture(tex, uv).rgb;
	gAlbedo2 = texture(tex, uv).rgb;
}