//
//  AnimatedGraphic.cpp
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 17/02/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#include "AnimatedGraphic.h"

using namespace std;

AnimatedGraphic::AnimatedGraphic() : PlatformerObject()
{
    
}

void AnimatedGraphic::Load(LoaderParams& params)
{
	PlatformerObject::Load(params);
	m_animSpeed = params.getAnimSpeed();
}

void AnimatedGraphic::Draw()
{
    PlatformerObject::Draw();
}

void AnimatedGraphic::Update()
{
    m_currentFrame = int(((SDL_GetTicks() / (1000 / m_animSpeed)) % m_numFrames));
}

void AnimatedGraphic::Clean()
{
    PlatformerObject::Clean();
}