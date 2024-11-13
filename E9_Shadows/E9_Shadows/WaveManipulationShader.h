#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class WaveManipulationShader : public BaseShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView[2];
		XMMATRIX lightProjection[2];
	};

	struct lsLightStr
	{
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float padding;
	};

	struct LightBufferType
	{
		lsLightStr lsLights[2];
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

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMap[], Light* lights[], float tTime, float amp, float len, float spe);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* timerBuffer;


};