/*
	FOR GETTING MORE INFORMATION ABOUT THIS CODE PLEASE CHECK http://directx11-1-gameprogramming.azurewebsites.net/ 
	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.
    Copyright (c) Microsoft Corporation. All rights reserved

	File Name        : ModelManager.h
	Generated by     : Pooya Eimandar (http://Pooya-Eimandar.com/)
	File Description : This class responsible for managing all models
 */
#pragma once

#include "Model.h"
#include <vector>

ref class ModelManager
{
internal:
	ModelManager();
	void LoadModel(Platform::String^ path);
	void UpdateActiveModel(DirectX::XMFLOAT3 transfome, float Yaw );
	void UpdateModels( float time );
	void RenderModels();
	void RenderModels(ID3D11DeviceContext1* context);
	void RenderModels(ID3D11DeviceContext1* context, int startIndex, int stopIndex);
	void Unload();	

	property UINT ActiveIndex
	{
		UINT get()
		{
			return this->activeIndex;
		}
		void set(UINT val)
		{
			this->activeIndex = val;
		}
	};
	property DirectX::XMFLOAT3 ActivePositionModel
	{
		DirectX::XMFLOAT3 get()
		{
			if(this->models.size() > 0)
			{
				return this->models[activeIndex]->Position;
			}
			return DirectX::XMFLOAT3();
		}
	};
	property float ActiveYawModel
	{
		float get()
		{
			if(this->models.size() > 0)
			{
				return this->models[activeIndex]->Rotation.y;
			}
			return 0;
		}
	};
	property UINT ActiveTotalTriangles
	{
		UINT get()
		{
			if(this->models.size() > 0)
			{
				return this->models[this->activeIndex]->Triangles;
			}
			return 0;
		}
	};

private:
	UINT activeIndex;
	std::vector<Model^> models;
	void RenderModel(Model^ model);
};
