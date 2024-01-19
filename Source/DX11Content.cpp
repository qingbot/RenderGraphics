#include"DX11Content.h"

D3D11App::D3D11App()
{

}

D3D11App::D3D11App(D3D11App&& other)
{

}

D3D11App::~D3D11App()
{

}

bool D3D11App::InitMainWindow()
{
	return false;
}

bool D3D11App::InitDirect3D()
{
	HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, D3D11_CREATE_DEVICE_DEBUG, nullptr, 0,
		D3D11_SDK_VERSION, &m_pDevice, &m_featureLevel, &m_pImmediateContext);
	return false;
}