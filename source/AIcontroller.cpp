#include "AIController.h"

#include <algorithm>
#include <cmath>
#include <SDL3/SDL.h>


// =====================================================
// RIDER PROFILES
// =====================================================

AIProfile GetAIProfile(
    int racerIndex)
{
    // =====================================================
    // AI 1 - FAST / CLEAN
    // =====================================================

    if (racerIndex == 0)
    {
        return
        {
            1.10f,     // speed
            4.2f,      // jump strength
            1.10f,     // air control
            4,         // stunt every 4 jumps
            false,     // front flip
            0.50f,     // wheelie duration
            4.8f,      // wheelie interval
            1.25f      // balance
        };
    }


    // ============================================
// AI 2 - GRAVEL
// Steady climber, safer rider
// ============================================

    if (racerIndex == 1)
    {
        return {
            0.98f, // Speed
            3.9f,  // Jump strength
            0.92f, // Air control
            5,     // Stunt frequency (lower)
            false, // Prefers front flips
            0.35f, // Wheelie duration
            6.0f,  // Wheelie interval
            1.65f  // Strong balance
        };
    }

    // ============================================
    // AI 3 - RIOT
    // Wild, risky stunt racer
    // ============================================

    return {
        1.16f, // Speed
        5.1f,  // Jump strength
        1.60f, // Air control
        1,     // Frequent stunts
        true,  // Prefers backflips
        0.85f, // Wheelie duration
        2.5f,  // Wheelie interval
        0.95f  // Less balance
    };
}


// =====================================================
// TERRAIN SLOPE
// =====================================================

static float GetTerrainSlopeAtX(
    const std::vector<TerrainSegment>& terrainSegments,
    float worldX)
{
    for (const TerrainSegment& segment : terrainSegments)
    {
        float minimumX =
            std::min(
                segment.start.x,
                segment.end.x
            );

        float maximumX =
            std::max(
                segment.start.x,
                segment.end.x
            );


        if (worldX >= minimumX &&
            worldX <= maximumX)
        {
            float deltaX =
                segment.end.x -
                segment.start.x;

            float deltaY =
                segment.end.y -
                segment.start.y;


            if (std::abs(deltaX) >
                0.001f)
            {
                return deltaY /
                    deltaX;
            }
        }
    }


    return 0.0f;
}


// =====================================================
// SHARED AI BRAIN
// =====================================================

