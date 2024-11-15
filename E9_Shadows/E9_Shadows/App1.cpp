// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	screenW = screenWidth;
	screenY = screenHeight;
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/teapot.obj"); 
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 4, screenHeight / 4, -screenWidth / 2.7, screenHeight / 2.7);
	sphereMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	lightSphereMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	light2SphereMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthTexture = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 2048;
	int shadowmapHeight = 2048;
	

	// This is your shadow map
	shadowMap[0] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	shadowMap[1] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	// Configure directional light
	lights[0] = new Light();
	lights[0]->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	lights[0]->setDiffuseColour(0.0f, 0.0f, 1.0f, 1.0f);
	lights[0]->setDirection(0.0f, -0.7f, 0.7f);
	lights[0]->setPosition(0.f, 0.f, -10.f);
	lights[0]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	lights[0]->generateProjectionMatrix(0.1f, 100.f);
		
		
	lights[1] = new Light();
	lights[1]->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	lights[1]->setDiffuseColour(1.0f, 0.0f, 0.0f, 1.0f);
	lights[1]->setDirection(0.0f, -0.7f, 0.7f);
	lights[1]->setPosition(0.f, 0.f, -10.f);
	lights[1]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	lights[1]->generateProjectionMatrix(0.1f, 100.f);


	sphereX = 0;
	sphereDir = true;
	
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}


bool App1::frame()
{
	bool result;
	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	lights[0]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	lights[1]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	// update light position
	lights[0]->setPosition(lightPosX, lightPosY, lightPosZ);
	if (lightDirX == 0) { lightDirX = 0.0001; }
	if (lightDirY == 0) { lightDirY = 0.0001; }
	if (lightDirZ == 0) { lightDirZ = 0.0001; }
	lights[0]->setDirection(lightDirX, lightDirY, lightDirZ);

	lights[1]->setPosition(lightPosX2, lightPosY2, lightPosZ2);
	if (lightDirX2 == 0) { lightDirX2 = 0.0001; }
	if (lightDirY2 == 0) { lightDirY2 = 0.0001; }
	if (lightDirZ2 == 0) { lightDirZ2 = 0.0001; }
	lights[1]->setDirection(lightDirX2, lightDirY2, lightDirZ2);

	//Move Sphere Mesh
	if (sphereDir == true)
	{
		sphereX += 0.05;
		if (sphereX > 20)
		{
			sphereDir = false;
		}
	}
	else
	{
		sphereX -= 0.05;
		if (sphereX < -20)
		{
			sphereDir = true;
		}
	}

	// Perform depth pass
	depthPass();
	// Render scene
	finalPass();

	return true;
}

void App1::depthPass()
{
	for (int i = 0; i < 2; i++)
	{
		// Set the render target to be the render to texture.
		shadowMap[i]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

		// get the world, view, and projection matrices from the camera and d3d objects.
		lights[i]->generateViewMatrix();
		XMMATRIX lightViewMatrix = lights[i]->getViewMatrix();
		XMMATRIX lightProjectionMatrix = lights[i]->getOrthoMatrix();
		//XMMATRIX lightProjectionMatrix = light->getProjectionMatrix();
		XMMATRIX worldMatrix = renderer->getWorldMatrix();

		worldMatrix = XMMatrixTranslation(-50.f, 0.f, -50.f);
		// Render floor
		mesh->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
		XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
		worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
		// Render model
		model->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

		// Render sphere
		worldMatrix = XMMatrixTranslation(sphereX, 5.0f, 20.0f);
		sphereMesh->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), sphereMesh->getIndexCount());

		depthMap[i] = shadowMap[i]->getDepthMapSRV();

		// Set back buffer as render target and reset view port.
		renderer->setBackBufferRenderTarget();
		renderer->resetViewport();
	}
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -50.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), depthMap, lights);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), depthMap, lights);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());
	
	//Render Sphere
	worldMatrix = XMMatrixTranslation(sphereX, 5.0f, 20.0f);
	sphereMesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), depthMap, lights);
	shadowShader->render(renderer->getDeviceContext(), sphereMesh->getIndexCount());

	//Render Light Sphere
	worldMatrix = XMMatrixTranslation(lightPosX, lightPosY, lightPosZ);
	scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	sphereMesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, nullptr, depthMap, lights);
	shadowShader->render(renderer->getDeviceContext(), sphereMesh->getIndexCount());

	//Render Light Sphere
	worldMatrix = XMMatrixTranslation(lightPosX2, lightPosY2, lightPosZ2);
	scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	sphereMesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, nullptr, depthMap, lights);
	shadowShader->render(renderer->getDeviceContext(), sphereMesh->getIndexCount());

	// RENDER THE RENDER TEXTURE SCENE
	// Requires 2D rendering and an ortho mesh.
	renderer->setZBuffer(false);
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, depthMap[0]);
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	orthoViewMatrix *= XMMatrixTranslation(screenW / 1.35, 0.0f, 0.0f);
	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, depthMap[1]);
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	gui();
	renderer->endScene();
}



void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	//Light Controls
	ImGui::SliderFloat("X", &lightPosX, -100, 100);
	ImGui::SliderFloat("Y", &lightPosY, -100, 100);
	ImGui::SliderFloat("Z", &lightPosZ, -100, 100);
	ImGui::SliderFloat("dirX", &lightDirX, -1, 1);
	ImGui::SliderFloat("dirY", &lightDirY, -1, 1);
	ImGui::SliderFloat("dirZ", &lightDirZ, -1, 1);

	ImGui::SliderFloat("X2", &lightPosX2, -100, 100);
	ImGui::SliderFloat("Y2", &lightPosY2, -100, 100);
	ImGui::SliderFloat("Z2", &lightPosZ2, -100, 100);
	ImGui::SliderFloat("dirX2", &lightDirX2, -1, 1);
	ImGui::SliderFloat("dirY2", &lightDirY2, -1, 1);
	ImGui::SliderFloat("dirZ2", &lightDirZ2, -1, 1);

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

