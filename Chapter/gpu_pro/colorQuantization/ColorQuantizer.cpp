#include "ColorQuantizer.h"
#include "Effects.h"
#include "Vertex.h"

#define PALLETE_WIDTH 16
ColorQuantizer::ColorQuantizer()
	:mWidth(0)
	, mHeight(0)
	, mPallete0SRV(NULL)
	, mPallete0UAV(NULL)
	, mPallete0RTV(NULL)
	, mPallete1SRV(NULL)
	, mPallete1UAV(NULL)
	, mPallete1RTV(NULL)
	, mPalleteCountSRV(NULL)
	, mPalleteCountUAV(NULL)
	, mPalleteCountRTV(NULL)
{

}

ColorQuantizer::~ColorQuantizer()
{
	_ReleasePallete();
}

void ColorQuantizer::Init(ID3D11Device* device)
{
	_ReleasePallete();

	mPalleteViewport.TopLeftX = 0;
	mPalleteViewport.TopLeftY = 0;
	mPalleteViewport.Width = PALLETE_WIDTH;
	mPalleteViewport.Height = PALLETE_WIDTH;
	mPalleteViewport.MinDepth = 0;
	mPalleteViewport.MaxDepth = 0;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = PALLETE_WIDTH;
	texDesc.Height = PALLETE_WIDTH;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* texPalleteColor0 = NULL;
	HR(device->CreateTexture2D(&texDesc, NULL, &texPalleteColor0));
	ID3D11Texture2D* texPalleteColor1 = NULL;
	HR(device->CreateTexture2D(&texDesc, NULL, &texPalleteColor1));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	HR(device->CreateShaderResourceView(texPalleteColor0, &srvDesc, &mPallete0SRV));
	HR(device->CreateShaderResourceView(texPalleteColor1, &srvDesc, &mPallete1SRV));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = texDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	HR(device->CreateUnorderedAccessView(texPalleteColor0, &uavDesc, &mPallete0UAV));
	HR(device->CreateUnorderedAccessView(texPalleteColor1, &uavDesc, &mPallete1UAV));

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	HR(device->CreateRenderTargetView(texPalleteColor0, &rtvDesc, &mPallete0RTV));
	HR(device->CreateRenderTargetView(texPalleteColor1, &rtvDesc, &mPallete1RTV));

	ReleaseCOM(texPalleteColor0);
	ReleaseCOM(texPalleteColor1);

	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Format = texDesc.Format;
	uavDesc.Format = texDesc.Format;
	rtvDesc.Format = texDesc.Format;

	ID3D11Texture2D* texPalleteCount = NULL;
	HR(device->CreateTexture2D(&texDesc, NULL, &texPalleteCount));
	HR(device->CreateShaderResourceView(texPalleteCount, &srvDesc, &mPalleteCountSRV));
	HR(device->CreateUnorderedAccessView(texPalleteCount, &uavDesc, &mPalleteCountUAV));
	HR(device->CreateRenderTargetView(texPalleteCount, &rtvDesc, &mPalleteCountRTV));
	ReleaseCOM(texPalleteCount);
}

