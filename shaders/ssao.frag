#version 450 core

//layout (location = 0) out vec4 fColor;
out float FragColor;

uniform mat4x4 projMat;
uniform mat4x4 viewMat;

//----unit 4-6
uniform sampler2D gBufferPosTex;
uniform sampler2D gBufferNormalTex;
uniform sampler2D gBufferAlbedoTex;

//----unit 8
uniform sampler2D texNoise;

uniform vec3 ssaoSamples[32];

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
	/*
	*/

	vec3 fragPos   = texture(gBufferPosTex, uv).xyz;
	vec3 normal    = texture(gBufferNormalTex, uv).rgb;
	vec3 randomVec = texture(texNoise, uv * noiseScale).xyz;

	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	//tangent = vec3(1, 0, 0);
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	int kernelSize = 32;
	float radius = 0.25;
	float bias = 0.025;
	//FragColor = 0;
	for(int i = 0; i < kernelSize; ++i)
	{
		// get sample position
		vec3 samp = TBN * ssaoSamples[i]; // from tangent to view-space
		samp = fragPos + samp * radius;
    
		vec4 offset = vec4(samp, 1.0);
		offset      = projMat * offset;		  // from view to clip-space
		offset.xyz /= offset.w;               // perspective divide
		offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

		float sampleDepth = texture(gBufferPosTex, offset.xy).z;

		// radius and depth also in the view-space, not the ndc coord, so we can compare them directly
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion       += (sampleDepth >= samp.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	occlusion = 1.0 - (occlusion / kernelSize);
	FragColor = occlusion;

	//fColor = vec4(occlusion, occlusion, occlusion, 1);
}