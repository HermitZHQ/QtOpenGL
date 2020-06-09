#version 450 core

//layout (location = 0) out vec4 fColor;
out float FragColor;

uniform mat4x4 projMat;
uniform mat4x4 viewMat;

uniform sampler2D ssaoTex;

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

// tile noise texture over screen, based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0); // screen = 800x600

void main()
{
	vec2 scaleSize = 1.0 / textureSize(ssaoTex, 0);

	float res = 0;
	for(int x = -2; x < 2; ++x){
		for(int y = -2; y < 2; ++y){
			res += texture(ssaoTex, uv + vec2(x, y) * scaleSize).r;
		}
	}

	FragColor = res / 16.0;
}