#include "AnimationHelper.h"


KeyFrame::KeyFrame()
	:TimePos(0.f)
	,Translation(0.f,0.f,0.f)
	,Scale(1.f,1.f,1.f)
	,RotationQuat(0.f,0.f,0.f,1.f)
{

}

KeyFrame::~KeyFrame()
{

}

float BoneAnimation::GetStartTime() const
{
	return KeyFrames.front().TimePos;
}

float BoneAnimation::GetEndTime() const
{
	return KeyFrames.back().TimePos;
}

void BoneAnimation::Interpolate(float t, XMFLOAT4X4& M) const
{
	if (t <= KeyFrames.front().TimePos)
	{
		XMVECTOR scale = XMLoadFloat3(&KeyFrames.front().Scale);
		XMVECTOR trans = XMLoadFloat3(&KeyFrames.front().Translation);
		XMVECTOR quat = XMLoadFloat4(&KeyFrames.front().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(scale, zero, quat, trans));
	}
	else if (t >= KeyFrames.back().TimePos)
	{
		XMVECTOR scale = XMLoadFloat3(&KeyFrames.back().Scale);
		XMVECTOR trans = XMLoadFloat3(&KeyFrames.back().Translation);
		XMVECTOR quat = XMLoadFloat4(&KeyFrames.back().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(scale, zero, quat, trans));
	}
	else
	{
		for (UINT i = 0; i < KeyFrames.size() - 1; ++i)
		{
			if (t >= KeyFrames[i].TimePos && t <= KeyFrames[i + 1].TimePos)
			{
				float lerp = (t - KeyFrames[i].TimePos) / (KeyFrames[i + 1].TimePos - KeyFrames[i].TimePos);

				XMVECTOR scale0 = XMLoadFloat3(&KeyFrames[i].Scale);
				XMVECTOR trans0 = XMLoadFloat3(&KeyFrames[i].Translation);
				XMVECTOR quat0 = XMLoadFloat4(&KeyFrames[i].RotationQuat);

				XMVECTOR scale1 = XMLoadFloat3(&KeyFrames[i+1].Scale);
				XMVECTOR trans1 = XMLoadFloat3(&KeyFrames[i+1].Translation);
				XMVECTOR quat1 = XMLoadFloat4(&KeyFrames[i+1].RotationQuat);

				XMVECTOR scale = XMVectorLerp(scale0, scale1, lerp);
				XMVECTOR trans = XMVectorLerp(trans0, trans1, lerp);
				XMVECTOR quat = XMQuaternionSlerp(quat0,quat1, lerp);

				XMVECTOR zero = XMVectorSet(0.f, 0.f, 0.f, 1.f);
				XMStoreFloat4x4(&M, XMMatrixAffineTransformation(scale, zero, quat, trans));
				break;
			}
		}
	}
}
