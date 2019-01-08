#pragma once


#include "d3dUtil.h"

class ColorQuantizer
{
public:
	ColorQuantizer();
	~ColorQuantizer();

	void Init(ID3D11Device* device);
	void Quantize(ID3D11Device* device, ID3D11DeviceContext* dc, UINT width, UINT height, ID3D11ShaderResourceView* inputSRV, ID3D11UnorderedAccessView* inputUAV);
	ID3D11ShaderResourceView* GetPalleteTexture() { return mPallete0SRV; }
	ID3D11ShaderResourceView* GetPalleteUsedTexture() { return mPalleteCountSRV; }
private:
	void _ReleasePallete();
	void _SwitchTextures();
private:
	UINT mWidth;
	UINT mHeight;
	D3D11_VIEWPORT mPalleteViewport;

	ID3D11ShaderResourceView* mPallete0SRV;
	ID3D11UnorderedAccessView* mPallete0UAV;
	ID3D11RenderTargetView* mPallete0RTV;
	ID3D11ShaderResourceView* mPallete1SRV;
	ID3D11UnorderedAccessView* mPallete1UAV;
	ID3D11RenderTargetView* mPallete1RTV;
	ID3D11ShaderResourceView* mPalleteCountSRV;
	ID3D11UnorderedAccessView* mPalleteCountUAV;
	ID3D11RenderTargetView* mPalleteCountRTV;

	ID3D11DepthStencilView* mDepthStencilView;
};