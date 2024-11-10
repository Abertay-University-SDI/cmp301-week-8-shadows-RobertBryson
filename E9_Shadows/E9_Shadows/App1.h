// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"

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

	Light* light;
	AModel* model;
	ShadowShader* shadowShader;
	DepthShader* depthShader;

	ShadowMap* shadowMap;

	float screenW;
	float screenY;

	float sphereX;
	bool sphereDir;


	float lightPosX;
	float lightPosY;
	float lightPosZ;
				   
	float lightDirX;
	float lightDirY;
	float lightDirZ;
};

#endif