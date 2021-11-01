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

uniform float meltThreshold;// map to angle 1-360
uniform float uQuantLevel;// map to strength 1-10(/100.0f)
uniform float uWaterPower;// map to sample num 1-100(/100.0f)

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

//looks good @50, can go pretty high tho
const vec2 iResolution = vec2(800, 600);

vec4 dirBlur(sampler2D tex, vec2 uv, vec2 angle)
{
	int samples = int(uWaterPower);
    vec3 acc = vec3(0);
    
    const float delta = 2.0 / float(samples);
    
    for(float i = -1.0; i <= 1.0; i += delta)
    {
        acc += texture(tex, uv - vec2(angle.x * i, angle.y * i)).rgb;
    }
    
    return vec4((1.0 / float(samples)) * acc, 0);
}


void main()
{
    vec2 screen_uv = gl_FragCoord.xy / iResolution.xy;
	screen_uv.y = 1.0 - screen_uv.y;
    
    //quick converter so that I can input an angle and it'll always
    float angle = meltThreshold;
    float strength = uQuantLevel / 1000.0f;
    
    float r = radians(angle);
    vec2 direction = normalize(vec2(sin(r), cos(r)));
    
    fColor = dirBlur(tex, screen_uv, strength*direction);
	//fColor = texture(tex, screen_uv);
}