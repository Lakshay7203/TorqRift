#pragma once

#include "Bike.h"
#include "Input.h"
#include "Terrain.h"

#include <vector>


enum class AIAction
{
    Racing,
    Wheelie,
    BunnyHop,
    FrontFlip,
    BackFlip,
    LandingRecovery
};


struct AIProfile
{
    // Racing performance.
    float speedMultiplier = 1.0f;

    // Stunt performance.
    float jumpStrength = 3.5f;
    float airControlMultiplier = 1.0f;

    // Behaviour.
    int stuntEveryNJumps = 3;

    bool prefersBackflip = false;

    float wheelieDuration = 0.6f;
    float wheelieInterval = 4.0f;

    // Higher = stronger invisible balance assistance.
    float balanceSkill = 1.0f;
};


struct AIState
{
    AIAction action =
        AIAction::Racing;

    bool initialized = false;

    bool wasGrounded = true;

    float actionTimer = 0.0f;

    float hopCooldown = 0.0f;
    float wheelieCooldown = 0.0f;

    int jumpCount = 0;

    float previousAngle = 0.0f;
    float accumulatedRotation = 0.0f;

    float wheelieTime = 0.0f;
    bool wheelieRewarded = false;

    float boostMeter = 0.0f;
    bool boostActive = false;
};


AIProfile GetAIProfile(
    int racerIndex
);


InputState BuildAIInput(
    const Bike& bike,
    bool bikeGrounded,
    bool raceFinished,
    int racerIndex,
    float deltaTime,
    AIState& state,
    const std::vector<TerrainSegment>& terrainSegments
);


void ApplyAIStabilityAssist(
    const Bike& bike,
    bool bikeGrounded,
    const AIProfile& profile,
    AIState& state
);