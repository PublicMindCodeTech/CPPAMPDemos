/*
	FOR GETTING MORE INFORMATION ABOUT THIS CODE PLEASE CHECK http://directx11-1-gameprogramming.azurewebsites.net/
	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.
	Copyright (c) Microsoft Corporation. All rights reserved

	File Name        : Manager.cpp
	Generated by     : Pooya Eimandar (http://Pooya-Eimandar.com/)
	File Description : 
*/

#include "pch.h"
#include "FrameWork\DXHelper.h"
#include "..\Manager.h"

using namespace Concurrency;
using namespace Platform;
using namespace DirectX;
using namespace MathHelper;
using namespace std;

Manager::Manager() : activeIndex(0)
{
	this->xSound = ref new XSound();
	this->mediaEngine = ref new MediaEngine();
	mediaEngine->Load();
}

task<void> Manager::LoadModel(GraphicsDevice GDevice, String^ ModelPath)
{
	return LoadModel(GDevice, ModelPath, nullptr, nullptr, XMFLOAT3(0,0,0), XMFLOAT3(0,0,0));
}

task<void> Manager::LoadModel(GraphicsDevice GDevice, String^ ModelPath, String^ BumpMapPath ,String^ SpecMapPath,
			XMFLOAT3 InitialPosition, XMFLOAT3 InitialRotation)
{
	DXHelper::CheckPath(ModelPath);

	auto G3D = GDevice.G3D;
	//Load Default shader
	auto shader = ref new Shader(G3D, true);
	return shader->LoadAsync("DefaultVertexShader.cso", "DefaultPixelShader.cso", nullptr, nullptr,
		nullptr, VertexTypes::PositionNormalTangentColorTexture).then([=]()
	{
#pragma region Load Models

		FILE* f = nullptr;
		_wfopen_s(&f, ModelPath->Data(), L"rb"); 
		if (f == nullptr)
		{
			throw ref new Exception(0, "Could not load model on following path : " + ModelPath );
		}
		else
		{
			// How many models?
			UINT Lenght = 0;
			fread(&Lenght, sizeof(Lenght), 1, f);
			//Load each mesh
			for (UINT i = 0; i < Lenght; i++)
			{
				auto model = ref new Model();
				if(!BumpMapPath->IsEmpty())
				{
					model->BumpMapPath = BumpMapPath;
				}
				if(!SpecMapPath ->IsEmpty())
				{
					model->SpecMapPath = SpecMapPath;
				}
				model->Load(GDevice.G3D, shader, f);
				this->models.push_back(model);

				this->positions.push_back(InitialPosition);
				this->rotations.push_back(InitialRotation);
				this->infos.push_back(XMFLOAT3(MathHelper::Rand(100, 500), MathHelper::Rand(0, 360), MathHelper::Rand(0.005f, 0.015f)));
				this->worlds.push_back(XMFLOAT4X4());
			}
		}
		if (f != nullptr)
		{
			fclose(f);
		}

#pragma endregion				
	});
}

task<void> Manager::LoadBillboard(GraphicsDevice GDevice)
{
	auto billboard = ref new Billboard();
	return billboard->LoadAsync(GDevice).then([=]()
	{
		this->billboards.push_back(billboard);
	});
}

void Manager::LoadSound(String^ soundName)
{
	auto targetHitSound = this->mediaEngine->LoadSound(soundName);
	this->xSound->Load(this->mediaEngine->Engine, this->mediaEngine->Format, targetHitSound);
}

void Manager::PlaySound()
{
	this->xSound->Play(1);
}

void Manager::UpdateActiveModel(XMFLOAT3 transfome, float Yaw )
{
	if(this->models.size() > 0)
	{
		this->positions[activeIndex] = this->positions[activeIndex] + transfome;
		this->rotations[activeIndex].y = Yaw;
		this->models[activeIndex]->Color = XMFLOAT4(0.3f, 0, 0, 0);
	}
}

