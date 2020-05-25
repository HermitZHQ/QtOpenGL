#version 450 core

layout (location = 0) out vec4 fColor;

uniform sampler2D tex;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;
uniform samplerCube skybox;

uniform mat4x4 lightVPMat;

uniform vec4 ambientColor;
uniform vec4 specularColor;

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

float CalculateTheShadowValue(){	
	vec4 posInLightSpace = lightVPMat * vec4(worldPos, 1);
	posInLightSpace = posInLightSpace / posInLightSpace.w;
	posInLightSpace = (posInLightSpace + 1.0) / 2.0;
	float vertexDepth = posInLightSpace.z;
	float depth = texture(shadowMap, posInLightSpace.xy).x;

	float bias = 0.000001;
	//float bias = max(0.000005 * (1.0 - dot(worldNormal, vec3(1, 1, 1))), 0.000001);
	return (depth < vertexDepth - bias ? 0.2 : 1);
}

void main()
{
	// assumption directional light color and dir
	vec3 worldLightColor = vec3(1, 1, 1);
	vec3 worldLightDir = vec3(1, 1, 1);
	worldLightDir = normalize(worldLightDir);

	// assumption ambient light color
	vec3 ambient = vec3(1, 1, 1);
	ambient = ambientColor.rgb;
	// assumption diffuse color
	vec3 diffuseColor = vec3(1, 1, 1);
	// assumption specular color
	vec3 specular = vec3(1, 1, 1);
	specular = specularColor.rgb;

	// Get "normal" from the normalmap
	vec3 normal = texture(normalMap, uv).rgb;
	normal = normal * 2 - 1;
	normal = normalize(tangentToModelMat * normal);
	//normal = normalize(mat3(worldMat) * normal);//not for method 1

	// Get depth from the shadowmap
	float shadowValue = CalculateTheShadowValue();

	vec3 viewDir = normalize(camPosWorld - worldPos);
	vec3 halfDir = normalize(viewDir + worldLightDir);

	vec4 albedo = texture(tex, uv) * 0.5 + texture(skybox, skyboxUV) * 0.5;
	//albedo = texture(skybox, skyboxUV);
	ambient = ambient * 0.75 * albedo.rgb;
	vec3 diffuse = worldLightColor * ambient.rgb * clamp(dot(worldLightDir, normal), 0.0, 1.0);

	float spec = pow(max(dot(halfDir, normal), 0.0), 512);
	vec3 specularRes = worldLightColor * specular.rgb * spec;

	fColor = vec4((ambient + diffuse + specularRes) * shadowValue, 1);
	//fColor = vec4(albedo.rgb, 1);
	//fColor = albedo;
	//fColor = vec4(vertexDepth, depth, vertexDepth, 1);
}