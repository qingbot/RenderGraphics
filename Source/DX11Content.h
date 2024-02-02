//#pragma once
//#include<d3d11.h>
//#include<windows.h>
//#include<wrl.h>
//
//template<class T>
//using ComPtr = Microsoft::WRL::ComPtr<T>;
//
//class D3D11App 
//{
//public:
//	D3D11App();
//	D3D11App(const D3D11App& other) = delete;
//	D3D11App(D3D11App&& other);
//	~D3D11App();
//
//	void InitWindow();
//
//protected:
//
//	DXGI_FORMAT RTFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
//
//	UINT m_msaaQuality = 1;
//
//	HINSTANCE m_hAppInst;
//	ComPtr<ID3D11Device> m_pDevice;
//	D3D_FEATURE_LEVEL m_featureLevel;
//	ComPtr<ID3D11DeviceContext> m_pImmediateContext;
//
//	bool InitMainWindow();
//	bool InitDirect3D();
//
//public:
//	UINT GetMSAAQuality() const { return m_msaaQuality; }
//
//}; 