void Manager::UpdateModels( float time )
{
	int size = this->models.size();
	if (size == 0) return;

	array_view<XMFLOAT3, 1> p(size, this->positions);
	array_view<XMFLOAT3, 1> r(size, this->rotations);
	array_view<XMFLOAT3, 1> _infos(size, this->infos);
	array_view<XMFLOAT4X4, 1> w(size, this->worlds);    
	w.discard_data();

	#pragma region Create World Matrix

	parallel_for_each(w.extent, [=](index<1> i) restrict(amp)
	{
		using namespace Concurrency::fast_math;

		if (i[0] != 0)//If not player
		{
			//teta + velocity
			_infos[i].y = fmodf(_infos[i].y + _infos[i].z, 360);
			
			//Calculate position of each model
			p[i].z = _infos[i].x * sin(_infos[i].y);
			p[i].x = _infos[i].x * cos(_infos[i].y);

			//Apply rotation for each model
			r[i].y = _infos[i].y;
		}

		auto cosX = cos(r[i].x);
		auto sinX = sin(r[i].x);
		auto cosY = cos(r[i].y);
		auto sinY = sin(r[i].y);
		auto cosZ = cos(r[i].z);
		auto sinZ = sin(r[i].z);
		auto sinXY = sinX * sinY;
		auto cosXsinY = cosX * sinY;

		#pragma region Apply rotation
		
		w[i]._11 = cosY * cosZ;			
		w[i]._12 = -cosY * sinZ;
		w[i]._13 = sinY;
		
		w[i]._21 = sinXY * cosZ + cosX * sinZ;			
		w[i]._22 = -sinXY * sinZ + cosX * cosZ;
		w[i]._23 = -sinX * cosY;

		w[i]._31 = -cosXsinY * cosZ + sinX * sinZ;			
		w[i]._32 = cosXsinY * sinZ + sinX * cosZ;
		w[i]._33 = cosX * cosY;

		#pragma endregion

		#pragma region Apply position

		w[i]._14 = p[i].x;
		w[i]._24 = p[i].y;
		w[i]._34 = p[i].z;

		#pragma  endregion
		
		w[i]._41 = w[i]._42 = w[i]._43 = 0;
		w[i]._44 = 1;		
	});
	p.synchronize();
	w.synchronize();

	#pragma endregion

	#pragma region Check collisions

	const float debounce = 10.0f;
	float radius = this->models[0]->Radius;

	parallel_for_each(p.extent, [=](index<1> i) restrict(amp)
	{
		using namespace Concurrency::fast_math;

		if (i[0] == 0)//if is player
		{
			for ( int j = 1 ; j < size; j++)
			{
				// Check collision between player and others
				auto normalVectorX = p[i].x - p[j].x;
				auto normalVectorY = p[i].y - p[j].y;
				auto normalVectorZ = p[i].z - p[j].z;

				auto distance = fabsf(sqrt( powf(normalVectorX, 2) + powf(normalVectorY, 2) + powf(normalVectorZ, 2)));
				if (distance < (radius * 2) - debounce)
				{
					//Reset player position
					p[i].x = p[i].y = p[i].z = 0;
				}
			}
		}
	});
	p.synchronize();

	#pragma endregion
}

void Manager::RenderModels(GraphicsDevice GDevice)
{
	for (int i = 0; i < this->models.size(); i++)
	{
		this->models[i]->Render(GDevice, this->worlds[i]);
	}
}

void Manager::UpdateBillboards()
{
	std::for_each(this->billboards.begin(), this->billboards.end(), [&](Billboard^ b)
	{
		b->Update();		
	});
}

void Manager::RenderBillboards(GraphicsDevice GDevice, float Time)
{
	std::for_each(this->billboards.begin(), this->billboards.end(), [&](Billboard^ b)
	{
		b->Render(GDevice, Time);		
	});
}

void Manager::Unload()
{
	std::for_each(this->models.begin(), this->models.end(), [&](Model^ m)
	{
		m->Unload();		
	});
	std::for_each(this->billboards.begin(), this->billboards.end(), [&](Billboard^ b)
	{
		b->Unload();		
	});
	this->models.clear();
	this->positions.clear();
	this->rotations.clear();
	this->infos.clear();
	this->worlds.clear();

	std::vector<Billboard^> billboards;
}
