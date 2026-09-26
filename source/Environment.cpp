#include "Environment.h"


// ---------------------------------------------------------
// CONSTRUCTOR
// ---------------------------------------------------------

Environment::Environment()
{
    // =====================================================
    // DAY
    // =====================================================

    dayTheme.skyColor =
    {
        135,
        206,
        235,
        255
    };

    dayTheme.farMountainColor =
    {
        0.55f,
        0.68f,
        0.58f,
        1.0f
    };

    dayTheme.nearMountainColor =
    {
        0.38f,
        0.55f,
        0.40f,
        1.0f
    };


    // =====================================================
    // SUNSET
    // =====================================================

    sunsetTheme.skyColor =
    {
        245,
        145,
        105,
        255
    };

    sunsetTheme.farMountainColor =
    {
        0.52f,
        0.40f,
        0.48f,
        1.0f
    };

    sunsetTheme.nearMountainColor =
    {
        0.32f,
        0.27f,
        0.35f,
        1.0f
    };


    // =====================================================
    // NIGHT
    // =====================================================

    nightTheme.skyColor =
    {
        25,
        35,
        75,
        255
    };

    nightTheme.farMountainColor =
    {
        0.18f,
        0.22f,
        0.32f,
        1.0f
    };

    nightTheme.nearMountainColor =
    {
        0.10f,
        0.14f,
        0.22f,
        1.0f
    };
}


// ---------------------------------------------------------
// UPDATE
// ---------------------------------------------------------

void Environment::Update(float newPlayerX)
{
    playerX =
        newPlayerX;
}


// ---------------------------------------------------------
// CLAMP
// ---------------------------------------------------------

float Environment::Clamp01(float value) const
{
    if (value < 0.0f)
    {
        return 0.0f;
    }

    if (value > 1.0f)
    {
        return 1.0f;
    }

    return value;
}


// ---------------------------------------------------------
// SDL COLOR BLEND
// ---------------------------------------------------------

SDL_Color Environment::LerpColor(
    const SDL_Color& start,
    const SDL_Color& end,
    float amount
) const
{
    amount =
        Clamp01(amount);


    SDL_Color result;


    result.r =
        static_cast<Uint8>(
            start.r +
            (end.r - start.r) *
            amount
            );


    result.g =
        static_cast<Uint8>(
            start.g +
            (end.g - start.g) *
            amount
            );


    result.b =
        static_cast<Uint8>(
            start.b +
            (end.b - start.b) *
            amount
            );


    result.a =
        255;


    return result;
}


// ---------------------------------------------------------
// SDL FCOLOR BLEND
// ---------------------------------------------------------

SDL_FColor Environment::LerpColor(
    const SDL_FColor& start,
    const SDL_FColor& end,
    float amount
) const
{
    amount =
        Clamp01(amount);


    SDL_FColor result;


    result.r =
        start.r +
        (end.r - start.r) *
        amount;


    result.g =
        start.g +
        (end.g - start.g) *
        amount;


    result.b =
        start.b +
        (end.b - start.b) *
        amount;


    result.a =
        1.0f;


    return result;
}


// ---------------------------------------------------------
// GET CURRENT THEME
// ---------------------------------------------------------

Environment::EnvironmentTheme
Environment::GetCurrentTheme() const
{
    EnvironmentTheme currentTheme =
        dayTheme;


    // =====================================================
    // DAY -> SUNSET
    // =====================================================

    if (playerX >= 70.0f &&
        playerX < 140.0f)
    {
        float blend =
            (playerX - 70.0f) /
            (140.0f - 70.0f);


        currentTheme.skyColor =
            LerpColor(
                dayTheme.skyColor,
                sunsetTheme.skyColor,
                blend
            );


        currentTheme.farMountainColor =
            LerpColor(
                dayTheme.farMountainColor,
                sunsetTheme.farMountainColor,
                blend
            );


        currentTheme.nearMountainColor =
            LerpColor(
                dayTheme.nearMountainColor,
                sunsetTheme.nearMountainColor,
                blend
            );
    }


    // =====================================================
    // SUNSET
    // =====================================================

    else if (playerX >= 140.0f &&
        playerX < 200.0f)
    {
        currentTheme =
            sunsetTheme;
    }


    // =====================================================
    // SUNSET -> NIGHT
    // =====================================================

    else if (playerX >= 200.0f &&
        playerX < 270.0f)
    {
        float blend =
            (playerX - 200.0f) /
            (270.0f - 200.0f);


        currentTheme.skyColor =
            LerpColor(
                sunsetTheme.skyColor,
                nightTheme.skyColor,
                blend
            );


        currentTheme.farMountainColor =
            LerpColor(
                sunsetTheme.farMountainColor,
                nightTheme.farMountainColor,
                blend
            );


        currentTheme.nearMountainColor =
            LerpColor(
                sunsetTheme.nearMountainColor,
                nightTheme.nearMountainColor,
                blend
            );
    }


    // =====================================================
    // NIGHT
    // =====================================================

    else if (playerX >= 270.0f)
    {
        currentTheme =
            nightTheme;
    }


    return currentTheme;
}


