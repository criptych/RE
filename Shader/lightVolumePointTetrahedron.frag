#version 430 core

#include "Include/CommonUBO.incl"
#include "Include/DeferredLighting.incl"
#include "Include/DeferredPassTex.incl"
#include "Include/Shadow.incl"
#include "Include/BasicLightVolumeFrag.incl"

in VS_OUT
{
	vec3 positionVS;
} fs_in;

out vec4 color;

uniform Light light;
uniform mat4 shadowMat;
uniform mat4 lightProjRemapMat[4];

void main() 
{	
	vec2 uv = gl_FragCoord.xy / resolution.xy;
	
	vec3 normal, position, view;
	vec4 albedo;
	float metallic, roughness, ao;	
	GetGBufferValue(uv, fs_in.positionVS, normal, position, view, albedo, metallic, roughness, ao);
	
	float shadowFactor = 1;
	if(light.shadowDataCount > 0)
	{
		vec3 lightDir = normalize(light.positionInvR.xyz - position);
		shadowFactor = CalcShadowTetrahedron(position, normal, lightDir, lightProjRemapMat, shadowMat, gShadowTiledTex, 0.00025, 0.0004);
	}
	
	vec3 result = CalcLight(light, normal, position, view, albedo.rgb, metallic, roughness) * min(shadowFactor, 1-ao);
		
	color = vec4(result, 1.0f);
	//color = vec4(vec3(shadowFactor), 1.f);
}