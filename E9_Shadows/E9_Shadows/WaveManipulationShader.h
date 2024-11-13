#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class WaveManipulationShader : public BaseShader
{
private:
	struct LightBufferType
	{
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float padding;
	};

	struct TimerBufferType
	{
		float time;
		float amplitude;
		float frequency;
		float speed;
	};

public:
	WaveManipulationShader(ID3D11Device* device, HWND hwnd);
	~WaveManipulationShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, Light* light, float tTime, float amp, float len, float spe);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* timerBuffer;


};