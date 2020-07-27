#version 450 core

//layout (location = 0) out vec4 fColor;
/**/
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;


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
uniform sampler2D offScreenTex;
uniform samplerCube skybox;

uniform mat4x4 lightVPMat;

uniform vec4 ambientColor;
uniform vec4 specularColor;

uniform uint time;

//----in vars
in Vertex {
	vec2 uv;
	vec3 skyboxUV;
	vec3 worldNormal;
	vec3 worldPos;
	mat4x4 worldMat;
	vec3 camPosWorld;
	mat3x3 tangentToModelMat;
	vec4 scrPos;
};

float CalculateTheShadowValue()
{	
	vec4 posInLightSpace = lightVPMat * vec4(worldPos, 1);
	posInLightSpace = posInLightSpace / posInLightSpace.w;
	posInLightSpace = (posInLightSpace + 1.0) / 2.0;
	float vertexDepth = posInLightSpace.z;
	float depth = texture(shadowMap, posInLightSpace.xy).x;

	float bias = 0.000001;
	//float bias = max(0.000005 * (1.0 - dot(worldNormal, vec3(1, 1, 1))), 0.000001);
	return (depth < vertexDepth - bias ? 0.05 : 1);
}

vec4 CalculateDirLight(Light light)
{
	light.dir = normalize(light.dir);

	float shadowValue = CalculateTheShadowValue();

	// use this speed to get bump uv
	vec2 speed = time * vec2(0.00001, 0.00001);

	// Get "normal" from the normalmap
	float scale = 12.0;
	vec3 normal1 = texture(normalMap, uv + speed * scale).rgb;
	vec3 normal2 = texture(normalMap, uv - speed * scale).rgb;
	vec3 normal = (normal1 + normal2);
	normal = normal * 2 - 1;
	normal = normalize(tangentToModelMat * normal);

	// get the distortion refraction image
	vec3 ambient = ambientColor.rgb;
	vec2 uv2 = scrPos.xy / scrPos.w;
	uv2 = uv2 * 0.5 + 0.5;
	vec2 offsetUV = uv2 + normal.xy * 0.013;

	vec4 albedo = texture(offScreenTex, offsetUV) * texture(tex, uv + normal.xy * 0.01);
	//vec4 albedo = texture(tex, uv) * 0.2 + texture(offScreenTex, scrPos) * 0.8;
	//vec4 albedo = texture(tex, uv) * 0.5 + texture(skybox, skyboxUV) * 0.5;

	ambient = ambient * 0.95 * albedo.rgb;

	vec3 viewDir = normalize(camPosWorld - worldPos);
	vec3 halfDir = normalize(viewDir + light.dir);
	vec3 diffuse = light.color.rgb * ambient.rgb * clamp(dot(light.dir, normal), 0.0, 1.0);

	vec3 skyUV = reflect(-viewDir, normal);
	vec3 reflColor = texture(skybox, skyUV).rgb;

	float fresnel = pow(1 - clamp(dot(viewDir, normal), 0.0, 1.0), 4);
	vec3 finalColor = reflColor * fresnel + ambient * (1 - fresnel);
	//finalColor = ambient * (1 - fresnel);

	//float spec = pow(max(dot(halfDir, normal), 0.0), 512);
	//vec3 specularRes = light.color.rgb * specularColor.rgb * spec;

	//return vec4(uv2.x, uv2.y, 0, 1);
	return vec4(finalColor, 1);
	return vec4(ambient + (diffuse) * shadowValue, 1);
}

vec4 CalculatePointLight(Light light)
{
	vec3 pointLightDir = light.pos - worldPos;
	float len = length(light.pos - worldPos);
	pointLightDir = normalize(pointLightDir);

	float attenuation = 1.0;
	// calculate the attenuation of point light
	attenuation = 1 / (light.constant + light.linear * len + light.quadratic * (len * len));
	if (len > light.radius){
		//attenuation = 0.0;
	}

	// Get "normal" from the normalmap
	vec3 normal = texture(normalMap, uv).rgb;
	normal = normal * 2 - 1;
	normal = normalize(tangentToModelMat * normal);

	vec4 albedo = texture(tex, uv);

	vec3 viewDir = normalize(camPosWorld - worldPos);
	vec3 halfDir = normalize(viewDir + pointLightDir);
	vec3 diffuse = light.color.rgb * albedo.rgb * clamp(dot(pointLightDir, normal), 0.0, 1.0);

	float spec = pow(max(dot(halfDir, normal), 0.0), 512);
	vec3 specularRes = light.color.rgb * specularColor.rgb * spec;

	//return vec4(attenuation, 0, 0, 1);
	return vec4((diffuse + specularRes) * attenuation, 1);
}

