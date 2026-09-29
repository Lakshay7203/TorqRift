#include "AIController.h"


InputState BuildAIInput(
    const Bike& bike,
    bool bikeGrounded,
    bool levelComplete)
{
    InputState aiInput;


    // Don't move after the race has finished.
    if (levelComplete)
    {
        return aiInput;
    }


    // =====================================================
    // THROTTLE
    // =====================================================

    aiInput.driveForward = true;


    // =====================================================
    // AIR BALANCE
    // =====================================================

    if (!bikeGrounded)
    {
        float chassisAngle =
            b2Rot_GetAngle(
                b2Body_GetRotation(
                    bike.chassisBodyId
                )
            );


        constexpr float angleDeadZone =
            0.08f;


        // Bike rotated too far backward.
        // Apply forward lean to bring it back.
        if (chassisAngle >
            angleDeadZone)
        {
            aiInput.leanForward =
                true;
        }


        // Bike rotated too far forward.
        // Apply backward lean.
        else if (chassisAngle <
            -angleDeadZone)
        {
            aiInput.leanBackward =
                true;
        }
    }


    return aiInput;
}
