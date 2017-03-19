#version 330 core

struct Light 
{
	vec3 position;
	vec3 direction;
	vec3 diffuse;
	vec3 specular;
	vec3 ambient;
	float radius;
};

#define LIGHT_COUNT 4

in VS_OUT
{
	vec3 positionVS;
	vec2 texCoords;
} fs_in;

out vec4 color;

layout(std140) uniform RenderInfo
{
	mat4 viewMat;
	mat4 projMat;
	vec2 resolution;
};

uniform sampler2D gPositionTex;
uniform sampler2D gNormalTex;
uniform sampler2D gAlbedoSpecTex;
uniform sampler2D gDepthStencilTex;

uniform float specPower;

//uniform vec3 viewPos;
uniform Light lights[LIGHT_COUNT];

vec3 CalcLight(vec3 light, vec3 normal, vec3 view, vec3 colorDiff, vec3 colorSpec, vec3 colorAmb, vec3 albedo, float specIntensity, float attenuation)
{
	vec3 halfVec = normalize(light + view);
	
	vec3 ambient = colorAmb * attenuation  * specIntensity; // hack! using spec to exclude pixel we don't draw on
	vec3 diff = albedo * max(dot(normal, light), 0.f) * colorDiff * attenuation;
	vec3 spec = pow(max(dot(normal, halfVec), 0.f), specPower) * colorSpec * attenuation * specIntensity;
	//spec = vec3(0,0,0);
	
	//return vec3(specV, specV, specV);
	return ambient + diff + spec;
}

vec3 CalcPointLight(Light lightData, vec3 normal, vec3 pos, vec3 view, vec3 albedo, float specIntensity)
{
	vec3 light = vec3(viewMat * vec4(lightData.position, 1.f)) - pos;
	float dist = length(light);
	float attRatio = min(dist / lightData.radius, 1.f);
	float attenuation = 1.f - attRatio * attRatio;
	//float attenuation = 1.f;
	light /= dist;

	return CalcLight(light, normal, view, lightData.diffuse, lightData.specular, lightData.ambient, albedo, specIntensity, attenuation);
}

vec3 CalcDirectionalLight(Light lightData, vec3 normal, vec3 pos, vec3 view, vec3 albedo, float specIntensity)
{
	vec3 light = mat3(viewMat) * -lightData.direction;
	float attenuation = 1.f;

	return CalcLight(light, normal, view, lightData.diffuse, lightData.specular, lightData.ambient, albedo, specIntensity, attenuation);
}

vec3 GetPosition(float depth)
{
	float ndcZ = depth * 2.f - 1.f;
	float viewZ = projMat[3][2] / (projMat[2][3] * ndcZ - projMat[2][2]);
	return fs_in.positionVS / fs_in.positionVS.z * viewZ;
	
	// ref:
	// https://www.khronos.org/opengl/wiki/Compute_eye_space_from_window_space
	// https://mynameismjp.wordpress.com/2010/09/05/position-from-depth-3/
	// https://mynameismjp.wordpress.com/2010/03/22/attack-of-the-depth-buffer/
}

void main() 
{	
	vec2 uv = gl_FragCoord.xy / resolution;
	vec3 normal = normalize(texture(gNormalTex, uv).rgb * 2.0f - 1.0f);
	vec4 depthStencil = texture(gDepthStencilTex, uv);
	float depth = depthStencil.r;
	//vec3 normal = texture(gNormalTex, fs_in.texCoords).rgb;
	//vec3 rposition = texture(gPositionTex, fs_in.texCoords).rgb;
	vec3 position = GetPosition(depth);
	//vec3 view = normalize(viewPos - position);
	vec3 view = normalize(-position);	
	vec4 albedoSpec = texture(gAlbedoSpecTex, uv);
	
	vec3 result = vec3(0,0,0);
	for(int i = 0; i < LIGHT_COUNT; ++i)
	{
		result += CalcDirectionalLight(lights[i], normal, position, view, albedoSpec.rgb, albedoSpec.a);
	}
	//result += CalcPointLight(lights[0], normal, position, view, albedoSpec.rgb, albedoSpec.a);
	color = vec4(result, 1.0f);
	//color = vec4(abs((rposition - position) / rposition) * 100, 1.0f);
	//color = vec4(normal, 1.0f);
	//color = vec4(depthStencil.a, depthStencil.a, depthStencil.a, 1.0f);
	gl_FragDepth = depth;
}