#include "d3dApp.h"
#include "Waves.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "GeometryGenerator.h"
#include "ColorQuantizer.h"

class ColorQuantazitionApp :public D3DApp
{
public:
	ColorQuantazitionApp(HINSTANCE hInstance);
	~ColorQuantazitionApp();

	bool Init();
	void DrawScene();
	virtual void UpdateScene(float dt);
private:
	void BuildQuadBuffer();
	void BuildDestViews();
private:
	ID3D11Buffer* mQuadIB;
	ID3D11Buffer* mSrcQuadVB;
	ID3D11Buffer* mDestQuadVB;
	ID3D11Buffer* mPalleteVB;
	ID3D11Buffer* mPalleteUsedVB;

	ID3D11ShaderResourceView*	mSrcSRV;

	ID3D11ShaderResourceView*	mDestSRV;
	ID3D11UnorderedAccessView*	mDestUAV;

	ColorQuantizer* mQuanter;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	ColorQuantazitionApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

ColorQuantazitionApp::ColorQuantazitionApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
	, mSrcQuadVB(NULL)
	, mQuadIB(NULL)
	, mDestQuadVB(NULL)
	, mPalleteUsedVB(NULL)
	, mSrcSRV(NULL)
	, mDestSRV(NULL)
	, mDestUAV(NULL)
	,mPalleteVB(NULL)
	,mQuanter(NULL)
{
	mMainWndCaption = L"Color Quantization";
	mEnable4xMsaa = false;
}

ColorQuantazitionApp::~ColorQuantazitionApp()
{
	md3dImmediateContext->ClearState();
	ReleaseCOM(mSrcQuadVB);
	ReleaseCOM(mQuadIB);
	ReleaseCOM(mDestQuadVB);
	ReleaseCOM(mPalleteUsedVB);
	ReleaseCOM(mPalleteVB);
	ReleaseCOM(mSrcSRV);
	ReleaseCOM(mDestSRV);
	ReleaseCOM(mDestUAV);

	SafeDelete(mQuanter);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool ColorQuantazitionApp::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}

	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	/*HR(CreateDDSTextureFromFile(md3dDevice,
		L"Textures/bricks.dds", NULL, &mSrcSRV));*/
	HR(CreateWICTextureFromFile(md3dDevice,
		L"Textures/ColorQuantization0.jpg", NULL, &mSrcSRV));

	BuildDestViews();
	BuildQuadBuffer();
	return true;
}

