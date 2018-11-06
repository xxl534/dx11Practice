#include "ParticleSystem.h"
#include "Vertex.h"
#include "Camera.h"
#include "Effects.h"

ParticleSystem::ParticleSystem()
	:mInitVB(NULL)
	, mDrawVB(NULL)
	, mStreamOutVB(NULL)
	, mTexArraySRV(NULL)
	, mRandomTexSRV(NULL)
	, mFirstRun(true)
	, mGameTime(0.f)
	, mTimeStep(0.f)
	, mAge(0.f)
	, mEyePosW(0.f, 0.f, 0.f)
	, mEmitPosW(0.f, 0.f, 0.f)
	, mEmitDirW(0.f, 1.f, 0.f)
{
	
}

ParticleSystem::~ParticleSystem()
{
	ReleaseCOM(mInitVB);
	ReleaseCOM(mDrawVB);
	ReleaseCOM(mStreamOutVB);
}

float ParticleSystem::GetAge() const
{
	return mAge;
}

void ParticleSystem::SetEyePos(const XMFLOAT3& eyePosW)
{
	mEyePosW = eyePosW;
}

void ParticleSystem::SetEmitPos(const XMFLOAT3& emitPosW)
{
	mEmitPosW = emitPosW;
}

void ParticleSystem::SetEmitDir(const XMFLOAT3& emitDirW)
{
	mEmitDirW = emitDirW;
}

void ParticleSystem::Init(ID3D11Device* device, ParticleEffect* fx, ID3D11ShaderResourceView* texArraySRV, ID3D11ShaderResourceView* randomTexSRV, UINT maxParticles)
{
	mMaxParticles = maxParticles;
	mFX = fx;
	mTexArraySRV = texArraySRV;
	mRandomTexSRV = randomTexSRV;

	BuildVB(device);
}

void ParticleSystem::Reset()
{
	mFirstRun = true;
	mAge = 0.f;
}

void ParticleSystem::Update(float dt, float gameTime)
{
	mGameTime = gameTime;
	mTimeStep = dt;

	mAge += dt;
}

void ParticleSystem::Draw(ID3D11DeviceContext* dc, const Camera& cam)
{
	XMMATRIX viewProj = cam.ViewProj();

	mFX->SetViewProj(viewProj);
	mFX->SetGameTime(mGameTime);
	mFX->SetTimeStep(mTimeStep);
	mFX->SetEyePosW(mEyePosW);
	mFX->SetEmitPosW(mEmitPosW);
	mFX->SetEmitDirW(mEmitDirW);
	mFX->SetTexArray(mTexArraySRV);
	mFX->SetRandomTex(mRandomTexSRV);

	dc->IASetInputLayout(InputLayouts::Particle);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(Vertex::Particle);
	UINT offset = 0;

	//dc->Begin(mDebugQuery);
	if (mFirstRun)
		dc->IASetVertexBuffers(0, 1, &mInitVB, &stride, &offset);
	else
		dc->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

	dc->SOSetTargets(1, &mStreamOutVB, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	mFX->StreamOutTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mFX->StreamOutTech->GetPassByIndex(p)->Apply(0, dc);

		if (mFirstRun)
		{
			dc->Draw(1, 0);
			mFirstRun = false;
		}
		else
		{
			dc->DrawAuto();
		}
	}
	/*dc->End(mDebugQuery);
	dc->CopyResource(mDebugVB, mStreamOutVB);
	D3D11_QUERY_DATA_SO_STATISTICS stat;
	while (S_OK != dc->GetData(mDebugQuery, &stat, sizeof(D3D11_QUERY_DATA_SO_STATISTICS), 0))
	{
	}
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(dc->Map(mDebugVB, NULL, D3D11_MAP_READ, 0, &mappedData));
	Vertex::Particle* pPar = (Vertex::Particle*)mappedData.pData;
	dc->Unmap(mDebugVB, 0);*/

	ID3D11Buffer* bufferArray[1] = { 0 };
	dc->SOSetTargets(1, bufferArray, &offset);

	// ping-pong the vertex buffers
	std::swap(mDrawVB, mStreamOutVB);

	//
	// Draw the updated particle system we just streamed-out. 
	//
	dc->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

	mFX->DrawTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mFX->DrawTech->GetPassByIndex(p)->Apply(0, dc);

		dc->DrawAuto();
	}
}

void ParticleSystem::BuildVB(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(Vertex::Particle) * 1;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	Vertex::Particle p;
	ZeroMemory(&p, sizeof(Vertex::Particle));
	p.Age = 0.0f;
	p.Type = 0;

	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = &p;

	HR(device->CreateBuffer(&vbd, &vInitData, &mInitVB));

	vbd.ByteWidth = sizeof(Vertex::Particle) * mMaxParticles;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

	HR(device->CreateBuffer(&vbd, NULL, &mDrawVB));
	HR(device->CreateBuffer(&vbd, NULL, &mStreamOutVB));

	vbd.BindFlags = 0;
	vbd.Usage = D3D11_USAGE_STAGING;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	HR(device->CreateBuffer(&vbd, NULL, &mDebugVB));

	D3D11_QUERY_DESC qd;
	qd.MiscFlags = 0;
	qd.Query = D3D11_QUERY_SO_STATISTICS;
	HR(device->CreateQuery(&qd, &mDebugQuery));

}
