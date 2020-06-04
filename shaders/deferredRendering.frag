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

uniform samplerCube skybox;

uniform mat4x4 lightVPMat;

uniform vec4 ambientColor;
uniform vec4 specularColor;

//----in vars
in Vertex {
	vec2 uv;
	vec3 camPosWorld;
};

float CalculateTheShadowValue()
{	
	vec3 normal = texture(gBufferNormalTex, uv).xyz;
	vec3 wPos = texture(gBufferPosTex, uv).xyz;

	vec4 posInLightSpace = lightVPMat * vec4(wPos, 1);
	posInLightSpace = posInLightSpace / posInLightSpace.w;
	posInLightSpace = (posInLightSpace + 1.0) / 2.0;
	float vertexDepth = posInLightSpace.z;
	float depth = texture(shadowMap, posInLightSpace.xy).x;

	float bias = 0.000001;
	//float bias = max(0.000005 * (1.0 - dot(normal, vec3(1, 1, 1))), 0.000001);
	return (depth < vertexDepth - bias ? 0.05 : 1);
}

vec4 CalculateDirLight(Light light)
{
	light.dir = normalize(light.dir);

	float shadowValue = CalculateTheShadowValue();

	vec3 normal = texture(gBufferNormalTex, uv).xyz;
	vec3 wPos = texture(gBufferPosTex, uv).xyz;

	vec3 ambient = ambientColor.rgb;
	vec3 albedo = texture(gBufferAlbedoTex, uv).rgb;
	ambient = ambient * 0.35 * albedo;

	vec3 viewDir = normalize(camPosWorld - wPos);
	vec3 halfDir = normalize(viewDir + light.dir);
	vec3 diffuse = light.color.rgb * ambient.rgb * clamp(dot(light.dir, normal), 0.0, 1.0);
	diffuse += texture(gBufferSkyboxTex, uv).rgb;

	float spec = pow(max(dot(halfDir, normal), 0.0), 512);
	vec3 specularRes = light.color.rgb * specularColor.rgb * spec;

	//return vec4(texture(gBufferSkyboxTex, uv).rgb, 1);
	return vec4(ambient + diffuse + specularRes, 1);
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
			fColor += CalculatePointLight(lights[i]);
		}
		else if (lights[i].isEnabled && !lights[i].isPoint && !lights[i].isDirectional){
			fColor += CalculateSpotLight(lights[i]);
		}
	}
}