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

// bind unit start from 0-2
uniform sampler2D tex;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;

// bind unit start from 4 to 7
uniform sampler2D gBufferPosTex;
uniform sampler2D gBufferNormalTex;
uniform sampler2D gBufferAlbedoTex;
uniform sampler2D gBufferSkyboxTex;

//9, 10
uniform sampler2D ssaoTex;
uniform sampler2D ssaoBlurTex;

uniform samplerCube skybox;

uniform mat4x4 lightVPMat;
uniform mat4x4 viewMat;

uniform vec4 ambientColor;
uniform vec4 specularColor;

//----PBR params
uniform float metallic;
uniform float roughness;
uniform float ao;

//----in vars
in Vertex {
	vec2 uv;
	vec3 camPosWorld;
	mat4x4 matWorld;
};

//--------------------------------------------------functions
//----PBR relevant
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

//----Shadow relevant
float CalculateTheShadowValue()
{
	vec3 normal = texture(gBufferNormalTex, uv).xyz;
	vec3 wPos = texture(gBufferPosTex, uv).xyz;
	//normal = normalize((vec4(normal, 1) * inverse(viewMat)).xyz);
	wPos = (inverse(viewMat) * vec4(wPos, 1)).xyz;

	vec4 posInLightSpace = lightVPMat * vec4(wPos, 1);
	posInLightSpace = posInLightSpace / posInLightSpace.w;
	posInLightSpace = (posInLightSpace + 1.0) / 2.0;
	float vertexDepth = posInLightSpace.z;
	float depth = texture(shadowMap, posInLightSpace.xy).x;

	float bias = 0.00015;
	//float bias = max(0.000005 * (1.0 - dot(normal, vec3(1, 1, 1))), 0.000001);
	return (depth < vertexDepth - bias ? 0.05 : 1);
}

float CalculateTheShadowValueWithWorldPos(vec3 worldPos)
{	
	vec4 posInLightSpace = lightVPMat * vec4(worldPos, 1);
	posInLightSpace = posInLightSpace / posInLightSpace.w;
	posInLightSpace = (posInLightSpace + 1.0) / 2.0;
	float vertexDepth = posInLightSpace.z;
	float depth = texture(shadowMap, posInLightSpace.xy).x;

	float bias = 0.00015;
	//float bias = max(0.000005 * (1.0 - dot(normal, vec3(1, 1, 1))), 0.000001);
	return (depth < vertexDepth - bias ? 0.05 : 1);
}

// Mie scaterring approximated with Henyey-Greenstein phase function.
float ComputeScattering(float lightDotView)
{
	float G_SCATTERING = 0.2;
	float PI = 3.1415926;
	float result = 1.0f - G_SCATTERING * G_SCATTERING;
	result /= (4.0f * PI * pow(1.0f + G_SCATTERING * G_SCATTERING - (2.0f * G_SCATTERING) * lightDotView, 1.5f));
	return result;
}

vec4 CalculateDirLight(Light light)
{
	light.dir = normalize(light.dir);

	float shadowValue = CalculateTheShadowValue();

	vec3 normal = texture(gBufferNormalTex, uv).xyz;
	vec3 wPos = texture(gBufferPosTex, uv).xyz;
	normal = normalize(vec4(normal, 1) * viewMat).xyz;

	//----test volumetric light
	vec3 worldPos = (inverse(viewMat) * vec4(wPos, 1)).xyz;

	//----fog(use the depth from camera to decide the fog density)
	vec3 fogColor = vec3(0, 0, 0);
	float fogDensity = 0.0;
	//--first fog style(Depth Fog:distance from camera)
	/*
	vec3 diffPos = worldPos - camPosWorld;
	float depth = length(diffPos);

	fogColor = vec3(0.45, 0.45, 0.45);
	if (depth > 60.0){
		fogDensity = min((depth - 60.0) / 60.0, 1.0);
	}*/

	//--second fog style(Height Fog: fog from the specify height)
	/*
	float height = worldPos.y;
	vec3 fogColor = vec3(0.3, 0.3, 0.3);
	float fogDensity = 0.0;
	if (height > 0){
		fogDensity = 1.0 - min((height) / 40.0, 1.0);
	}*/


	vec3 ray = worldPos - camPosWorld;
	float rayLen = length(ray);
	vec3 rayDir = normalize(ray);

	const int steps = 40;
	float stepLen = rayLen / steps;

	vec3 step = rayDir * stepLen;
	vec3 curPos = camPosWorld;

	vec3 accumulateFog = vec3(0, 0, 0);
	for(int i = 0; i < steps; ++i){
		shadowValue = CalculateTheShadowValueWithWorldPos(curPos);
		if (shadowValue > 0.99){
			accumulateFog += ComputeScattering(dot(rayDir, light.dir)) * vec3(1, 1, 0.3);
		}
		curPos += step;
	}
	accumulateFog /= steps;
	//accumulateFog = rayDir;
	//----test end


	vec3 ambient = ambientColor.rgb;
	vec3 albedo = texture(gBufferAlbedoTex, uv).rgb;
	ambient = ambient * albedo * 0.75;

	vec3 viewDir = normalize(camPosWorld - wPos);
	vec3 halfDir = normalize(viewDir + light.dir);
	vec3 diffuse = light.color.rgb * ambient.rgb * clamp(dot(light.dir, normal), 0.0, 1.0);

	vec3 skyboxColor = texture(gBufferSkyboxTex, uv).rgb;
	float threshold = 0.2;
	if (skyboxColor.r < threshold && skyboxColor.r > -threshold 
	&& skyboxColor.g < threshold && skyboxColor.g > -threshold
	&& skyboxColor.b < threshold && skyboxColor.b > -threshold){
	}
	else{
		//diffuse += skyboxColor;
	}
	//diffuse += skyboxColor;

	float spec = pow(max(dot(halfDir, normal), 0.0), 512);
	vec3 specularRes = light.color.rgb * specularColor.rgb * spec;

	// exposure tone mapping
	//float exposure = 3;
    //diffuse = vec3(1.0) - exp(-diffuse * exposure);

    // SSAO
	float occlusion = texture(ssaoBlurTex, uv).r;

	//return vec4(fogColor, 1);
	//return vec4(accumulateFog, 1);

	// original light mix
	//return vec4(skyboxColor + specularRes + ambient * occlusion + diffuse + fogColor * fogDensity, 1);

	// test light mix
	return vec4(skyboxColor + ambient + (specularRes + diffuse) * shadowValue, 1);
}