void ColorQuantizer::_ReleasePallete()
{
	ReleaseCOM(mPallete0SRV);
	ReleaseCOM(mPallete0UAV);
	ReleaseCOM(mPallete0RTV);
	ReleaseCOM(mPallete1SRV);
	ReleaseCOM(mPallete1UAV);
	ReleaseCOM(mPallete1RTV);
	ReleaseCOM(mPalleteCountSRV);
	ReleaseCOM(mPalleteCountUAV);
	ReleaseCOM(mPalleteCountRTV);
}
void ColorQuantizer::_SwitchTextures()
{
	MathHelper::Swap(mPallete0SRV, mPallete1SRV);
	MathHelper::Swap(mPallete0UAV, mPallete1UAV);
	MathHelper::Swap(mPallete0RTV, mPallete1RTV);
}
#define  MAX_ITERATION 256
inline int _CalcNeighborSize(int iter)
{
	/*if (iter >= 0 && iter < MAX_ITERATION / 16)
	{
		return 4;
	}
	if (iter >= MAX_ITERATION / 16 && iter < MAX_ITERATION / 8)
	{
		return 3;
	}
	if (iter >= MAX_ITERATION / 8 && iter < MAX_ITERATION / 4)
	{
		return 2;
	}
	if (iter >= MAX_ITERATION / 4 && iter < MAX_ITERATION / 2)
	{
		return 1;
	}*/
	if (iter >= 0 && iter < MAX_ITERATION / 8)
	{
		return 2;
	}
	if (iter >= MAX_ITERATION / 8 && iter < MAX_ITERATION / 2)
	{
		return 1;
	}
	return 0;
}
void ColorQuantizer::Quantize(ID3D11Device* device, ID3D11DeviceContext* dc, UINT width, UINT height, ID3D11ShaderResourceView * inputSRV, ID3D11UnorderedAccessView * outputUAV)
{
	//初始化
	ID3D11Buffer* vertexBuffer;
	Vertex::ColorQuantIter* vertices = new Vertex::ColorQuantIter[width*height];
	int idx = 0;
	for (int row = 0; row < height; ++row)
	{
		for (int col = 0; col < width; ++col)
		{
			vertices[idx].Pos.x = col;
			vertices[idx].Pos.y = row;
			++idx;
		}
	}
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::ColorQuantIter) * width * height;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vInitData; vInitData.pSysMem = vertices;
	HR(device->CreateBuffer(&vbd, &vInitData, &vertexBuffer));


	D3DX11_TECHNIQUE_DESC techDesc;
	Effects::ColorQuantPalleteFX->InitTech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		Effects::ColorQuantPalleteFX->SetPalleteTex(mPallete0UAV);
		Effects::ColorQuantPalleteFX->SetPalleteCountTex(mPalleteCountUAV);
		Effects::ColorQuantPalleteFX->InitTech->GetPassByIndex(p)->Apply(0, dc);

		dc->Dispatch(1, 1, 1);
	}
	Effects::ColorQuantPalleteFX->UnbindViews(dc);
	ID3D11RenderTargetView* nullRTV[2] = { NULL,NULL };
	//迭代
	D3D11_VIEWPORT oldViewport;
	UINT unused = 1;
	dc->RSGetViewports(&unused, &oldViewport);
	dc->RSSetViewports(1,&mPalleteViewport);
	dc->IASetInputLayout(InputLayouts::ColorQuantIter);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	UINT stride = sizeof(Vertex::ColorQuantIter);
	UINT offset = 0;
	dc->IASetVertexBuffers( 0, 1, &vertexBuffer, &stride, &offset );
	{
		ID3DX11EffectPass* pIterPass = Effects::ColorQuantIterateFX->IterateTech->GetPassByIndex(0);
		ID3DX11EffectPass* pCalcPass = Effects::ColorQuantPalleteFX->CalcColorTech->GetPassByIndex(0);
		for (int i = 0; i < MAX_ITERATION; ++i)
		{
			ID3D11RenderTargetView* rtvs[] = { mPallete1RTV, mPalleteCountRTV };
			dc->OMSetRenderTargets(2, rtvs, NULL);
			static const float transparent[4] = { 0.f,0.f,0.f,0.f };
			dc->ClearRenderTargetView(mPallete1RTV, transparent);
			int nNeighborSize = _CalcNeighborSize(i);
			Effects::ColorQuantIterateFX->SetPointSize(nNeighborSize);
			Effects::ColorQuantIterateFX->SetInputTex(inputSRV);
			Effects::ColorQuantIterateFX->SetPelleteTex(mPallete0SRV);
			pIterPass->Apply(0, dc);
			dc->Draw(width*height, 0);
			Effects::ColorQuantIterateFX->UnbindViews(dc);
			dc->OMSetRenderTargets(1, nullRTV, NULL);

			Effects::ColorQuantPalleteFX->SetPalleteTex(mPallete1UAV);
			Effects::ColorQuantPalleteFX->SetPalleteCountTex(mPalleteCountUAV);
			pCalcPass->Apply(0, dc);
			dc->Dispatch(1, 1, 1);
			Effects::ColorQuantIterateFX->UnbindViews(dc);
			_SwitchTextures();
		}
	}
	dc->RSSetViewports(1, &oldViewport);

	//输出
	Effects::ColorQuantOutputFX->OutputTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pPass = Effects::ColorQuantOutputFX->OutputTech->GetPassByIndex(0);
		Effects::ColorQuantOutputFX->SetInputTex(inputSRV);
		Effects::ColorQuantOutputFX->SetPelleteTex(mPallete0SRV);
		Effects::ColorQuantOutputFX->SetOutputTex(outputUAV);
		Effects::ColorQuantOutputFX->SetOutputPalleteUsedTex(mPalleteCountUAV);
		pPass->Apply(0, dc);
		dc->Dispatch((UINT)ceilf(width / 32.f), (UINT)ceilf(height / 32.f), 1);
		Effects::ColorQuantOutputFX->UnbindViews(dc);
	}
	ReleaseCOM(vertexBuffer);
	SafeDeleteArray(vertices);
}

