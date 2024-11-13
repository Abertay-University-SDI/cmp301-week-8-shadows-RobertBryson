// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"
#include "WaveManipulationShader.h"


class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void depthPass();
	void finalPass();
	void gui();

private:
	TextureShader* textureShader;
	TextureShader* depthTexture;
	PlaneMesh* mesh;
	OrthoMesh* orthoMesh;
	SphereMesh* sphereMesh;
	SphereMesh* lightSphereMesh;
	SphereMesh* light2SphereMesh;

	Light* lights[2];
	AModel* model;
	ShadowShader* shadowShader;
	DepthShader* depthShader;

	ShadowMap* shadowMap[2];
	ID3D11ShaderResourceView* depthMap[2];

	float screenW;
	float screenY;

	float sphereX;
	bool sphereDir;


	float lightPosX = 15;
	float lightPosY = 10;
	float lightPosZ = -15;
				   
	float lightDirX = -0.6;
	float lightDirY = -0.15;
	float lightDirZ = 0.25;

	float lightPosX2;
	float lightPosY2 = 10;
	float lightPosZ2 = 15;
				   
	float lightDirX2;
	float lightDirY2 = -0.15;
	float lightDirZ2 = -0.25;

	int sceneWidth = 100;
	int sceneHeight = 100;

	//wave
	WaveManipulationShader* waveShader;

	Timer* timer;

	float time = 0;
	float amplitude = 0.75;
	float waveLength = 2;
	float speed = 1;
};

#endif