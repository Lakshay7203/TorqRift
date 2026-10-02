#pragma once

#include "Bike.h"
#include <box2d/box2d.h>


struct Racer
{
    Bike bike;

    bool isPlayer = false;

    bool grounded = true;

    bool finished = false;

    int currentPosition = 0;

    int finishPlace = 0;

    b2Vec2 spawnPosition =
    {
        0.0f,
        0.0f
    };
};