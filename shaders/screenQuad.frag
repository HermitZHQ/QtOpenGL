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

uniform float meltThreshold;// map to blur(val) 10-500
uniform float uQuantLevel;// map to gray_threshold 1-100(/100.0f)
uniform float uWaterPower;// map to gray_rate 1-100(/100.0f)

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

const vec2 texSize = vec2(256., 256.);

float kernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16  
);

void main(void)  
{
	float offset = 1.0 / meltThreshold;
	vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(tex, uv.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

	vec3 res = col * 0.5 + texture(normalMap, uv).xyz * 0.5;
	//Gray = R*0.299 + G*0.587 + B*0.114

	float gray = dot(res.rgb, vec3(0.299, 0.587, 0.114));

	if (gray < uQuantLevel / 10.0f) {
		gray = gray * uWaterPower / 10.0f;
	}
	else{
		gray = gray * (1.0f + uWaterPower / 10.0f);
	}

	gl_FragColor = vec4(res, 1.0);
	gl_FragColor = vec4(vec3(gray), 1.0);
}