#include "SkinnedData.h"



Keyframe::Keyframe()
	:TimePos(0.f)
	, Translation(0.f, 0.f, 0.f)
	, Scale(1.f, 1.f, 1.f)
	, RotationQuat(0.f, 0.f, 0.f, 1.f)
{

}

Keyframe::~Keyframe()
{

}

float BoneAnimation::GetStartTime() const
{
	return Keyframes.front().TimePos;
}

float BoneAnimation::GetEndTime() const
{
	return Keyframes.back().TimePos;
}

void BoneAnimation::Interpolate(float t, XMFLOAT4X4& M) const
{
	if (t <= Keyframes.front().TimePos)
	{
		XMVECTOR scale = XMLoadFloat3(&Keyframes.front().Scale);
		XMVECTOR trans = XMLoadFloat3(&Keyframes.front().Translation);
		XMVECTOR quat = XMLoadFloat4(&Keyframes.front().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(scale, zero, quat, trans));
	}
	else if (t >= Keyframes.back().TimePos)
	{
		XMVECTOR scale = XMLoadFloat3(&Keyframes.back().Scale);
		XMVECTOR trans = XMLoadFloat3(&Keyframes.back().Translation);
		XMVECTOR quat = XMLoadFloat4(&Keyframes.back().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(scale, zero, quat, trans));
	}
	else
	{
		for (UINT i = 0; i < Keyframes.size() - 1; ++i)
		{
			if (t >= Keyframes[i].TimePos && t <= Keyframes[i + 1].TimePos)
			{
				float lerp = (t - Keyframes[i].TimePos) / (Keyframes[i + 1].TimePos - Keyframes[i].TimePos);

				XMVECTOR scale0 = XMLoadFloat3(&Keyframes[i].Scale);
				XMVECTOR trans0 = XMLoadFloat3(&Keyframes[i].Translation);
				XMVECTOR quat0 = XMLoadFloat4(&Keyframes[i].RotationQuat);

				XMVECTOR scale1 = XMLoadFloat3(&Keyframes[i + 1].Scale);
				XMVECTOR trans1 = XMLoadFloat3(&Keyframes[i + 1].Translation);
				XMVECTOR quat1 = XMLoadFloat4(&Keyframes[i + 1].RotationQuat);

				XMVECTOR scale = XMVectorLerp(scale0, scale1, lerp);
				XMVECTOR trans = XMVectorLerp(trans0, trans1, lerp);
				XMVECTOR quat = XMQuaternionSlerp(quat0, quat1, lerp);

				XMVECTOR zero = XMVectorSet(0.f, 0.f, 0.f, 1.f);
				XMStoreFloat4x4(&M, XMMatrixAffineTransformation(scale, zero, quat, trans));
				break;
			}
		}
	}
}

float AnimationClip::GetClipStartTime() const
{
	float t = MathHelper::Infinity;
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = MathHelper::Min(t, BoneAnimations[i].GetStartTime());
	}
	return t;
}

float AnimationClip::GetClipEndTime() const
{
	float t = 0.f;
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = MathHelper::Max(t, BoneAnimations[i].GetEndTime());
	}
	return t;
}

void AnimationClip::Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransforms) const
{
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		BoneAnimations[i].Interpolate(t, boneTransforms[i]);
	}
}

UINT SkinnedData::BoneCount() const
{
	return mBoneHierarchy.size();
}

float SkinnedData::GetClipStartTime(const std::string& clipName) const
{
	std::map<std::string, AnimationClip>::const_iterator it = mAnimations.find(clipName);
	if (it != mAnimations.end())
	{
		return it->second.GetClipStartTime();
	}
	return MathHelper::Infinity;
}

float SkinnedData::GetClipEndTime(const std::string& clipName) const
{
	std::map<std::string, AnimationClip>::const_iterator it = mAnimations.find(clipName);
	if (it != mAnimations.end())
	{
		return it->second.GetClipEndTime();
	}
	return 0.f;
}

void SkinnedData::Set(std::vector<int>& boneHierarchy, std::vector<XMFLOAT4X4>& boneOffsets, std::map<std::string, AnimationClip>& animations)
{
	mBoneHierarchy = boneHierarchy;
	mBoneOffsets = boneOffsets;
	mAnimations = animations;
}

void SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos, std::vector<XMFLOAT4X4>& finalTransforms)
{
	UINT numBones = mBoneOffsets.size();

	std::vector<XMFLOAT4X4> toParentTransforms(numBones);

	std::map<std::string, AnimationClip>::const_iterator it = mAnimations.find(clipName);
	it->second.Interpolate(timePos, toParentTransforms);

	//
	// Traverse the hierarchy and transform all the bones to the root space.
	//
	std::vector<XMFLOAT4X4> toRootTransforms(numBones);

	//The root bone has index 0. The boot bone has no parent, so its toRootTransform is just its local transform.
	toRootTransforms[0] = toParentTransforms[0];

	//Now find the toRootTransform of the children.
	for (UINT i = 1; i < numBones; ++i)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);

		int parentIndex = mBoneHierarchy[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);

		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}

	for (UINT i = 0; i < numBones; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4(&mBoneOffsets[i]);
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
		XMStoreFloat4x4(&finalTransforms[i], XMMatrixMultiply(offset, toRoot));
	}
}