InputState BuildAIInput(
    const Bike& bike,
    bool bikeGrounded,
    bool raceFinished,
    int racerIndex,
    float deltaTime,
    AIState& state,
    const std::vector<TerrainSegment>& terrainSegments)
{
    InputState input;


    if (raceFinished)
    {
        return input;
    }


    AIProfile profile =
        GetAIProfile(
            racerIndex
        );


    // =================================================
    // INITIAL STATE
    // =================================================

    if (!state.initialized)
    {
        state.initialized = true;

        state.lastProgressX =
            b2Body_GetPosition(bike.chassisBodyId).x;

        // Prevent all racers doing the same thing
        // at exactly the same time.
        state.wheelieCooldown =
            1.5f +
            static_cast<float>(racerIndex) *
            0.8f;
    }


    input.driveForward = true;


    b2Vec2 position =
        b2Body_GetPosition(
            bike.chassisBodyId
        );


    b2Vec2 velocity =
        b2Body_GetLinearVelocity(
            bike.chassisBodyId
        );


    float angle =
        b2Rot_GetAngle(
            b2Body_GetRotation(
                bike.chassisBodyId
            )
        );


    // =====================================================
    // AI STUCK DETECTION
    // =====================================================

    constexpr float MIN_PROGRESS = 0.75f;
    constexpr float STUCK_TIME = 1.5f;

    if (!bikeGrounded)
    {
        // Don't classify normal jumps as being stuck.
        state.lastProgressX = position.x;
        state.stuckTimer = 0.0f;
        state.isStuck = false;
    }
    else
    {
        float progress =
            position.x - state.lastProgressX;

        if (progress >= MIN_PROGRESS)
        {
            // Racer has moved forward enough.
            state.lastProgressX = position.x;
            state.stuckTimer = 0.0f;
            state.isStuck = false;
        }
        else
        {
            // Not enough forward progress yet.
            state.stuckTimer += deltaTime;

            if (state.stuckTimer >= STUCK_TIME &&
                !state.isStuck)
            {
                state.isStuck = true;

                SDL_Log(
                    "AI %d STUCK at X: %.2f",
                    racerIndex + 1,
                    position.x
                );
            }
        }
    }


    // =================================================
    // TIMERS
    // =================================================

    state.actionTimer =
        std::max(
            0.0f,
            state.actionTimer - deltaTime
        );


    state.hopCooldown =
        std::max(
            0.0f,
            state.hopCooldown - deltaTime
        );


    state.wheelieCooldown =
        std::max(
            0.0f,
            state.wheelieCooldown - deltaTime
        );


    // =================================================
    // AI STUCK RECOVERY
    // =================================================

    if (state.isStuck &&
        bikeGrounded &&
        state.hopCooldown <= 0.0f)
    {
        // Attempt to jump over the obstacle.
        input.jumpPressed = true;

        // Keep driving forward.
        input.driveForward = true;

        // Lean forward to help control the bike.
        input.leanForward = true;

        // Avoid continuously attempting recovery.
        state.hopCooldown = 2.5f;

        // Restart stuck detection.
        state.stuckTimer = 0.0f;
        state.lastProgressX = position.x;
        state.isStuck = false;

        state.action = AIAction::BunnyHop;
        state.wasGrounded = bikeGrounded;

        SDL_Log(
            "AI %d RECOVERY HOP at X: %.2f",
            racerIndex + 1,
            position.x
        );

        return input;
    }


    // =================================================
    // TAKEOFF / LANDING
    // =================================================

    bool justLeftGround =
        state.wasGrounded &&
        !bikeGrounded;


    bool justLanded =
        !state.wasGrounded &&
        bikeGrounded;


    if (justLeftGround)
    {
        ++state.jumpCount;

        state.previousAngle =
            angle;

        state.accumulatedRotation =
            0.0f;


        // Only stunt when this is a REAL jump.
        bool enoughAirForStunt =
            velocity.y > 2.8f;


        bool shouldStunt =
            enoughAirForStunt &&
            profile.stuntEveryNJumps > 0 &&
            state.jumpCount %
            profile.stuntEveryNJumps == 0;


        if (shouldStunt)
        {
            state.action =
                profile.prefersBackflip
                ? AIAction::BackFlip
                : AIAction::FrontFlip;
        }
        else
        {
            state.action =
                AIAction::LandingRecovery;
        }
    }


    // =================================================
    // ROTATION TRACKING
    // =================================================

    if (!bikeGrounded)
    {
        float angleDifference =
            angle -
            state.previousAngle;


        constexpr float PI =
            3.14159265f;


        if (angleDifference > PI)
        {
            angleDifference -=
                2.0f * PI;
        }


        if (angleDifference < -PI)
        {
            angleDifference +=
                2.0f * PI;
        }


        state.accumulatedRotation +=
            angleDifference;


        state.previousAngle =
            angle;
    }


    // =================================================
    // LANDING REWARD
    // =================================================

    if (justLanded)
    {
        constexpr float FLIP_THRESHOLD =
            5.5f;


        bool completedFlip =
            std::abs(
                state.accumulatedRotation
            ) >= FLIP_THRESHOLD;


        if (completedFlip)
        {
            state.boostMeter +=
                50.0f;
        }


        state.boostMeter =
            std::min(
                state.boostMeter,
                100.0f
            );


        state.action =
            AIAction::Racing;

        state.accumulatedRotation =
            0.0f;
    }


    // =================================================
    // ACTUAL WHEELIE DETECTION
    // =================================================

    bool rearGrounded =
        IsRearWheelGrounded(
            bike
        );


    bool frontGrounded =
        IsFrontWheelGrounded(
            bike
        );


    bool realWheelie =
        rearGrounded &&
        !frontGrounded &&
        velocity.x > 2.5f &&
        angle > 0.20f;


    if (realWheelie)
    {
        state.wheelieTime +=
            deltaTime;


        if (state.wheelieTime >= 0.8f &&
            !state.wheelieRewarded)
        {
            state.boostMeter +=
                25.0f;


            state.boostMeter =
                std::min(
                    state.boostMeter,
                    100.0f
                );


            state.wheelieRewarded =
                true;
        }
    }
    else
    {
        state.wheelieTime = 0.0f;

        state.wheelieRewarded = false;
    }


    // =================================================
    // AUTOMATIC BOOST
    // =================================================

    if (!state.boostActive &&
        state.boostMeter >= 100.0f)
    {
        state.boostMeter =
            100.0f;

        state.boostActive =
            true;
    }


    if (state.boostActive)
    {
        constexpr float BOOST_DRAIN =
            20.0f;


        state.boostMeter -=
            BOOST_DRAIN *
            deltaTime;


        if (state.boostMeter <= 0.0f)
        {
            state.boostMeter =
                0.0f;

            state.boostActive =
                false;
        }
    }


    // =================================================
    // GROUNDED DECISIONS
    // =================================================

    if (bikeGrounded)
    {
        float currentSlope =
            GetTerrainSlopeAtX(
                terrainSegments,
                position.x
            );


        // Faster racers look farther ahead.
        float lookAheadDistance = std::clamp(
            std::max(velocity.x, 0.0f) * 0.35f,
            2.5f,
            5.0f
        );

        float aheadSlope =
            GetTerrainSlopeAtX(
                terrainSegments,
                position.x + lookAheadDistance
            );


        // =================================================
        // TERRAIN-AWARE SPEED CONTROL
        // =================================================

        // Different AI personalities have different
        // speed limits when approaching steep descents.
        float safeDescentSpeed =
            9.0f * profile.speedMultiplier;

        bool steepDescentAhead =
            aheadSlope < -0.40f;

        bool approachingTooFast =
            velocity.x > safeDescentSpeed;

        if (steepDescentAhead && approachingTooFast)
        {
            // Reduce motor speed before the descent.
            input.driveForward = false;
        }


        bool approachingCrest =
            currentSlope > 0.10f &&
            aheadSlope <
            currentSlope - 0.15f;


        // =============================================
        // TERRAIN-AWARE BUNNY HOP
        // =============================================

        if (approachingCrest &&
            state.hopCooldown <= 0.0f &&
            velocity.x > 3.0f)
        {
            input.jumpPressed = true;

            state.action =
                AIAction::BunnyHop;

            state.hopCooldown =
                2.0f;

            state.wasGrounded =
                bikeGrounded;

            return input;
        }


        // =============================================
        // WHEELIE
        // =============================================

        bool roughlyFlat =
            std::abs(currentSlope) <
            0.20f;


        if (state.action ==
            AIAction::Racing &&
            roughlyFlat &&
            state.wheelieCooldown <= 0.0f &&
            velocity.x > 3.5f)
        {
            state.action =
                AIAction::Wheelie;

            state.actionTimer =
                profile.wheelieDuration;

            state.wheelieCooldown =
                profile.wheelieInterval;
        }


        if (state.action ==
            AIAction::Wheelie)
        {
            input.leanBackward =
                true;


            if (state.actionTimer <= 0.0f)
            {
                state.action =
                    AIAction::Racing;
            }
        }
    }


    // =================================================
    // AIRBORNE DECISIONS
    // =================================================

    else
    {
        constexpr float FLIP_TARGET =
            6.1f;


        // =============================================
        // FRONT FLIP
        // =============================================
        if (state.action ==
            AIAction::FrontFlip)
        {
            if (std::abs(
                state.accumulatedRotation
            ) < FLIP_TARGET)
            {
                input.leanForward =
                    true;
            }
            else
            {
                state.action =
                    AIAction::LandingRecovery;
            }
        }


        // =============================================
        // BACKFLIP
        // =============================================

        else if (state.action ==
            AIAction::BackFlip)
        {
            if (std::abs(
                state.accumulatedRotation
            ) < FLIP_TARGET)
            {
                input.leanBackward =
                    true;
            }
            else
            {
                state.action =
                    AIAction::LandingRecovery;
            }
        }


        // =============================================
        // LANDING RECOVERY
        // =============================================


        if (state.action == AIAction::LandingRecovery)
        {
            // Read the bike's current rotation speed.
            float angularVelocity =
                b2Body_GetAngularVelocity(
                    bike.chassisBodyId
                );

            // Predict the bike's angle 0.15 seconds ahead.
            constexpr float predictionTime = 0.15f;

            float predictedAngle =
                angle + angularVelocity * predictionTime;

            // Keep the predicted angle in [-PI, PI].
            constexpr float TWO_PI = 6.2831853f;

            predictedAngle =
                std::remainder(predictedAngle, TWO_PI);

            constexpr float deadZone = 0.10f;

            // Correct the predicted landing orientation.
            if (predictedAngle > deadZone)
            {
                input.leanForward = true;
            }
            else if (predictedAngle < -deadZone)
            {
                input.leanBackward = true;
            }
        }

    }


    state.wasGrounded =
        bikeGrounded;


    return input;
}