vec4 CalculatePointLight(Light light)
{
	vec3 normal = texture(gBufferNormalTex, uv).xyz;
	vec3 wPos = texture(gBufferPosTex, uv).xyz;

	vec3 pointLightDir = light.pos - wPos;
	float len = length(light.pos - wPos);
	pointLightDir = normalize(pointLightDir);

	float attenuation = 1.0;
	// calculate the attenuation of point light
	attenuation = 1 / (light.constant + light.linear * len + light.quadratic * (len * len));
	if (len > light.radius){
		//attenuation = 0.0;
	}

	vec4 albedo = texture(gBufferAlbedoTex, uv);

	vec3 viewDir = normalize(camPosWorld - wPos);
	vec3 halfDir = normalize(viewDir + pointLightDir);
	vec3 diffuse = light.color.rgb * albedo.rgb * clamp(dot(pointLightDir, normal), 0.0, 1.0);

	float spec = pow(max(dot(halfDir, normal), 0.0), 512);
	vec3 specularRes = light.color.rgb * specularColor.rgb * spec;

	//return vec4(attenuation, 0, 0, 1);
	return vec4((diffuse + specularRes) * attenuation, 1);
}

vec4 CalculateSpotLight(Light light)
{	
	vec3 normal = texture(gBufferNormalTex, uv).xyz;
	vec3 wPos = texture(gBufferPosTex, uv).xyz;

	vec3 spotLightDir = light.pos - wPos;
	float len = length(spotLightDir);
	spotLightDir = normalize(spotLightDir);
	light.dir = normalize(light.dir);
	float vertexRadian = dot(light.dir, spotLightDir);

	float attenuation = 1.0;
	// calculate the attenuation of point light
	float phi = light.innerCutoff - light.outerCutoff;
	float theta = vertexRadian - light.outerCutoff;
	attenuation = clamp((theta / phi), 0.0, 1.0);
	if (vertexRadian < light.outerCutoff || (len / light.radius) > 1.2){
		//attenuation = 0.0;
	}

	vec4 albedo = texture(gBufferAlbedoTex, uv);

	vec3 viewDir = normalize(camPosWorld - wPos);
	vec3 halfDir = normalize(viewDir + spotLightDir);
	vec3 diffuse = light.color.rgb * albedo.rgb * clamp(dot(spotLightDir, normal), 0.0, 1.0);

	float spec = pow(max(dot(halfDir, normal), 0.0), 512);
	vec3 specularRes = light.color.rgb * specularColor.rgb * spec;

	//return vec4(vertexRadian / 255.0, vertexRadian / 255.0, vertexRadian / 255.0, 1);
	return vec4((diffuse + specularRes) * attenuation, 1);
}

void main()
{
	fColor =  vec4(0, 0, 0, 0);

	for(int i = 0; i < 8; ++i){
		if (lights[i].isEnabled && lights[i].isDirectional){
			fColor += CalculateDirLight(lights[i]);
		}
		else if (lights[i].isEnabled && lights[i].isPoint){
			//fColor += CalculatePointLight(lights[i]);
		}
		else if (lights[i].isEnabled && !lights[i].isPoint && !lights[i].isDirectional){
			//fColor += CalculateSpotLight(lights[i]);
		}
	}
}