//
// Created by Viktor Brandmaier on 05.07.25.
//

#pragma once
#include "Entity.h"
#include <string>

class Enemy: public Entity{

public:

    float  setStartingPosition(float _dimension);
    explicit Enemy(const std::string& _texturePath);
    void move(float _rateX, float _rateY, unsigned _borderX, unsigned _borderY) override;

private:
    float m_speed;

};



