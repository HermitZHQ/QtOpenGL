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
uniform mat4x4 insta360;

const vec2 solution = vec2(800, 600);
const vec2 center = solution / 2.0f;
const float midx = solution.x / 2.0f;
const float len_deg = midx / 180.0f;

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

void main()
{	
	float melt_threshold = texture(tex, uv).r;
	if (melt_threshold < meltThreshold){
		//fColor = texture(shadowMap, uv);
	}
	else{
		//fColor = texture(normalMap, uv);
	}

	fColor = texture(tex, uv);

	// test area1(here we just just simple area, y axis bigger than some val)
	vec2 xy = gl_FragCoord.xy;

	// red the outside area first(for test)
	if (length(xy - center) > center.y){
		//fColor = vec4(1, 0, 0, 0);
	}

	float start_r = insta360[0][0];
	float end_r = insta360[0][1];
	float area_width = end_r - start_r;
	float map_start_r = insta360[0][2];
	float map_end_r = insta360[0][3];
	float map_area_width = map_end_r - map_start_r;
	float map_rate = area_width / map_area_width;

	// l&r circle
	float r = length(xy - center);
	if (r > map_start_r && r < map_end_r) {
		vec2 y_axis = vec2(0, 1);
		float off_rad = acos(dot(y_axis, normalize(xy - center)));
		float dir = xy.x > center.x ? 1.0f : -1.0f;

		// cur x,y: (original)x + sin(), (original)y - cos()
		float ox = center.x + degrees(off_rad) * len_deg * dir;
		float oy = start_r + (r - map_start_r) * map_rate;

		fColor = texture(tex, vec2(ox / solution.x, 1.0 - oy / solution.y));
		//fColor = vec4(off_rad, 0, 0, 0);
		if (degrees(off_rad) > 30.0f){
			//fColor = vec4(1, 0, 0, 0);
		}
	}

	// test vector dot angle
	vec2 y = vec2(0, 1);
	vec2 v2 = normalize(vec2(1, 1));
	if (degrees(acos(dot(y, v2))) > 45.02f) {
		//fColor = vec4(1, 1, 0, 0);
	}

	//fColor =  vec4(1, 1, 0, 0);
	//fColor = vec4(lights[0].color);
}