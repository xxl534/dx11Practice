#include "RenderStates.h"

ID3D11RasterizerState* RenderStates::WireframesRS = NULL;
ID3D11RasterizerState* RenderStates::NoCullRS = NULL;
ID3D11RasterizerState* RenderStates::CullClockwiseRS = NULL;

ID3D11BlendState* RenderStates::AlphaToCoverageBS = NULL;
ID3D11BlendState* RenderStates::TransparentBS = NULL;
ID3D11BlendState* RenderStates::NoRenderTargetWritesBS = NULL;
ID3D11BlendState* RenderStates::BlackAsTransparentBS = NULL;

ID3D11DepthStencilState* RenderStates::NoDepthWriteDSS = NULL;
ID3D11DepthStencilState* RenderStates::MarkMirrorDSS = NULL;
ID3D11DepthStencilState* RenderStates::DrawReflectionDSS = NULL;
ID3D11DepthStencilState* RenderStates::NoDoubleBlendDSS = NULL;
ID3D11DepthStencilState* RenderStates::LessEqualDSS = NULL;

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

	D3D11_RASTERIZER_DESC cullClockwiseDesc;
	ZeroMemory(&cullClockwiseDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
	cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
	cullClockwiseDesc.FrontCounterClockwise = TRUE;
	cullClockwiseDesc.DepthClipEnable = TRUE;
	HR(device->CreateRasterizerState(&cullClockwiseDesc, &CullClockwiseRS));

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

	D3D11_BLEND_DESC blackAsTransparentDesc = { 0 };
	blackAsTransparentDesc.AlphaToCoverageEnable = FALSE;
	blackAsTransparentDesc.IndependentBlendEnable = FALSE;
	blackAsTransparentDesc.RenderTarget[0].BlendEnable = TRUE;
	blackAsTransparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
	blackAsTransparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_COLOR;
	blackAsTransparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blackAsTransparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blackAsTransparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blackAsTransparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blackAsTransparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HR(device->CreateBlendState(&blackAsTransparentDesc, &BlackAsTransparentBS));

	D3D11_BLEND_DESC noRenderTargetWritesDesc = { 0 };
	noRenderTargetWritesDesc.AlphaToCoverageEnable = FALSE;
	noRenderTargetWritesDesc.IndependentBlendEnable = FALSE;
	noRenderTargetWritesDesc.RenderTarget[0].BlendEnable = FALSE;
	/*noRenderTargetWritesDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;*/
	noRenderTargetWritesDesc.RenderTarget[0].RenderTargetWriteMask = 0;
	HR(device->CreateBlendState(&noRenderTargetWritesDesc, &NoRenderTargetWritesBS));

	D3D11_DEPTH_STENCIL_DESC mirrorDesc;
	mirrorDesc.DepthEnable = TRUE;
	mirrorDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	mirrorDesc.DepthFunc = D3D11_COMPARISON_LESS;
	mirrorDesc.StencilEnable = TRUE;
	mirrorDesc.StencilReadMask = 0xff;
	mirrorDesc.StencilWriteMask = 0xff;
	mirrorDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	mirrorDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
	HR(device->CreateDepthStencilState(&mirrorDesc, &MarkMirrorDSS));

	D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
	drawReflectionDesc.DepthEnable = TRUE;
	drawReflectionDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	drawReflectionDesc.DepthFunc = D3D11_COMPARISON_LESS;
	drawReflectionDesc.StencilEnable = TRUE;
	drawReflectionDesc.StencilReadMask = 0xff;
	drawReflectionDesc.StencilWriteMask = 0xff;
	drawReflectionDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	drawReflectionDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
	HR(device->CreateDepthStencilState(&drawReflectionDesc, &DrawReflectionDSS));

	D3D11_DEPTH_STENCIL_DESC noDoubleBlendDesc;
	noDoubleBlendDesc.DepthEnable = TRUE;
	noDoubleBlendDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	noDoubleBlendDesc.DepthFunc = D3D11_COMPARISON_LESS;
	noDoubleBlendDesc.StencilEnable = TRUE;
	noDoubleBlendDesc.StencilReadMask = 0xff;
	noDoubleBlendDesc.StencilWriteMask = 0xff;
	noDoubleBlendDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDoubleBlendDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	noDoubleBlendDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
	HR(device->CreateDepthStencilState(&noDoubleBlendDesc, &NoDoubleBlendDSS));

	D3D11_DEPTH_STENCIL_DESC noDepthWriteDesc;
	noDepthWriteDesc.DepthEnable = TRUE;
	noDepthWriteDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	noDepthWriteDesc.DepthFunc = D3D11_COMPARISON_LESS;
	noDepthWriteDesc.StencilEnable = FALSE;
	noDepthWriteDesc.StencilReadMask = 0xff;
	noDepthWriteDesc.StencilWriteMask = 0xff;
	noDepthWriteDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDepthWriteDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDepthWriteDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	noDepthWriteDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NEVER;
	noDepthWriteDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDepthWriteDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDepthWriteDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	noDepthWriteDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
	HR(device->CreateDepthStencilState(&noDepthWriteDesc, &NoDepthWriteDSS));

	D3D11_DEPTH_STENCIL_DESC lessEqualDesc;
	lessEqualDesc.DepthEnable = TRUE;
	lessEqualDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	lessEqualDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	lessEqualDesc.StencilEnable = FALSE;
	HR(device->CreateDepthStencilState(&lessEqualDesc, &LessEqualDSS));
}

void 
RenderStates::DestroyAll()
{
	ReleaseCOM(WireframesRS);
	ReleaseCOM(NoCullRS);
	ReleaseCOM(CullClockwiseRS);

	ReleaseCOM(AlphaToCoverageBS);
	ReleaseCOM(TransparentBS);
	ReleaseCOM(NoRenderTargetWritesBS);
	ReleaseCOM(BlackAsTransparentBS);

	ReleaseCOM(MarkMirrorDSS);
	ReleaseCOM(DrawReflectionDSS);
	ReleaseCOM(NoRenderTargetWritesBS);
	ReleaseCOM(NoDepthWriteDSS);
	ReleaseCOM(LessEqualDSS);
}
