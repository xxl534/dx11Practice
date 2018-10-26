#include "d3dApp.h"
#include "Waves.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "DDSTextureLoader.h"
#include "GeometryGenerator.h"

struct Data
{
	XMFLOAT3 v1;
	XMFLOAT2 v2;
};

class ComputeApp :public D3DApp
{
public:
	ComputeApp(HINSTANCE hInstance);
	~ComputeApp();

	bool Init();
	void UpdateScene(float dt);
	void DrawScene();

	void DoComputeWork();

private:
	void BuildBuffersAndViews();
private:
	ID3D11Buffer* mOutputBuffer;
	ID3D11Buffer* mOutputDebugBuffer;
	
	ID3D11ShaderResourceView* mInputASRV;
	ID3D11ShaderResourceView* mInputBSRV;
	ID3D11UnorderedAccessView* mOutputUAV;

	UINT mNumElements;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	ComputeApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	theApp.DoComputeWork();
	return 0;
}

ComputeApp::ComputeApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
	,mOutputBuffer(NULL)
	,mOutputDebugBuffer(NULL)
	,mInputASRV(NULL)
	,mInputBSRV(NULL)
	,mOutputUAV(NULL)
	,mNumElements(32)
{
	mMainWndCaption = L"Compute Demo";
	mEnable4xMsaa = false;
}

ComputeApp::~ComputeApp()
{
	md3dImmediateContext->ClearState();
	ReleaseCOM(mOutputBuffer);
	ReleaseCOM(mOutputDebugBuffer);
	ReleaseCOM(mInputASRV);
	ReleaseCOM(mInputBSRV);
	ReleaseCOM(mOutputUAV);
	
	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool ComputeApp::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}

	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	BuildBuffersAndViews();
	return true;
}

void ComputeApp::UpdateScene(float dt)
{

}

void ComputeApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	HR(mSwapChain->Present(0, 0));
}

void ComputeApp::DoComputeWork()
{
	D3DX11_TECHNIQUE_DESC techDesc;

	Effects::VecAddFX->SetInputA(mInputASRV);
	Effects::VecAddFX->SetInputB(mInputBSRV);
	Effects::VecAddFX->SetOutput(mOutputUAV);

	Effects::VecAddFX->VecAddTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = Effects::VecAddFX->VecAddTech->GetPassByIndex(p);
		pass->Apply(0, md3dImmediateContext);
		md3dImmediateContext->Dispatch(1, 1, 1);
	}

	ID3D11ShaderResourceView* nullSRV[2] = { NULL };
	md3dImmediateContext->CSSetShaderResources(0, 2, nullSRV);

	ID3D11UnorderedAccessView* nullUAV[1] = { NULL };
	md3dImmediateContext->CSSetUnorderedAccessViews(0, 1, nullUAV, NULL);

	md3dImmediateContext->CSSetShader(NULL, NULL, 0);

	std::ofstream fout("results.txt");

	md3dImmediateContext->CopyResource(mOutputDebugBuffer, mOutputBuffer);
	D3D11_MAPPED_SUBRESOURCE mappedData;
	md3dImmediateContext->Map(mOutputDebugBuffer, 0, D3D11_MAP_READ, 0, &mappedData);
	
	Data* dataView = reinterpret_cast<Data*>(mappedData.pData);
	for (int i = 0; i < mNumElements; ++i)
	{
		fout << "(" << dataView[i].v1.x << ", " << dataView[i].v1.y << ", " << dataView[i].v1.z <<
			", " << dataView[i].v2.x << ", " << dataView[i].v2.y << ")" << std::endl;
	}

	md3dImmediateContext->Unmap(mOutputDebugBuffer, 0);

	fout.close();
}

void ComputeApp::BuildBuffersAndViews()
{
	std::vector<Data> dataA(mNumElements);
	std::vector<Data> dataB(mNumElements);
	for (int i = 0; i < mNumElements; ++i)
	{
		dataA[i].v1 = XMFLOAT3(i, i, i);
		dataA[i].v2 = XMFLOAT2(i, 0);

		dataB[i].v1 = XMFLOAT3(-i, i, 0.f);
		dataB[i].v2 = XMFLOAT2(0.f, -i);
	}

	D3D11_BUFFER_DESC inputDesc;
	inputDesc.Usage = D3D11_USAGE_DEFAULT;
	inputDesc.ByteWidth = sizeof(Data)*mNumElements;
	inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	inputDesc.CPUAccessFlags = 0;
	inputDesc.StructureByteStride = sizeof(Data);
	inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA vInitDataA;
	vInitDataA.pSysMem = &dataA[0];

	ID3D11Buffer* bufferA = NULL;
	HR(md3dDevice->CreateBuffer(&inputDesc, &vInitDataA, &bufferA));

	D3D11_SUBRESOURCE_DATA vInitDataB;
	vInitDataB.pSysMem = &dataB[0];
	ID3D11Buffer* bufferB = NULL;
	HR(md3dDevice->CreateBuffer(&inputDesc, &vInitDataB, &bufferB));

	D3D11_BUFFER_DESC outputDesc;
	outputDesc.Usage = D3D11_USAGE_DEFAULT;
	outputDesc.ByteWidth = sizeof(Data)*mNumElements;
	outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	outputDesc.CPUAccessFlags = 0;
	outputDesc.StructureByteStride = sizeof(Data);
	outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	HR(md3dDevice->CreateBuffer(&outputDesc, NULL, &mOutputBuffer));

	outputDesc.Usage = D3D11_USAGE_STAGING;
	outputDesc.BindFlags = 0;
	outputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	HR(md3dDevice->CreateBuffer(&outputDesc, NULL, &mOutputDebugBuffer));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.Flags = 0;
	srvDesc.BufferEx.NumElements = mNumElements;

	md3dDevice->CreateShaderResourceView(bufferA, &srvDesc, &mInputASRV);
	md3dDevice->CreateShaderResourceView(bufferB, &srvDesc, &mInputBSRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = 0;
	uavDesc.Buffer.NumElements = mNumElements;
	
	md3dDevice->CreateUnorderedAccessView(mOutputBuffer, &uavDesc, &mOutputUAV);

	ReleaseCOM(bufferA);
	ReleaseCOM(bufferB);
}