// =====================================================
// NON-TELEPORTING AI BALANCE ASSIST
// =====================================================

void ApplyAIStabilityAssist(
    const Bike& bike,
    bool bikeGrounded,
    const AIProfile& profile,
    AIState& state,
    const std::vector<TerrainSegment>& terrainSegments)
{
    bool doingFlip =
        state.action ==
        AIAction::FrontFlip ||
        state.action ==
        AIAction::BackFlip;


    if (doingFlip)
    {
        return;
    }


    float angle =
        b2Rot_GetAngle(
            b2Body_GetRotation(
                bike.chassisBodyId
            )
        );


    float currentAngularVelocity =
        b2Body_GetAngularVelocity(
            bike.chassisBodyId
        );


    float strength =
        bikeGrounded
        ? 5.0f
        : 3.5f;


    strength *=
        profile.balanceSkill;



    // Match the bike's balance target to the terrain.
    float targetAngle = 0.0f;


    if (bikeGrounded)
    {
        float rearX = b2Body_GetPosition(
            bike.rearWheelBodyId
        ).x;

        float frontX = b2Body_GetPosition(
            bike.frontWheelBodyId
        ).x;

        float rearSlope = GetTerrainSlopeAtX(
            terrainSegments, rearX
        );

        float frontSlope = GetTerrainSlopeAtX(
            terrainSegments, frontX
        );

        float averageSlope =
            (rearSlope + frontSlope) * 0.5f;

        targetAngle = std::atan(averageSlope);
    }
    else if (state.action == AIAction::LandingRecovery)
    {
        // Estimate where the bike is heading.
        b2Vec2 position = b2Body_GetPosition(
            bike.chassisBodyId
        );

        b2Vec2 velocity = b2Body_GetLinearVelocity(
            bike.chassisBodyId
        );

        constexpr float predictionTime = 0.20f;

        float predictedX =
            position.x + velocity.x * predictionTime;

        // Sample the terrain near the expected landing.
        float predictedSlope = GetTerrainSlopeAtX(
            terrainSegments,
            predictedX
        );

        // Convert terrain slope into a target bike angle.
        targetAngle = std::atan(predictedSlope);

        // Avoid extreme landing corrections.
        targetAngle = std::clamp(
            targetAngle,
            -0.35f,
            0.35f
        );
    }


    float angleError = targetAngle - angle;

    float targetAngularVelocity =
        angleError * strength;



    targetAngularVelocity =
        std::clamp(
            targetAngularVelocity,
            -4.0f,
            4.0f
        );


    float blend =
        bikeGrounded
        ? 0.22f
        : 0.14f;


    float newAngularVelocity =
        currentAngularVelocity +
        (
            targetAngularVelocity -
            currentAngularVelocity
            ) *
        blend;


    b2Body_SetAngularVelocity(
        bike.chassisBodyId,
        newAngularVelocity
    );
}

