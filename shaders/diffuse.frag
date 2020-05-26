#version 450 core

layout (location = 0) out vec4 fColor;

uniform vec3 worldCamPos;

uniform sampler2D tex;
uniform sampler2D normalMap;
uniform vec4 ambientColor;

//----in vars
in Vertex {
	vec2 uv;
	vec3 worldNormal;
	vec3 worldPos;
	mat4x4 worldMat;
	mat3x3 tangentToModelMat;
};

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
	vec3 specularColor = vec3(1, 1, 1);

	// Get "normal" from the normalmap
	vec3 normal = texture(normalMap, uv).rgb;
	normal = normal * 2 - 1;
	normal = normalize(tangentToModelMat * normal);
	//normal = normalize(mat3(worldMat) * normal);//not for method 1

	vec3 viewDir = normalize(worldCamPos - worldPos);
	vec3 halfDir = normalize(viewDir + worldLightDir);

	vec4 albedo = texture(tex, uv);
	ambient = ambient * 0.7 * albedo.rgb;
	vec3 diffuse = worldLightColor * ambient.rgb * clamp(dot(worldLightDir, worldNormal), 0.0, 1.0);

	float spec = pow(max(dot(halfDir, worldNormal), 0.0), 512);
	vec3 specular = worldLightColor * specularColor.rgb * spec;

	fColor = vec4(ambient + diffuse, 1);
	//fColor = vec4(albedo.rgb, 1);
	//fColor = albedo;
	//fColor = vec4(1, 1, 0, 1);
}