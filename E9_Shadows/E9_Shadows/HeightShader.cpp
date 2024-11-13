#include "HeightShader.h"

HeightShader::HeightShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"height_vs.cso", L"height_ps.cso");
}


HeightShader::~HeightShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the light constant buffer.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}


	//Release base shader components
	BaseShader::~BaseShader();
}

void HeightShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC lightBufferDesc;


	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	// Setup light buffer
	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	// Sampler for shadow map sampling.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	renderer->CreateSamplerState(&samplerDesc, &sampleStateShadow);


	//timer
	D3D11_BUFFER_DESC timerBufferDesc;
	timerBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	timerBufferDesc.ByteWidth = sizeof(TimerBufferType);
	timerBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	timerBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	timerBufferDesc.MiscFlags = 0;
	timerBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&timerBufferDesc, NULL, &timerBuffer);
}


void HeightShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMap[], Light* lights[], float height)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	// Transpose the matrices to prepare them for the shader.
	// Lock the constant buffer so it can be written to.
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	for (int i = 0; i < 2; i++)
	{
		XMMATRIX tLightViewMatrix = XMMatrixTranspose(lights[i]->getViewMatrix());
		XMMATRIX tLightProjectionMatrix = XMMatrixTranspose(lights[i]->getOrthoMatrix());

		dataPtr->lightView[i] = tLightViewMatrix;
		dataPtr->lightProjection[i] = tLightProjectionMatrix;
	}
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);


	//Timer
	TimerBufferType* timerPtr;
	deviceContext->Map(timerBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	timerPtr = (TimerBufferType*)mappedResource.pData;
	timerPtr->height = height;
	deviceContext->Unmap(timerBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &timerBuffer);

	//Lights
	LightBufferType* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	for (int i = 0; i < 2; i++)
	{
		lightPtr->lsLights[i].ambient = lights[i]->getAmbientColour();
		lightPtr->lsLights[i].diffuse = lights[i]->getDiffuseColour();
		lightPtr->lsLights[i].direction = lights[i]->getDirection();
		lightPtr->lsLights[i].padding = 0.0f;
	}
	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 2, depthMap);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(1, 1, &sampleStateShadow);

	deviceContext->VSSetShaderResources(0, 1, &texture);
	deviceContext->VSSetSamplers(0, 1, &sampleState);

	//HRESULT result;
	//D3D11_MAPPED_SUBRESOURCE mappedResource;
	//MatrixBufferType* dataPtr;

	//XMMATRIX tworld, tview, tproj;

	//// Transpose the matrices to prepare them for the shader.
	//tworld = XMMatrixTranspose(worldMatrix);
	//tview = XMMatrixTranspose(viewMatrix);
	//tproj = XMMatrixTranspose(projectionMatrix);
	//result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//dataPtr = (MatrixBufferType*)mappedResource.pData;
	//dataPtr->world = tworld;// worldMatrix;
	//dataPtr->view = tview;
	//dataPtr->projection = tproj;
	//deviceContext->Unmap(matrixBuffer, 0);
	//deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	////Additional
	//// Send light data to pixel shader
	//LightBufferType* lightPtr;
	//deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//lightPtr = (LightBufferType*)mappedResource.pData;
	//lightPtr->diffuse = light->getDiffuseColour();
	//lightPtr->direction = light->getDirection();
	//lightPtr->padding = 0.0f;
	//deviceContext->Unmap(lightBuffer, 0);
	//deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	//// Set shader texture resource in the pixel shader.
	//deviceContext->PSSetShaderResources(0, 1, &texture);
	//deviceContext->PSSetSamplers(0, 1, &sampleState);
	//deviceContext->VSSetShaderResources(0, 1, &texture);
	//deviceContext->VSSetSamplers(0, 1, &sampleState);
}