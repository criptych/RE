#version 330 core

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec2 texCoords;

out VS_OUT
{
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec2 texCoords;
} vs_out;

layout(std140) uniform RenderMatrices
{
	mat4 view;
	mat4 projection;
};

uniform mat4 model;
uniform mat3 normalMat;

void main()
{
	
	//gl_Position = vec4(inPosition.x, inPosition.y, 0, 1);
	vec4 worldPos = model * vec4(position, 1.0f);
	gl_Position = projection * view * worldPos;
	//gl_Position.xy = inPosition.xy * 0.5f;
	//gl_Position.zw = vec2(0, 1);
	vs_out.position = worldPos.xyz;
	vs_out.normal = normalMat * normal;
	vs_out.tangent = normalMat * tangent;
	vs_out.texCoords = texCoords;
}