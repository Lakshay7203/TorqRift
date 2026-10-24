#include "Environment.h"
#include <cmath>


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

    dayTheme.terrainTint =
    {
        1.0f,
        1.0f,
        1.0f,
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

    sunsetTheme.terrainTint =
    {
        0.95f,
        0.72f,
        0.60f,
        1.0f
    };

    // =====================================================
    // NIGHT
    // =====================================================


    nightTheme.skyColor =
    {
        15, 19, 49, 255
    };

    nightTheme.cloudColor =
    {
        76, 79, 130, 110
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

    nightTheme.terrainTint =
    {
        0.45f,
        0.55f,
        0.75f,
        1.0f
    };
}


// ---------------------------------------------------------
// UPDATE
// ---------------------------------------------------------

void Environment::Update(
    float newPlayerX,
    float deltaTime
)
{
    playerX =
        newPlayerX;

    animationTime +=
        deltaTime;
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

    float moonFade = 1.0f;



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

    float starFade = 1.0f;



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
    // Level 1 - Haunted Run
    // The entire race takes place at night.

    return nightTheme;
}


static void DrawHauntedBats(
    SDL_Renderer* renderer,
    float cameraX,
    float animationTime)
{
    // Same parallax as the haunted mansion.
    const float mansionX = 850.0f - cameraX * 5.0f;

    // Each bat has a different position and movement phase.
    const SDL_FPoint batPositions[] =
    {
        {  30.0f, 185.0f },
        { 100.0f, 145.0f },
        { 180.0f, 175.0f },
        { 250.0f, 125.0f },
        { 310.0f, 195.0f },
        { 360.0f, 155.0f }
    };

    SDL_SetRenderDrawColor(
        renderer, 10, 12, 27, 255
    );

    for (int i = 0; i < 6; ++i)
    {
        float phase = animationTime * 5.0f + i * 1.4f;

        float x =
            mansionX + batPositions[i].x +
            std::sin(animationTime * 0.8f + i) * 18.0f;

        float y =
            batPositions[i].y +
            std::sin(animationTime * 1.3f + i) * 9.0f;

        if (x < -40.0f || x > 1320.0f)
            continue;

        // Wing flap animation.
        float flap = std::sin(phase) * 8.0f;

        float size = 0.8f + (i % 3) * 0.2f;

        // Bat body.
        SDL_FRect body =
        {
            x - 3.0f * size,
            y - 3.0f * size,
            6.0f * size,
            7.0f * size
        };

        SDL_RenderFillRect(renderer, &body);

        // Left wing.
        SDL_RenderLine(
            renderer,
            x, y,
            x - 12.0f * size,
            y - flap
        );

        SDL_RenderLine(
            renderer,
            x - 12.0f * size,
            y - flap,
            x - 22.0f * size,
            y + flap * 0.4f
        );

        // Right wing.
        SDL_RenderLine(
            renderer,
            x, y,
            x + 12.0f * size,
            y - flap
        );

        SDL_RenderLine(
            renderer,
            x + 12.0f * size,
            y - flap,
            x + 22.0f * size,
            y + flap * 0.4f
        );
    }
}

static void DrawHauntedMansion(
    SDL_Renderer* renderer,
    float cameraX)
{
    // The mansion moves slowly with the camera.
    // This makes it appear far away.
    const float x = 850.0f - cameraX * 5.0f;
    const float y = 255.0f;

    // Skip drawing when completely off-screen.
    if (x > 1400.0f || x < -350.0f)
        return;

    auto FillBox = [&](float px, float py,
        float w, float h,
        Uint8 r, Uint8 g, Uint8 b)
        {
            SDL_SetRenderDrawColor(
                renderer, r, g, b, 255
            );

            SDL_FRect rect = { px, py, w, h };
            SDL_RenderFillRect(renderer, &rect);
        };

    auto Roof = [&](float left, float top,
        float width, float height)
        {
            SDL_Vertex vertices[3]{};

            vertices[0].position =
            { left + width / 2.0f, top };

            vertices[1].position =
            { left, top + height };

            vertices[2].position =
            { left + width, top + height };

            const SDL_FColor color =
            { 0.035f, 0.045f, 0.085f, 1.0f };

            for (SDL_Vertex& vertex : vertices)
                vertex.color = color;

            const int indices[] = { 0, 1, 2 };

            SDL_RenderGeometry(
                renderer, nullptr,
                vertices, 3, indices, 3
            );
        };


    // ============================================
    // HAUNTED MANSION CLIFF
    // ============================================

    SDL_Vertex cliff[6]{};

    // Left slope
    cliff[0].position =
    { x - 95.0f, y + 340.0f };

    // Top-left edge beneath the mansion
    cliff[1].position =
    { x + 10.0f, y + 245.0f };

    // Top-right edge beneath the mansion
    cliff[2].position =
    { x + 275.0f, y + 245.0f };

    // Right slope
    cliff[3].position =
    { x + 380.0f, y + 340.0f };

    // Bottom-right
    cliff[4].position =
    { x + 380.0f, 760.0f };

    // Bottom-left
    cliff[5].position =
    { x - 95.0f, 760.0f };

    const SDL_FColor cliffColor =
    {
        0.075f,
        0.095f,
        0.16f,
        1.0f
    };

    for (SDL_Vertex& vertex : cliff)
    {
        vertex.color = cliffColor;
    }

    const int cliffIndices[] =
    {
        0, 1, 2,
        0, 2, 3,
        0, 3, 4,
        0, 4, 5
    };

    SDL_RenderGeometry(
        renderer,
        nullptr,
        cliff,
        6,
        cliffIndices,
        12
    );


    // ============================================
    // MAIN BUILDING
    // ============================================

    FillBox(x + 65, y + 115,
        150, 130, 19, 22, 43);

    Roof(x + 55, y + 75, 170, 60);

    // ============================================
    // LEFT TOWER
    // ============================================

    FillBox(x + 22, y + 85,
        55, 160, 16, 20, 39);

    Roof(x + 12, y + 20, 75, 75);

    // ============================================
    // RIGHT TOWER
    // ============================================

    FillBox(x + 205, y + 65,
        52, 180, 16, 20, 39);

    Roof(x + 195, y + 0, 72, 75);

    // ============================================
    // GLOWING WINDOWS
    // ============================================

    // Dark window frames
    FillBox(x + 36, y + 125,
        22, 43, 5, 8, 20);

    FillBox(x + 220, y + 105,
        22, 43, 5, 8, 20);

    FillBox(x + 100, y + 155,
        25, 40, 5, 8, 20);

    FillBox(x + 158, y + 155,
        25, 40, 5, 8, 20);

    // Warm orange interiors
    FillBox(x + 40, y + 130,
        14, 34, 255, 153, 48);

    FillBox(x + 224, y + 110,
        14, 34, 255, 153, 48);

    FillBox(x + 105, y + 160,
        15, 30, 255, 180, 69);

    FillBox(x + 163, y + 160,
        15, 30, 255, 180, 69);

    // Window crossbars
    FillBox(x + 105, y + 174,
        15, 3, 27, 22, 34);

    FillBox(x + 163, y + 174,
        15, 3, 27, 22, 34);

    // ============================================
    // FRONT DOOR
    // ============================================

    FillBox(x + 127, y + 205,
        30, 40, 8, 10, 24);

    // Small door handle
    FillBox(x + 149, y + 225,
        4, 4, 210, 155, 72);
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


    RenderStars(renderer);

    // The haunted level never shows the sun.
    // RenderSun(renderer);

    RenderMoonGlow(renderer);

    RenderMoon(renderer);



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


    DrawHauntedBats(
        renderer,
        cameraX,
        animationTime
    );

    // ============================================
    // HAUNTED MANSION - BACKGROUND
    // ============================================

    DrawHauntedMansion(
        renderer,
        cameraX
    );


    // =====================================================
    // FOREGROUND SCENERY
    // =====================================================

    RenderForeground(
        renderer,
        cameraX,
        currentTheme
    );

    RenderFireflies(
        renderer
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

void Environment::SetTerrainDrawColor(
    SDL_Renderer* renderer,
    Uint8 red,
    Uint8 green,
    Uint8 blue
) const
{
    EnvironmentTheme currentTheme =
        GetCurrentTheme();


    Uint8 tintedRed =
        static_cast<Uint8>(
            red *
            currentTheme.terrainTint.r
            );


    Uint8 tintedGreen =
        static_cast<Uint8>(
            green *
            currentTheme.terrainTint.g
            );


    Uint8 tintedBlue =
        static_cast<Uint8>(
            blue *
            currentTheme.terrainTint.b
            );


    SDL_SetRenderDrawColor(
        renderer,
        tintedRed,
        tintedGreen,
        tintedBlue,
        255
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

void Environment::RenderMoonGlow(
    SDL_Renderer* renderer
) const
{

    float fade = 1.0f;



    const float moonX =
        850.0f;

    const float moonY =
        120.0f;


    SDL_Color outerGlow =
    {
        160,
        185,
        235,
        static_cast<Uint8>(
            25.0f * fade
        )
    };


    SDL_Color middleGlow =
    {
        180,
        200,
        245,
        static_cast<Uint8>(
            35.0f * fade
        )
    };


    SDL_Color innerGlow =
    {
        205,
        215,
        250,
        static_cast<Uint8>(
            45.0f * fade
        )
    };


    DrawFilledCircle(
        renderer,
        moonX,
        moonY,
        70.0f,
        outerGlow
    );


    DrawFilledCircle(
        renderer,
        moonX,
        moonY,
        55.0f,
        middleGlow
    );


    DrawFilledCircle(
        renderer,
        moonX,
        moonY,
        43.0f,
        innerGlow
    );
}

void Environment::RenderFireflies(
    SDL_Renderer* renderer
) const
{
    // Only start appearing near the final night section.

    if (playerX < 235.0f)
    {
        return;
    }


    float fade =
        Clamp01(
            (playerX - 235.0f) /
            35.0f
        );


    SDL_Color glowColor =
    {
        220,
        245,
        120,
        static_cast<Uint8>(
            210.0f * fade
        )
    };


    const SDL_FPoint fireflyPositions[] =
    {
        { 120.0f, 420.0f },
        { 230.0f, 500.0f },
        { 360.0f, 390.0f },
        { 510.0f, 470.0f },
        { 680.0f, 410.0f },
        { 810.0f, 510.0f },
        { 960.0f, 430.0f },
        { 1110.0f, 490.0f },
        { 1210.0f, 400.0f }
    };


    const int fireflyCount =
        sizeof(fireflyPositions) /
        sizeof(fireflyPositions[0]);


    for (int i = 0;
        i < fireflyCount;
        ++i)
    {
        float bob =
            std::sin(
                animationTime * 2.0f +
                static_cast<float>(i)
            ) *
            8.0f;


        float drift =
            std::sin(
                animationTime * 1.3f +
                static_cast<float>(i) * 2.0f
            ) *
            5.0f;


        float x =
            fireflyPositions[i].x +
            drift;

        float y =
            fireflyPositions[i].y +
            bob;


        // Small transparent glow.

        SDL_Color outerGlow =
        {
            glowColor.r,
            glowColor.g,
            glowColor.b,
            static_cast<Uint8>(
                45.0f * fade
            )
        };


        DrawFilledCircle(
            renderer,
            x,
            y,
            6.0f,
            outerGlow
        );


        // Bright centre.

        DrawFilledCircle(
            renderer,
            x,
            y,
            2.0f,
            glowColor
        );
    }
}