// ---------------------------------------------------------
// RENDER
// ---------------------------------------------------------

void Environment::Render(
    SDL_Renderer* renderer,
    float cameraX
) const
{
    EnvironmentTheme currentTheme =
        GetCurrentTheme();


    // =====================================================
    // SKY
    // =====================================================

    SDL_SetRenderDrawColor(
        renderer,
        currentTheme.skyColor.r,
        currentTheme.skyColor.g,
        currentTheme.skyColor.b,
        currentTheme.skyColor.a
    );

    SDL_RenderClear(
        renderer
    );


    // =====================================================
    // FAR MOUNTAINS
    // =====================================================

    const float farMountainShift =
        -cameraX * 1.5f;


    SDL_Vertex farMountains[12]{};


    // Mountain 1

    farMountains[0].position =
        SDL_FPoint
    {
        -100.0f + farMountainShift,
        530.0f
    };

    farMountains[1].position =
        SDL_FPoint
    {
        120.0f + farMountainShift,
        350.0f
    };

    farMountains[2].position =
        SDL_FPoint
    {
        340.0f + farMountainShift,
        530.0f
    };


    // Mountain 2

    farMountains[3].position =
        SDL_FPoint
    {
        220.0f + farMountainShift,
        530.0f
    };

    farMountains[4].position =
        SDL_FPoint
    {
        500.0f + farMountainShift,
        320.0f
    };

    farMountains[5].position =
        SDL_FPoint
    {
        780.0f + farMountainShift,
        530.0f
    };


    // Mountain 3

    farMountains[6].position =
        SDL_FPoint
    {
        650.0f + farMountainShift,
        530.0f
    };

    farMountains[7].position =
        SDL_FPoint
    {
        900.0f + farMountainShift,
        370.0f
    };

    farMountains[8].position =
        SDL_FPoint
    {
        1150.0f + farMountainShift,
        530.0f
    };


    // Mountain 4

    farMountains[9].position =
        SDL_FPoint
    {
        1050.0f + farMountainShift,
        530.0f
    };

    farMountains[10].position =
        SDL_FPoint
    {
        1300.0f + farMountainShift,
        340.0f
    };

    farMountains[11].position =
        SDL_FPoint
    {
        1550.0f + farMountainShift,
        530.0f
    };


    for (int i = 0; i < 12; ++i)
    {
        farMountains[i].color =
            currentTheme.farMountainColor;
    }


    const int farMountainIndices[12] =
    {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
        9, 10, 11
    };


    SDL_RenderGeometry(
        renderer,
        nullptr,
        farMountains,
        12,
        farMountainIndices,
        12
    );


    // =====================================================
    // NEAR MOUNTAINS
    // =====================================================

    const float nearMountainShift =
        -cameraX * 3.0f;


    SDL_Vertex nearMountains[9]{};


    // Mountain 1

    nearMountains[0].position =
        SDL_FPoint
    {
        -150.0f + nearMountainShift,
        560.0f
    };

    nearMountains[1].position =
        SDL_FPoint
    {
        100.0f + nearMountainShift,
        410.0f
    };

    nearMountains[2].position =
        SDL_FPoint
    {
        350.0f + nearMountainShift,
        560.0f
    };


    // Mountain 2

    nearMountains[3].position =
        SDL_FPoint
    {
        300.0f + nearMountainShift,
        560.0f
    };

    nearMountains[4].position =
        SDL_FPoint
    {
        620.0f + nearMountainShift,
        390.0f
    };

    nearMountains[5].position =
        SDL_FPoint
    {
        940.0f + nearMountainShift,
        560.0f
    };


    // Mountain 3

    nearMountains[6].position =
        SDL_FPoint
    {
        850.0f + nearMountainShift,
        560.0f
    };

    nearMountains[7].position =
        SDL_FPoint
    {
        1120.0f + nearMountainShift,
        420.0f
    };

    nearMountains[8].position =
        SDL_FPoint
    {
        1390.0f + nearMountainShift,
        560.0f
    };


    for (int i = 0; i < 9; ++i)
    {
        nearMountains[i].color =
            currentTheme.nearMountainColor;
    }


    const int nearMountainIndices[9] =
    {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8
    };


    SDL_RenderGeometry(
        renderer,
        nullptr,
        nearMountains,
        9,
        nearMountainIndices,
        9
    );
}