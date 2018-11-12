#pragma once
#include "d3dUtil.h"

struct KeyFrame
{
	KeyFrame();
	~KeyFrame();

	float TimePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;
};

struct BoneAnimation
{
	float GetStartTime() const;
	float GetEndTime() const;

	void Interpolate(float t, XMFLOAT4X4& M) const;

	std::vector<KeyFrame> KeyFrames;
};