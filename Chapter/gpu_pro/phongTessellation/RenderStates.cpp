#include "RenderStates.h"

ID3D11RasterizerState* RenderStates::WireframesRS = NULL;
ID3D11RasterizerState* RenderStates::NoCullRS = NULL;
ID3D11RasterizerState* RenderStates::CullBackfaceRS = NULL;

ID3D11BlendState* RenderStates::AlphaToCoverageBS = NULL;
ID3D11BlendState* RenderStates::TransparentBS = NULL;

void 
RenderStates::InitAll(ID3D11Device* device)
{
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = FALSE;
	wireframeDesc.DepthClipEnable = true;
	HR(device->CreateRasterizerState(&wireframeDesc, &WireframesRS));

	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = FALSE;
	noCullDesc.DepthClipEnable = true;
	HR(device->CreateRasterizerState(&noCullDesc, &NoCullRS));

	D3D11_RASTERIZER_DESC cullBackfaceDesc;
	ZeroMemory(&cullBackfaceDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullBackfaceDesc.FillMode = D3D11_FILL_SOLID;
	cullBackfaceDesc.CullMode = D3D11_CULL_BACK;
	cullBackfaceDesc.FrontCounterClockwise = FALSE;
	cullBackfaceDesc.DepthClipEnable = TRUE;
	HR(device->CreateRasterizerState(&cullBackfaceDesc, &CullBackfaceRS));

	D3D11_BLEND_DESC alphaToCoverageDesc = { 0 };
	alphaToCoverageDesc.AlphaToCoverageEnable = TRUE;
	alphaToCoverageDesc.IndependentBlendEnable = FALSE;
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = FALSE;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HR(device->CreateBlendState(&alphaToCoverageDesc, &AlphaToCoverageBS));

	D3D11_BLEND_DESC transparentDesc = { 0 };
	transparentDesc.AlphaToCoverageEnable = FALSE;
	transparentDesc.IndependentBlendEnable = FALSE;
	transparentDesc.RenderTarget[0].BlendEnable = TRUE;
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HR(device->CreateBlendState(&transparentDesc, &TransparentBS));
}

void 
RenderStates::DestroyAll()
{
	ReleaseCOM(WireframesRS);
	ReleaseCOM(NoCullRS);
	ReleaseCOM(AlphaToCoverageBS);
	ReleaseCOM(TransparentBS);
}
