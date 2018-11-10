#include "Ssao.h"
#include "Camera.h"
#include "Effects.h"
#include "Vertex.h"

Ssao::Ssao(ID3D11Device* device, ID3D11DeviceContext* dc, int width, int height, float fovy, float farZ)
	:md3dDevice(device)
	, mDC(dc)
	, mScreenQuadVB(NULL)
	, mScreenQuadIB(NULL)
	, mRandomVectorSRV(NULL)
	, mNormalDepthRTV(NULL)
	, mNormalDepthSRV(NULL)
	, mAmbientRTV0(NULL)
	, mAmbientSRV0(NULL)
	, mAmbientRTV1(NULL)
	, mAmbientSRV1(NULL)
{
	OnSize(width, height, fovy, farZ);

	BuildFullScreenQuad();
	BuildOffsetVectors();
	BuildRandomVectorTexture();
}

Ssao::~Ssao()
{
	ReleaseCOM(mScreenQuadVB);
	ReleaseCOM(mScreenQuadIB);
	ReleaseCOM(mRandomVectorSRV);

	ReleaseTextureViews();
}

ID3D11ShaderResourceView* Ssao::NormalDepthSRV()
{
	return mNormalDepthSRV;
}

ID3D11ShaderResourceView* Ssao::AmbientSRV()
{
	return mAmbientSRV0;
}

void Ssao::OnSize(int width, int height, float fovy, float farZ)
{
	mRenderTargetWidth = width;
	mRenderTargetHeight = height;

	mAmbientMapViewport.TopLeftX = 0.f;
	mAmbientMapViewport.TopLeftY = 0.f;
	mAmbientMapViewport.Width = width / 2.f;
	mAmbientMapViewport.Height = height / 2.f;
	mAmbientMapViewport.MinDepth = 0.f;
	mAmbientMapViewport.MaxDepth = 1.f;

	BuildFrustumFarCorners(fovy, farZ);
	BuildTextureViews();
}

void Ssao::SetNormalDepthRenderTarget(ID3D11DepthStencilView* dsv)
{
	ID3D11RenderTargetView* renderTargets[1] = { mNormalDepthRTV };
	mDC->OMSetRenderTargets(1, renderTargets, dsv);

	// Clear view space normal to (0,0,-1) and clear depth to be very far away.  
	float clearColor[] = { 0.f, 0.f, -1.f, 1e5f };
	mDC->ClearRenderTargetView(mNormalDepthRTV, clearColor);
}

void Ssao::ComputeSsao(const Camera& camera)
{
	// Bind the ambient map as the render target.  Observe that this pass does not bind 
	// a depth/stencil buffer--it does not need it, and without one, no depth test is
	// performed, which is what we want.
	ID3D11RenderTargetView* renderTarget[1] = { mAmbientRTV0 };
	mDC->OMSetRenderTargets(1, renderTarget, NULL);
	mDC->ClearRenderTargetView(mAmbientRTV0, reinterpret_cast<const float*>(&Colors::Black));
	mDC->RSSetViewports(1, &mAmbientMapViewport);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	static const XMMATRIX t(
		0.5f, 0.f, 0.f, 0.f,
		0.f, -0.5f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.5f, 0.5f, 0.f, 1.f);

	XMMATRIX proj = camera.Proj();
	XMMATRIX projTex = proj * t;

	Effects::SsaoFX->SetViewToTexSpace(projTex);
	Effects::SsaoFX->SetOffsetVectors(mOffsets);
	Effects::SsaoFX->SetFrustumCorners(mFrustumFarCorner);
	Effects::SsaoFX->SetNormalDepthMap(mNormalDepthSRV);
	Effects::SsaoFX->SetRandomVecMap(mRandomVectorSRV);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	mDC->IASetInputLayout(InputLayouts::Basic32);
	mDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mDC->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
	mDC->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R16_UINT, 0);

	ID3DX11EffectTechnique* tech = Effects::SsaoFX->SsaoTech;
	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, mDC);
		mDC->DrawIndexed(6, 0, 0);
	}
}

void Ssao::BlurAmbientMap(int blurCount)
{
	for (int i = 0; i < blurCount; ++i)
	{
		// Ping-pong the two ambient map textures as we apply
		// horizontal and vertical blur passes.
		BlurAmbientMap(mAmbientSRV0, mAmbientRTV1, true);
		BlurAmbientMap(mAmbientSRV1, mAmbientRTV0, false);
	}
}

void Ssao::BlurAmbientMap(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV, bool horzBlur)
{
	ID3D11RenderTargetView* renderTargets[1] = { outputRTV };
	mDC->OMSetRenderTargets(1, renderTargets, NULL);
	mDC->ClearRenderTargetView(outputRTV, reinterpret_cast<const float*>(&Colors::Black));
	mDC->RSSetViewports(1, &mAmbientMapViewport);

	Effects::SsaoBlurFX->SetTexelWidth(1.f / mAmbientMapViewport.Width);
	Effects::SsaoBlurFX->SetTexelHeight(1.f / mAmbientMapViewport.Height);
	Effects::SsaoBlurFX->SetNormalDepthMap(mNormalDepthSRV);
	Effects::SsaoBlurFX->SetInputImage(inputSRV);

	ID3DX11EffectTechnique* tech;
	if(horzBlur)
	{
		tech = Effects::SsaoBlurFX->HorzBlurTech;
	}
	else
	{
		tech = Effects::SsaoBlurFX->VertBlurTech;
	}

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	mDC->IASetInputLayout(InputLayouts::Basic32);
	mDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mDC->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
	mDC->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R16_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, mDC);
		mDC->DrawIndexed(6, 0, 0);

		// Unbind the input SRV as it is going to be an output in the next blur.
		Effects::SsaoBlurFX->SetInputImage(0);
		tech->GetPassByIndex(p)->Apply(0, mDC);
	}
}

void Ssao::BuildFrustumFarCorners(float fovy, float farZ)
{
	float aspect = (float)mRenderTargetWidth / (float)mRenderTargetHeight;

	float halfHeight = farZ * tanf(0.5f*fovy);
}

void Ssao::BuildFullScreenQuad()
{

}

void Ssao::BuildTextureViews()
{

}

void Ssao::ReleaseTextureViews()
{

}

void Ssao::BuildRandomVectorTexture()
{

}

void Ssao::BuildOffsetVectors()
{

}

void Ssao::DrawFullScreenQuad()
{

}


