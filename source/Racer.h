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

    // -1 means this racer has not finished yet.
    float finishTime = -1.0f;

    b2Vec2 spawnPosition =
    {
        0.0f,
        0.0f
    };
};