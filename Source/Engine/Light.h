#pragma once

#include "Math/REMath.h"

#include "Mesh.h"
#include "Material.h"

class Texture;

#define MAX_CASCADE_COUNT 3

struct ShadowData
{
	Texture* shadowMap = 0;
	Vector4_3 bounds; // x cascade width, y cascade height, z far plane

	// set in shadow pass
	// for spot/directional light this is remap * lightProj * lightView * invCameraView, which converts VS -> WS -> LS -> [-1, 1] -> [0, 1]
	// for point light this is lightView * invCameraView, which converts VS -> WS -> LS
	Matrix4 shadowMat;

	// set in shadow pass
	// only used for point light
	Matrix4 lightProjRemapMat;
};

class Light
{
public:

	Vector4_3 position;
	Vector4_3 direction;
	Vector4_3 color;
	float intensity;
	float radius;
	float outerHalfAngle;
	float innerHalfAngle;

	bool bCastShadow;
	bool bDynamic;
	bool bVolumetricFog;

	Vector4_3 colorIntensity;
	float invRadius;
	float radialAttenuationBlend;
	float outerCosHalfAngle;
	float outerTanHalfAngle;
	float invDiffCosHalfAngle;
	float endRadius;

	Vector4 attenParams; // bRadial, bSpot, outerCosHalfAngle, invDiffCosHalfAngle

	Matrix4 modelMat;
	Matrix4 lightViewMat;

	ShadowData shadowData[MAX_CASCADE_COUNT];

	Mesh* LightMesh;

	Light(Mesh* inLightMesh)
	{
		LightMesh = inLightMesh;
		position = Vector4_3(0, 0, 0);
		direction = Vector4_3(0, 0, -1);
		color = Vector4_3(0, 0, 0);
		intensity = 0;
		radius = 1;
		invRadius = 1;
		radialAttenuationBlend = 0;
		outerCosHalfAngle = 0;
		invDiffCosHalfAngle = 1;
		attenParams = Vector4(0, 0, 0, 1);

		bCastShadow = false;
		bDynamic = false;
		bVolumetricFog = false;
	}

	void SetRadius(float inRadius)
	{
		radius = inRadius;
		if (radius > KINDA_SMALL_NUMBER)
		{
			invRadius = 1.f / radius;
			radialAttenuationBlend = Clamp((radius - 2.5f) / 9.f, 0.f, 1.f);
		}
	}

	void BuildModelMat()
	{
		if (attenParams.y > 0)
		{
			// spot light
			modelMat = MakeMatrixFromForward(direction);
			modelMat.SetTranslation(position);

			lightViewMat = modelMat.GetTransformInverseNoScale();

			modelMat.ApplyScale(endRadius, endRadius, radius);
		}
		else if (attenParams.x > 0)
		{
			// point light
			modelMat = Matrix4::Identity();
			modelMat.SetTranslation(position);
			modelMat.ApplyScale(radius);

			lightViewMat = Matrix4::Identity();
			lightViewMat.SetTranslation(-position);
		}
		else
		{
			lightViewMat = MakeMatrixFromForward(direction).GetTransposed3();
		}
	}

	void SetDirectionLight(Vector4_3 inDir, Vector4_3 inColor, float inIntensity)
	{
		direction = inDir.GetNormalized3();
		color = inColor;
		intensity = inIntensity;

		colorIntensity = color * intensity;

		attenParams.x = 0;
		attenParams.y = 0;
		attenParams.z = outerCosHalfAngle;
		attenParams.w = invDiffCosHalfAngle;

		BuildModelMat();

		//LightMesh->material->SetParameter(ShaderNameBuilder("light")("color").c_str(), colorIntensity);
		//LightMesh->material->SetParameter(ShaderNameBuilder("light")("attenParams").c_str(), attenParams);
	}

	void SetPointLight(Vector4_3 inPos, float inRadius, Vector4_3 inColor, float inIntensity)
	{
		position = inPos;
		color = inColor;
		intensity = inIntensity;

		SetRadius(inRadius);
		
		colorIntensity = color * intensity;

		attenParams.x = 1;
		attenParams.y = 0;
		attenParams.z = outerCosHalfAngle;
		attenParams.w = invDiffCosHalfAngle;

		BuildModelMat();

		LightMesh->material->SetParameter(ShaderNameBuilder("light")("color").c_str(), colorIntensity, 3);
		LightMesh->material->SetParameter(ShaderNameBuilder("light")("attenParams").c_str(), attenParams, 4);
	}

	void SetSpotLight(Vector4_3 inPos, Vector4_3 inDir, float inRadius, float inOuterHalfAngle, float inInnerHalfAngle, Vector4_3 inColor, float inIntensity)
	{
		position = inPos;
		direction = inDir.GetNormalized3();
		color = inColor;
		intensity = inIntensity;
		outerHalfAngle = inOuterHalfAngle;
		innerHalfAngle = inInnerHalfAngle;

		SetRadius(inRadius);

		colorIntensity = color * intensity;
		float outerRad = DegToRad(outerHalfAngle);
		float innerRad = DegToRad(innerHalfAngle);
		outerCosHalfAngle = Cos(outerRad);
		outerTanHalfAngle = Tan(outerRad);
		invDiffCosHalfAngle = 1.f / (cosf(innerRad) - outerCosHalfAngle);
		endRadius = radius * outerTanHalfAngle;

		attenParams.x = 1;
		attenParams.y = 1;
		attenParams.z = outerCosHalfAngle;
		attenParams.w = invDiffCosHalfAngle;

		BuildModelMat();

		LightMesh->material->SetParameter(ShaderNameBuilder("light")("color").c_str(), colorIntensity, 3);
		LightMesh->material->SetParameter(ShaderNameBuilder("light")("attenParams").c_str(), attenParams, 4);
	}

	void SetPosition(const Vector4_3& inPosition)
	{
		position = inPosition;
		BuildModelMat();
	}

	void SetDirection(const Vector4_3& inDirection)
	{
		direction = inDirection;
		BuildModelMat();
	}

	Vector4 GetPositionVSInvR(const Matrix4& viewMat) const
	{
		Vector4 result = viewMat.TransformPoint(position);
		result.w = invRadius;
		return result;
	}

	Vector4 GetDirectionVSRAB(const Matrix4& viewMat) const
	{
		// re-normalized it here, make sure light direction is unit vector
		Vector4 result = viewMat.TransformVector(direction).GetNormalized3();
		result.w = radialAttenuationBlend;
		return result;
	}
};