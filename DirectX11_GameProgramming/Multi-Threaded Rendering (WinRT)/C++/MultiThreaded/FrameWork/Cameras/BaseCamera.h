/*
	FOR GETTING MORE INFORMATION ABOUT THIS CODE PLEASE CHECK http://directx11-1-gameprogramming.azurewebsites.net/ 
	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.
    Copyright (c) Microsoft Corporation. All rights reserved

	File Name        : BaseCamera.h
	Generated by     : Pooya Eimandar (http://Pooya-Eimandar.com/)
	File Description : The base class for types of camera
 */

#pragma once

#include <DirectXMath.h>

ref class BaseCamera
{
internal:
	BaseCamera();
	void UpdateView();
	void UpdateProjection(float fieldOfView, float aspectRatio, float nearPlane, float farPlane );

	property DirectX::XMMATRIX View
	{
		DirectX::XMMATRIX get()
		{
			return this->view;
		}
	}
	property DirectX::XMMATRIX Projection
	{
		DirectX::XMMATRIX get()
		{
			return this->projection;
		}
	}
	property DirectX::XMFLOAT3 Position
	{
		DirectX::XMFLOAT3 get()
		{
			return this->position;
		}
	}

protected private:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 up;
	DirectX::XMFLOAT3 lookAt;
	DirectX::XMFLOAT3 direction;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
};