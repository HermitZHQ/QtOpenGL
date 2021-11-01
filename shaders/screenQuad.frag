#version 450 core

layout (location = 0) out vec4 fColor;

struct Light
{
	bool			isEnabled;
	bool			isDirectional;
	bool			isPoint;
	vec3			dir;
	vec3			pos;
	vec4			color;
	float			radius;
	float			constant;
	float			linear;
	float			quadratic;
	float			innerCutoff;
	float			outerCutoff;
};
uniform Light lights[8];

uniform sampler2D tex;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;
uniform samplerCube skybox;

uniform mat4x4 lightVPMat;

uniform vec4 ambientColor;
uniform vec4 specularColor;

uniform bool hasNormalMap;

uniform float meltThreshold;

//----in vars
in Vertex {
	vec2 uv;
	vec3 skyboxUV;
	vec3 worldNormal;
	vec3 worldPos;
	mat4x4 worldMat;
	vec3 camPosWorld;
	mat3x3 tangentToModelMat;
};

uniform float uQuantLevel;   // 2-6  
uniform float uWaterPower;   // 8-64  
const vec2 texSize = vec2(256., 256.);  

vec4 quant(vec4 cl, float n)  
{
   cl.x = floor(cl.x * 255./n)*n/255.;  
   cl.y = floor(cl.y * 255./n)*n/255.;  
   cl.z = floor(cl.z * 255./n)*n/255.;  
   return cl;  
}

void main(void)  
{
   vec4 noiseColor = uWaterPower * texture2D(normalMap, uv);  
   vec2 newUV = vec2(uv.x + noiseColor.x / texSize.x, uv.y + noiseColor.y / texSize.y);  
   vec4 fColor = texture2D(tex, newUV);                    
   vec4 color = quant(fColor, 255./pow(2., uQuantLevel));  
   //vec4 color = vec4(1., 1., .5, 1.);  
   gl_FragColor = color;  
}