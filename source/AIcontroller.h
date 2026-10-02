#pragma once

#include "Bike.h"
#include "Input.h"


InputState BuildAIInput(
    const Bike& bike,
    bool bikeGrounded,
    bool levelComplete
);