vec4 CalculateSpotLight(Light light)
{
	vec3 spotLightDir = light.pos - worldPos;
	float len = length(spotLightDir);
	spotLightDir = normalize(spotLightDir);
	light.dir = normalize(light.dir);
	float vertexRadian = dot(light.dir, spotLightDir);

	float attenuation = 1.0;
	// calculate the attenuation of point light
	float phi = light.innerCutoff - light.outerCutoff;
	float theta = vertexRadian - light.outerCutoff;
	attenuation = clamp((theta / phi), 0.0, 1.0);
	if (vertexRadian < light.outerCutoff){
		//attenuation = 0.0;
	}

	// Get "normal" from the normalmap
	vec3 normal = texture(normalMap, uv).rgb;
	normal = normal * 2 - 1;
	normal = normalize(tangentToModelMat * normal);

	vec4 albedo = texture(tex, uv);

	vec3 viewDir = normalize(camPosWorld - worldPos);
	vec3 halfDir = normalize(viewDir + spotLightDir);
	vec3 diffuse = light.color.rgb * albedo.rgb * clamp(dot(spotLightDir, normal), 0.0, 1.0);

	float spec = pow(max(dot(halfDir, normal), 0.0), 512);
	vec3 specularRes = light.color.rgb * specularColor.rgb * spec;

	//return vec4(vertexRadian / 255.0, vertexRadian / 255.0, vertexRadian / 255.0, 1);
	return vec4((diffuse + specularRes) * attenuation, 1);
}

void main()
{
	/*
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
	}*/

	/**/
	// use this speed to get bump uv
	vec2 speed = time * vec2(0.00001, 0.00001);

	// Get "normal" from the normalmap
	float scale = 12.0;
	vec3 normal1 = texture(normalMap, uv + speed * scale).rgb;
	vec3 normal2 = texture(normalMap, uv - speed * scale).rgb;
	normal1 = normal1 * 2 - 1;
	normal2 = normal2 * 2 - 1;

	vec3 normal = (normal1 + normal2);
	normal = normalize(tangentToModelMat * normal);

	// get the distortion refraction image
	vec3 ambient = ambientColor.rgb;
	vec2 uv2 = scrPos.xy / scrPos.w;
	uv2 = uv2 * 0.5 + 0.5;
	vec2 offsetUV = uv2 + normal.xy * 0.015;

	vec3 albedo = texture(offScreenTex, offsetUV).rgb;
	vec3 refrColor = ambient * albedo.rgb * 0.3; // the color below the water should more darker

	vec3 viewDir = normalize(camPosWorld - worldPos);
	vec3 halfDir = normalize(viewDir + lights[0].dir);
	vec3 diffuse = lights[0].color.rgb * ambient.rgb * clamp(dot(lights[0].dir, normal), 0.0, 1.0);

	vec3 skyUV = reflect(-viewDir, normal);
	vec3 reflColor = texture(skybox, skyUV).rgb * texture(tex, uv).rgb;

	float fresnel = pow(1 - clamp(dot(viewDir, normal), 0.0, 1.0), 4);
	vec3 finalColor = reflColor * fresnel + refrColor * (1 - fresnel);
	// 暂时使用天空盒rgb，不知道为什么上面的rgb相乘要报错，单独设置到这里都可以。。。。。
	// 我目前怀疑跟HDR有关系，是不是乘法以后超过界限了？？？
	finalColor = texture(skybox, skyUV).rgb;

	gPosition = worldPos;
	gNormal = normal;
	gAlbedo = finalColor;
}