void ColorQuantazitionApp::DrawScene()
{
	ID3D11RenderTargetView* renderTargets[1] = { mRenderTargetView };
	md3dImmediateContext->OMSetRenderTargets(1, renderTargets, NULL);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	//md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX identity = XMMatrixIdentity();
	ID3DX11EffectTechnique* tech = Effects::BasicFX->Light0TexTech;
	float blendFactor[] = { 0.f, 0.f, 0.f, 0.f };
	md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mSrcQuadVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mQuadIB, DXGI_FORMAT_R32_UINT, 0);

		Effects::BasicFX->SetWorld(identity);
		Effects::BasicFX->SetWorldInvTranspose(identity);
		Effects::BasicFX->SetWorldViewProj(identity);
		Effects::BasicFX->SetTexTransform(identity);
		Effects::BasicFX->SetDiffuseMap(mSrcSRV);

		tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(6, 0, 0);
	}

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mDestQuadVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mQuadIB, DXGI_FORMAT_R32_UINT, 0);

		Effects::BasicFX->SetWorld(identity);
		Effects::BasicFX->SetWorldInvTranspose(identity);
		Effects::BasicFX->SetWorldViewProj(identity);
		Effects::BasicFX->SetTexTransform(identity);
		Effects::BasicFX->SetDiffuseMap(mDestSRV);

		tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(6, 0, 0);
	}

	tech = Effects::BasicFX->PointTexTech;
	//tech = Effects::BasicFX->Light0TexTech;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mPalleteVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mQuadIB, DXGI_FORMAT_R32_UINT, 0);

		Effects::BasicFX->SetWorld(identity);
		Effects::BasicFX->SetWorldInvTranspose(identity);
		Effects::BasicFX->SetWorldViewProj(identity);
		Effects::BasicFX->SetTexTransform(identity);
		Effects::BasicFX->SetDiffuseMap(mQuanter->GetPalleteTexture());

		tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(6, 0, 0);
	}

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mPalleteUsedVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mQuadIB, DXGI_FORMAT_R32_UINT, 0);

		Effects::BasicFX->SetWorld(identity);
		Effects::BasicFX->SetWorldInvTranspose(identity);
		Effects::BasicFX->SetWorldViewProj(identity);
		Effects::BasicFX->SetTexTransform(identity);
		Effects::BasicFX->SetDiffuseMap(mQuanter->GetPalleteUsedTexture());

		tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(6, 0, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

void ColorQuantazitionApp::UpdateScene(float dt)
{

}

void ColorQuantazitionApp::BuildQuadBuffer()
{
	std::vector<Vertex::Basic32> vertices(4);
	vertices[0].Normal = XMFLOAT3(0.f, 0.f, -1.f);
	vertices[0].Tex = XMFLOAT2(0.f, 0.f);

	vertices[1].Normal = XMFLOAT3(0.f, 0.f, -1.f);
	vertices[1].Tex = XMFLOAT2(1.f, 0.f);

	vertices[2].Normal = XMFLOAT3(0.f, 0.f, -1.f);
	vertices[2].Tex = XMFLOAT2(0.f, 1.f);

	vertices[3].Normal = XMFLOAT3(0.f, 0.f, -1.f);
	vertices[3].Tex = XMFLOAT2(1.f, 1.f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vertices.size();
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = &vertices[0];

	vertices[0].Pos = XMFLOAT3(-1.f, 1.f, 0.f);
	vertices[1].Pos = XMFLOAT3(1.f, 1.f, 0.f);
	vertices[2].Pos = XMFLOAT3(-1.f, 0.f, 0.f);
	vertices[3].Pos = XMFLOAT3(1.f, 0.f, 0.f);

	HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mSrcQuadVB));

	vertices[0].Pos = XMFLOAT3(-1.f, 0.f, 0.f);
	vertices[1].Pos = XMFLOAT3(1.f, 0.f, 0.f);
	vertices[2].Pos = XMFLOAT3(-1.f, -1.f, 0.f);
	vertices[3].Pos = XMFLOAT3(1.f, -1.f, 0.f);

	HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mDestQuadVB));

	vertices[0].Pos = XMFLOAT3(-0.2f, 0.1f, 0.f);
	vertices[1].Pos = XMFLOAT3(0.0f, 0.1f, 0.f);
	vertices[2].Pos = XMFLOAT3(-0.2f, -0.1f, 0.f);
	vertices[3].Pos = XMFLOAT3(0.0f, -0.1f, 0.f);
	HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mPalleteVB)); 

	vertices[0].Pos = XMFLOAT3(0.0f, 0.1f, 0.f);
	vertices[1].Pos = XMFLOAT3(0.2f, 0.1f, 0.f);
	vertices[2].Pos = XMFLOAT3(0.0f, -0.1f, 0.f);
	vertices[3].Pos = XMFLOAT3(0.2f, -0.1f, 0.f);
	HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mPalleteUsedVB));
	std::vector<UINT> indices(6);
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;

	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 3;

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mQuadIB));
}

void ColorQuantazitionApp::BuildDestViews()
{
	ID3D11Texture2D* pSrcTex;
	mSrcSRV->GetResource((ID3D11Resource**)&pSrcTex);
	D3D11_TEXTURE2D_DESC texDesc;
	pSrcTex->GetDesc(&texDesc);
	
	D3D11_TEXTURE2D_DESC destTexDesc;
	destTexDesc.Width = texDesc.Width;
	destTexDesc.Height = texDesc.Height;
	destTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	destTexDesc.SampleDesc.Quality = 0;
	destTexDesc.SampleDesc.Count = 1;
	destTexDesc.MipLevels = 1;
	destTexDesc.ArraySize = 1;
	destTexDesc.Usage = D3D11_USAGE_DEFAULT;
	destTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	destTexDesc.CPUAccessFlags = 0;
	destTexDesc.MiscFlags = 0;
	ID3D11Texture2D* pDestTex;
	md3dDevice->CreateTexture2D(&destTexDesc, NULL, &pDestTex);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = destTexDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	HR(md3dDevice->CreateShaderResourceView(pDestTex, &srvDesc, &mDestSRV));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = destTexDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	HR(md3dDevice->CreateUnorderedAccessView(pDestTex, &uavDesc, &mDestUAV));

	ReleaseCOM(pDestTex);

	mQuanter = new ColorQuantizer();
	mQuanter->Init( md3dDevice );
	mQuanter->Quantize(md3dDevice, md3dImmediateContext, texDesc.Width, texDesc.Height, mSrcSRV, mDestUAV);
}
