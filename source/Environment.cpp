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

    dayTheme.cloudColor =
    {
        235,
        240,
        242,
        190
    };


    dayTheme.farHillColor =
    {
        0.68f,
        0.76f,
        0.70f,
        1.0f
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

    dayTheme.foregroundColor =
    {
        0.20f,
        0.34f,
        0.22f,
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

    sunsetTheme.cloudColor =
    {
        235,
        165,
        150,
        190
    };


    sunsetTheme.farHillColor =
    {
        0.58f,
        0.45f,
        0.48f,
        1.0f
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

    sunsetTheme.foregroundColor =
    {
        0.22f,
        0.18f,
        0.22f,
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

    nightTheme.cloudColor =
    {
        80,
        90,
        120,
        100
    };


    nightTheme.farHillColor =
    {
        0.14f,
        0.18f,
        0.27f,
        1.0f
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

    nightTheme.foregroundColor =
    {
        0.05f,
        0.08f,
        0.12f,
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

void Environment::DrawFilledCircle(
    SDL_Renderer* renderer,
    float centerX,
    float centerY,
    float radius,
    SDL_Color color
) const
{
    SDL_SetRenderDrawColor(
        renderer,
        color.r,
        color.g,
        color.b,
        color.a
    );

    for (float y = -radius;
        y <= radius;
        y += 1.0f)
    {
        for (float x = -radius;
            x <= radius;
            x += 1.0f)
        {
            if (x * x + y * y <=
                radius * radius)
            {
                SDL_RenderPoint(
                    renderer,
                    centerX + x,
                    centerY + y
                );
            }
        }
    }
}

void Environment::RenderSun(
    SDL_Renderer* renderer
) const
{
    // Sun disappears once night approaches.
    if (playerX >= 230.0f)
    {
        return;
    }


    SDL_Color daySun =
    {
        255,
        235,
        135,
        255
    };


    SDL_Color sunsetSun =
    {
        255,
        105,
        55,
        255
    };


    float transition =
        Clamp01(
            (playerX - 70.0f) /
            130.0f
        );


    SDL_Color sunColor =
        LerpColor(
            daySun,
            sunsetSun,
            transition
        );


    // Start high in the sky.
    float sunX =
        850.0f;

    float sunY =
        110.0f;


    // During the level the sun slowly moves
    // toward the horizon.
    sunX -=
        140.0f *
        transition;

    sunY +=
        210.0f *
        transition;


    // Fade sun out as night approaches.
    if (playerX > 190.0f)
    {
        float fade =
            Clamp01(
                (230.0f - playerX) /
                40.0f
            );

        sunColor.a =
            static_cast<Uint8>(
                255.0f *
                fade
                );
    }


    DrawFilledCircle(
        renderer,
        sunX,
        sunY,
        38.0f,
        sunColor
    );
}

void Environment::RenderMoon(
    SDL_Renderer* renderer
) const
{
    if (playerX < 200.0f)
    {
        return;
    }


    float moonFade =
        Clamp01(
            (playerX - 200.0f) /
            70.0f
        );


    SDL_Color moonColor =
    {
        225,
        230,
        245,
        static_cast<Uint8>(
            255.0f *
            moonFade
        )
    };


    DrawFilledCircle(
        renderer,
        850.0f,
        120.0f,
        32.0f,
        moonColor
    );

    SDL_Color craterColor =
    {
        185,
        195,
        215,
        static_cast<Uint8>(
            170.0f *
            moonFade
        )
    };


    DrawFilledCircle(
        renderer,
        840.0f,
        112.0f,
        6.0f,
        craterColor
    );

    DrawFilledCircle(
        renderer,
        860.0f,
        128.0f,
        5.0f,
        craterColor
    );

    DrawFilledCircle(
        renderer,
        848.0f,
        137.0f,
        3.0f,
        craterColor
    );
}

void Environment::RenderStars(
    SDL_Renderer* renderer
) const
{
    if (playerX < 190.0f)
    {
        return;
    }


    float starFade =
        Clamp01(
            (playerX - 190.0f) /
            80.0f
        );


    SDL_Color starColor =
    {
        245,
        245,
        225,
        static_cast<Uint8>(
            255.0f *
            starFade
        )
    };


    SDL_SetRenderDrawColor(
        renderer,
        starColor.r,
        starColor.g,
        starColor.b,
        starColor.a
    );


    const SDL_FPoint stars[] =
    {
        { 80.0f,  80.0f },
        { 150.0f, 145.0f },
        { 230.0f, 75.0f },
        { 310.0f, 180.0f },
        { 390.0f, 110.0f },
        { 475.0f, 60.0f },
        { 550.0f, 155.0f },
        { 630.0f, 95.0f },
        { 710.0f, 190.0f },
        { 790.0f, 70.0f },
        { 870.0f, 145.0f },
        { 950.0f, 60.0f },
        { 1130.0f, 180.0f },
        { 1190.0f, 90.0f },
        { 1230.0f, 230.0f },

        { 120.0f, 260.0f },
        { 260.0f, 235.0f },
        { 430.0f, 250.0f },
        { 610.0f, 270.0f },
        { 820.0f, 245.0f }
    };


    const int starCount =
        sizeof(stars) /
        sizeof(stars[0]);


    for (int i = 0;
        i < starCount;
        ++i)
    {
        float size =
            (i % 4 == 0)
            ? 3.0f
            : 2.0f;


        SDL_FRect starRect =
        {
            stars[i].x,
            stars[i].y,
            size,
            size
        };


        SDL_RenderFillRect(
            renderer,
            &starRect
        );
    }
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

        currentTheme.cloudColor =
            LerpColor(
                dayTheme.cloudColor,
                sunsetTheme.cloudColor,
                blend
            );


        currentTheme.farHillColor =
            LerpColor(
                dayTheme.farHillColor,
                sunsetTheme.farHillColor,
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

        currentTheme.foregroundColor =
            LerpColor(
                dayTheme.foregroundColor,
                sunsetTheme.foregroundColor,
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

        currentTheme.cloudColor =
            LerpColor(
                sunsetTheme.cloudColor,
                nightTheme.cloudColor,
                blend
            );


        currentTheme.farHillColor =
            LerpColor(
                sunsetTheme.farHillColor,
                nightTheme.farHillColor,
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

        currentTheme.foregroundColor =
            LerpColor(
                sunsetTheme.foregroundColor,
                nightTheme.foregroundColor,
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


    SDL_SetRenderDrawColor(
        renderer,
        currentTheme.skyColor.r,
        currentTheme.skyColor.g,
        currentTheme.skyColor.b,
        currentTheme.skyColor.a
    );

    SDL_RenderClear(renderer);


    SDL_SetRenderDrawBlendMode(
        renderer,
        SDL_BLENDMODE_BLEND
    );


    // =====================================================
    // SKY OBJECTS
    // =====================================================

    RenderStars(
        renderer
    );

    RenderSun(
        renderer
    );

    RenderMoon(
        renderer
    );


    // =====================================================
    // CLOUDS
    // =====================================================

    RenderClouds(
        renderer,
        cameraX,
        currentTheme
    );


    // =====================================================
    // VERY FAR HILLS
    // =====================================================

    RenderFarHills(
        renderer,
        cameraX,
        currentTheme
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

    // =====================================================
    // FOREGROUND SCENERY
    // =====================================================

    RenderForeground(
        renderer,
        cameraX,
        currentTheme
    );
}

void Environment::DrawTree(
    SDL_Renderer* renderer,
    float x,
    float y,
    float scale,
    SDL_FColor color
) const
{
    SDL_SetRenderDrawColorFloat(
        renderer,
        color.r,
        color.g,
        color.b,
        color.a
    );


    // Trunk

    SDL_FRect trunk =
    {
        x - 5.0f * scale,
        y - 50.0f * scale,
        10.0f * scale,
        50.0f * scale
    };

    SDL_RenderFillRect(
        renderer,
        &trunk
    );


    // Tree top

    SDL_Vertex treeTop[3]{};

    treeTop[0].position =
    {
        x,
        y - 110.0f * scale
    };

    treeTop[1].position =
    {
        x - 38.0f * scale,
        y - 42.0f * scale
    };

    treeTop[2].position =
    {
        x + 38.0f * scale,
        y - 42.0f * scale
    };


    for (int i = 0; i < 3; ++i)
    {
        treeTop[i].color =
            color;
    }


    const int indices[3] =
    {
        0,
        1,
        2
    };


    SDL_RenderGeometry(
        renderer,
        nullptr,
        treeTop,
        3,
        indices,
        3
    );
}

void Environment::DrawBush(
    SDL_Renderer* renderer,
    float x,
    float y,
    float scale,
    SDL_FColor color
) const
{
    SDL_Color bushColor =
    {
        static_cast<Uint8>(
            color.r * 255.0f
        ),

        static_cast<Uint8>(
            color.g * 255.0f
        ),

        static_cast<Uint8>(
            color.b * 255.0f
        ),

        255
    };


    DrawFilledCircle(
        renderer,
        x,
        y - 12.0f * scale,
        18.0f * scale,
        bushColor
    );


    DrawFilledCircle(
        renderer,
        x + 20.0f * scale,
        y - 15.0f * scale,
        21.0f * scale,
        bushColor
    );


    DrawFilledCircle(
        renderer,
        x + 40.0f * scale,
        y - 10.0f * scale,
        17.0f * scale,
        bushColor
    );
}

void Environment::DrawRock(
    SDL_Renderer* renderer,
    float x,
    float y,
    float scale,
    SDL_FColor color
) const
{
    SDL_Vertex rock[3]{};


    rock[0].position =
    {
        x,
        y
    };


    rock[1].position =
    {
        x + 22.0f * scale,
        y - 32.0f * scale
    };


    rock[2].position =
    {
        x + 52.0f * scale,
        y
    };


    for (int i = 0; i < 3; ++i)
    {
        rock[i].color =
            color;
    }


    const int indices[3] =
    {
        0,
        1,
        2
    };


    SDL_RenderGeometry(
        renderer,
        nullptr,
        rock,
        3,
        indices,
        3
    );
}

void Environment::RenderForeground(
    SDL_Renderer* renderer,
    float cameraX,
    const EnvironmentTheme& theme
) const
{
    // Faster than the mountain layers,
    // so these objects feel much closer.

    const float foregroundShift =
        -cameraX * 6.0f;


    const float groundY =
        590.0f;


    // =====================================================
    // TREES
    // =====================================================

    DrawTree(
        renderer,
        150.0f + foregroundShift,
        groundY,
        0.75f,
        theme.foregroundColor
    );


    DrawTree(
        renderer,
        760.0f + foregroundShift,
        groundY,
        1.0f,
        theme.foregroundColor
    );


    DrawTree(
        renderer,
        1450.0f + foregroundShift,
        groundY,
        0.85f,
        theme.foregroundColor
    );


    DrawTree(
        renderer,
        2200.0f + foregroundShift,
        groundY,
        1.1f,
        theme.foregroundColor
    );


    DrawTree(
        renderer,
        2900.0f + foregroundShift,
        groundY,
        0.80f,
        theme.foregroundColor
    );


    // =====================================================
    // BUSHES
    // =====================================================

    DrawBush(
        renderer,
        430.0f + foregroundShift,
        groundY,
        0.70f,
        theme.foregroundColor
    );


    DrawBush(
        renderer,
        1180.0f + foregroundShift,
        groundY,
        0.90f,
        theme.foregroundColor
    );


    DrawBush(
        renderer,
        1850.0f + foregroundShift,
        groundY,
        0.65f,
        theme.foregroundColor
    );


    DrawBush(
        renderer,
        2550.0f + foregroundShift,
        groundY,
        0.80f,
        theme.foregroundColor
    );


    // =====================================================
    // ROCKS
    // =====================================================

    DrawRock(
        renderer,
        600.0f + foregroundShift,
        groundY,
        0.8f,
        theme.foregroundColor
    );


    DrawRock(
        renderer,
        1680.0f + foregroundShift,
        groundY,
        1.0f,
        theme.foregroundColor
    );


    DrawRock(
        renderer,
        2700.0f + foregroundShift,
        groundY,
        0.7f,
        theme.foregroundColor
    );
}

void Environment::DrawCloud(
    SDL_Renderer* renderer,
    float x,
    float y,
    float scale,
    SDL_Color color
) const
{
    // Left puff
    DrawFilledCircle(
        renderer,
        x,
        y,
        20.0f * scale,
        color
    );


    // Middle / largest puff
    DrawFilledCircle(
        renderer,
        x + 28.0f * scale,
        y - 10.0f * scale,
        28.0f * scale,
        color
    );


    // Right puff
    DrawFilledCircle(
        renderer,
        x + 58.0f * scale,
        y,
        22.0f * scale,
        color
    );


    // Cloud base
    SDL_SetRenderDrawColor(
        renderer,
        color.r,
        color.g,
        color.b,
        color.a
    );


    SDL_FRect cloudBase =
    {
        x - 18.0f * scale,
        y,
        98.0f * scale,
        22.0f * scale
    };


    SDL_RenderFillRect(
        renderer,
        &cloudBase
    );
}

void Environment::RenderClouds(
    SDL_Renderer* renderer,
    float cameraX,
    const EnvironmentTheme& theme
) const
{
    const float cloudShift =
        -cameraX * 0.35f;


    DrawCloud(
        renderer,
        100.0f + cloudShift,
        120.0f,
        0.75f,
        theme.cloudColor
    );


    DrawCloud(
        renderer,
        420.0f + cloudShift,
        185.0f,
        0.55f,
        theme.cloudColor
    );


    DrawCloud(
        renderer,
        720.0f + cloudShift,
        95.0f,
        0.65f,
        theme.cloudColor
    );


    DrawCloud(
        renderer,
        1080.0f + cloudShift,
        180.0f,
        0.50f,
        theme.cloudColor
    );
}

void Environment::RenderFarHills(
    SDL_Renderer* renderer,
    float cameraX,
    const EnvironmentTheme& theme
) const
{
    const float hillShift =
        -cameraX * 0.65f;


    SDL_Vertex hills[15]{};


    // Hill 1

    hills[0].position =
        SDL_FPoint{
            -250.0f + hillShift,
            550.0f
    };

    hills[1].position =
        SDL_FPoint{
            -20.0f + hillShift,
            430.0f
    };

    hills[2].position =
        SDL_FPoint{
            220.0f + hillShift,
            550.0f
    };


    // Hill 2

    hills[3].position =
        SDL_FPoint{
            100.0f + hillShift,
            550.0f
    };

    hills[4].position =
        SDL_FPoint{
            370.0f + hillShift,
            400.0f
    };

    hills[5].position =
        SDL_FPoint{
            650.0f + hillShift,
            550.0f
    };


    // Hill 3

    hills[6].position =
        SDL_FPoint{
            520.0f + hillShift,
            550.0f
    };

    hills[7].position =
        SDL_FPoint{
            780.0f + hillShift,
            440.0f
    };

    hills[8].position =
        SDL_FPoint{
            1040.0f + hillShift,
            550.0f
    };


    // Hill 4

    hills[9].position =
        SDL_FPoint{
            900.0f + hillShift,
            550.0f
    };

    hills[10].position =
        SDL_FPoint{
            1160.0f + hillShift,
            410.0f
    };

    hills[11].position =
        SDL_FPoint{
            1420.0f + hillShift,
            550.0f
    };


    // Hill 5

    hills[12].position =
        SDL_FPoint{
            1300.0f + hillShift,
            550.0f
    };

    hills[13].position =
        SDL_FPoint{
            1540.0f + hillShift,
            435.0f
    };

    hills[14].position =
        SDL_FPoint{
            1780.0f + hillShift,
            550.0f
    };


    for (int i = 0;
        i < 15;
        ++i)
    {
        hills[i].color =
            theme.farHillColor;
    }


    const int indices[15] =
    {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
        9, 10, 11,
        12, 13, 14
    };


    SDL_RenderGeometry(
        renderer,
        nullptr,
        hills,
        15,
        indices,
        15
    );
}