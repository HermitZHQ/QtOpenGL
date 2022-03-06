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

// 31(set in shaderHelper.cpp)
uniform sampler2DArray texArr01;
// 30
uniform sampler3D tex3d01;

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

#define M_PI 3.1415926535897932384626433832795

//--------------------------------------------------functions
//----PBR relevant
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
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
	//return depth;// test depth map texture
	return ((depth < (vertexDepth - bias)) ? 0.05 : 1);
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
	//return vec4(shadowValue, shadowValue, shadowValue, 1);

	vec3 normal = texture(gBufferNormalTex, uv).xyz;
	vec3 wPos = texture(gBufferPosTex, uv).xyz;
	// 警示点：这里又犯了之前一个严重的错误，就是normalize对vec4和vec3使用的结果是完全不一样的
	// 因为一个vec4的变量和矩阵进行计算后，你没有办法保证vec4中的w分量是0，这时候直接对vec4进行标量化就有问题
	// 必须对vec3进行标量化的处理才是正确的，当normal不正确时的调试方式，我们可以采用定值的调试方法
	// 比如在GBuffer的shader中，我们都传递的是vec3(1, 0, 0)和viewMat的组合结果（viewMat在前，且是逆转置）
	//normal = normalize((inverse(inverse(transpose(viewMat))) * vec4(normal, 0)).xyz);

	// 因为inverse两次相互抵消，而transpose可以通过左右换位来实现，最终可以简化为如下算式
	normal = normalize((vec4(normal, 0) * viewMat).xyz);

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
	// 使用平行光方向代替片元和摄像机之间的方向，应该是错误的，因为算散射的方程要求light和view的dot
	//rayDir = light.dir;

	const int steps = 40;
	float stepLen = rayLen / steps;

	vec3 step = rayDir * stepLen;
	vec3 curPos = camPosWorld;

	vec3 accumulateFog = vec3(0, 0, 0);
	vec3 sunColor = vec3(2, 2, 0.6);// vec3(1, 1, 0.3)
	float worldPosShadowVal = 0;
	for(int i = 0; i < steps; ++i){
		worldPosShadowVal = CalculateTheShadowValueWithWorldPos(curPos);
		if (worldPosShadowVal > 0.99){
			accumulateFog += ComputeScattering(dot(rayDir, light.dir)) * sunColor;
		}
		curPos += step;
	}
	accumulateFog /= steps;
	//accumulateFog = rayDir;
	//----test end


	vec3 ambient = ambientColor.rgb;
	vec3 albedo = texture(gBufferAlbedoTex, uv).rgb;
	ambient = ambient * albedo * 0.75;

	vec3 viewDir = normalize(camPosWorld - worldPos);
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

	//shadowValue = CalculateTheShadowValueWithWorldPos(worldPos);
	//return vec4(fogColor, 1);
	vec3 mix_color = albedo * shadowValue + skyboxColor;
	//return vec4(texture(shadowMap, uv));
	//return vec4(shadowValue, shadowValue, shadowValue, 1);
	//return vec4(accumulateFog, 1);
	return vec4(mix_color + accumulateFog, 1);

	// original light mix
	//return vec4(skyboxColor + specularRes + ambient * occlusion + diffuse + fogColor * fogDensity, 1);

	// test light mix
	//return vec4(albedo, 1);
	return vec4(albedo + skyboxColor + specularRes + accumulateFog, 1);
}

vec4 CalculatePointLight(Light light)
{
	vec3 normal = texture(gBufferNormalTex, uv).xyz;
	vec3 wPos = texture(gBufferPosTex, uv).xyz;

	// 将pos和normal从view空间还原到世界空间
	wPos = (inverse(viewMat) * vec4(wPos, 1)).xyz;
	//normal = normalize((inverse(transpose(inverse(viewMat))) * vec4(normal, 0)).xyz);
	// 目前整个PBR的效果已经实现了，但是最奇怪的地方在于高亮光斑在球的内部，需要把normal进行反向，这是唯一一个我暂时不理解的地方
	normal = normalize((vec4(normal, 0) * viewMat).xyz) * -1;

	vec3 pointLightDir = normalize(light.pos - wPos);
	float len = length(light.pos - wPos);

	float attenuation = 1.0;
	// calculate the attenuation of point light
	attenuation = 1 / (light.constant + light.linear * len + light.quadratic * (len * len));
	if (len > light.radius){
		//attenuation = 0.0;
	}

	vec4 albedo = texture(gBufferAlbedoTex, uv);

	vec3 viewDir = normalize(camPosWorld - wPos);
	vec3 halfDir = normalize(viewDir + pointLightDir);
	float dotRes = clamp(dot(pointLightDir, normal), 0.0, 1.0);
	dotRes = dot(pointLightDir, normal);
	vec3 diffuse = light.color.rgb * albedo.rgb * dotRes;
	
	//----specular light res
	float spec = pow(max(dot(halfDir, normal), 0.0), 512);
	vec3 specularRes = light.color.rgb * specularColor.rgb * spec;

	//----PBR
	/**/
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo.rgb, metallic);

	// cook-torrance BRDF
	float NDF = DistributionGGX(normal, halfDir, roughness);
	float G = GeometrySmith(normal, viewDir, pointLightDir, roughness);
	vec3 F = fresnelSchlick(max(dot(halfDir, viewDir), 0.0), F0);

	vec3 kS = F;// reflection
	vec3 kD = vec3(1.0) - kS;// refraction
	kD *= (1.0 - metallic);

	vec3 numerator    = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, pointLightDir), 0.0);
	vec3 specular     = numerator / max(denominator, 0.001);

	float NdotL = max(dot(normal, pointLightDir), 0.0);
	// 原生累计光照公式
    //Lo += (kD * albedo / M_PI + specular) * radiance * NdotL;
	//----PBR return
	vec3 radiance = light.color.rgb * (1.0 / (len * len));
	//return vec4(kS, 1);

	vec3 color = (kD * albedo.rgb / M_PI + specular) * radiance * NdotL;
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));
	return vec4(color, 1);
	

	//----Normal point light return
	//return vec4(diffuse, 1);
	return vec4((diffuse + specularRes) * attenuation, 1);
}

vec4 CalculateSpotLight(Light light)
{	
	vec3 normal = texture(gBufferNormalTex, uv).xyz;
	vec3 wPos = texture(gBufferPosTex, uv).xyz;

	// 将pos和normal从view空间还原到世界空间
	wPos = (inverse(viewMat) * vec4(wPos, 1)).xyz;
	normal = normalize((inverse(transpose(inverse(viewMat))) * vec4(normal, 1)).xyz);

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
			//fColor += CalculateDirLight(lights[i]);
		}
		else if (lights[i].isEnabled && lights[i].isPoint){
			//fColor += CalculatePointLight(lights[i]);
		}
		else if (lights[i].isEnabled && !lights[i].isPoint && !lights[i].isDirectional){
			//fColor += CalculateSpotLight(lights[i]);
		}
	}
	
	// test for 2d array tex
	fColor += texture(texArr01, vec3(uv.x, 1.0 - uv.y, 4));
	//fColor += texture(tex3d01, vec3(uv.x, uv.y, 4));
}