#pragma once
#include<d3d11.h>
#include<windows.h>

class D3D11App 
{
public:
	D3D11App();
	D3D11App(const D3D11App& other) = delete;
	D3D11App(D3D11App&& other);
	~D3D11App();


protected:
	
	ID3D11Device* m_pDevice;
	D3D_FEATURE_LEVEL m_featureLevel;
	ID3D11DeviceContext * m_pImmediateContext;

	bool InitMainWindow();
	bool InitDirect3D();


};