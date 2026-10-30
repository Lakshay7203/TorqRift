#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>
#include <vector>
#include "Bike.h"
#include "Input.h"
#include "Terrain.h"
#include <algorithm>
#include <cmath>
#include "ParticleSystem.h"
#include "Environment.h"
#include "AIController.h"
#include "Racer.h"
#include <string>
#include <fstream>
#include <sstream>

// ---------------------------------------------------------
// CONSTANTS
// ---------------------------------------------------------

constexpr float SCREEN_WIDTH = 1280.0f;
constexpr float SCREEN_HEIGHT = 720.0f;

constexpr float PIXELS_PER_METER = 40.0f;

constexpr float SCREEN_CENTER_X = SCREEN_WIDTH / 2.0f;
//constexpr float SCREEN_CENTER_Y = SCREEN_HEIGHT / 2.0f;
constexpr float SCREEN_CENTER_Y = 300.0f;

constexpr float CAMERA_TARGET_X = 400.0f;

constexpr float GROUND_HALF_WIDTH = 400.0f;
constexpr float GROUND_HEIGHT = 2.0f;

constexpr float FINISH_X = 315.0f;
constexpr float CHECKPOINT_X = 155.0f;

constexpr float COMBO_WINDOW = 3.0f;


struct RiderDefinition
{
    const char* name;
    bool unlockedByDefault;
    int winsRequired;
};

constexpr RiderDefinition RIDERS[] =
{
    { "GHOST", true, 0 },
    { "RIDER 02", true, 0 }
};

constexpr int RIDER_COUNT =
sizeof(RIDERS) / sizeof(RIDERS[0]);



enum class GameScreen
{
    MainMenu,
    LevelSelect,
    RiderSelect,
    Garage,
    Achievements,
    Controls,
    Race,
    Pause
};


void ProcessInput(
    bool& running,
    InputState& input)
{

    SDL_Event event;

    input.jumpPressed = false;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            running = false;
        }
        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.scancode == SDL_SCANCODE_SPACE &&
                !event.key.repeat)
            {
                input.jumpPressed = true;
            }
        }

    }

    const bool* keyboardState =
        SDL_GetKeyboardState(nullptr);

    input.driveForward =
        keyboardState[SDL_SCANCODE_W];

    input.driveBackward =
        keyboardState[SDL_SCANCODE_S];

    input.leanBackward =
        keyboardState[SDL_SCANCODE_A];

    input.leanForward =
        keyboardState[SDL_SCANCODE_D];

    input.resetPressed =
        keyboardState[SDL_SCANCODE_R];

}

// ---------------------------------------------------------
// DRAW FILLED CIRCLE
// Temporary wheel rendering.
// ---------------------------------------------------------

void DrawFilledCircle(
    SDL_Renderer* renderer,
    float centerX,
    float centerY,
    float radius)
{
    for (float y = -radius; y <= radius; y += 1.0f)
    {
        for (float x = -radius; x <= radius; x += 1.0f)
        {
            if (x * x + y * y <= radius * radius)
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

void DrawRotatedChassis(
    SDL_Renderer* renderer,
    b2BodyId chassisBodyId,
    float cameraX)
{
    // Get the chassis position + rotation from Box2D.
    b2Transform transform =
        b2Body_GetTransform(chassisBodyId);

    // Chassis corners in LOCAL space.
    b2Vec2 localCorners[4] =
    {
        { -BIKE_CHASSIS_WIDTH / 2.0f, -BIKE_CHASSIS_HEIGHT / 2.0f },
        {  BIKE_CHASSIS_WIDTH / 2.0f, -BIKE_CHASSIS_HEIGHT / 2.0f },
        {  BIKE_CHASSIS_WIDTH / 2.0f,  BIKE_CHASSIS_HEIGHT / 2.0f },
        { -BIKE_CHASSIS_WIDTH / 2.0f,  BIKE_CHASSIS_HEIGHT / 2.0f }
    };

    SDL_Vertex vertices[4]{};

    const SDL_FColor green =
    {
        0.0f,
        180.0f / 255.0f,
        0.0f,
        1.0f
    };

    for (int i = 0; i < 4; ++i)
    {
        // Rotate + move each local corner using Box2D.
        b2Vec2 worldPoint =
            b2TransformPoint(
                transform,
                localCorners[i]
            );

        // Convert Box2D world position to SDL screen position.
        vertices[i].position.x =
            CAMERA_TARGET_X +
            (worldPoint.x - cameraX) *
            PIXELS_PER_METER;

        vertices[i].position.y =
            SCREEN_CENTER_Y -
            worldPoint.y *
            PIXELS_PER_METER;

        vertices[i].color = green;
    }

    // Two triangles make one rectangle.
    const int indices[6] =
    {
        0, 1, 2,
        0, 2, 3
    };

    SDL_RenderGeometry(
        renderer,
        nullptr,
        vertices,
        4,
        indices,
        6
    );
}

void DrawFinishLine(
    SDL_Renderer* renderer,
    float cameraX)
{
    const float finishScreenX =
        CAMERA_TARGET_X +
        (FINISH_X - cameraX) *
        PIXELS_PER_METER;

    // Our normal ground surface is y = -9 in Box2D.
    const float groundScreenY =
        SCREEN_CENTER_Y -
        (-9.0f * PIXELS_PER_METER);


    // =============================================
    // FLAG POLE
    // =============================================

    SDL_SetRenderDrawColor(
        renderer,
        40,
        40,
        40,
        255
    );

    SDL_FRect pole =
    {
        finishScreenX,
        groundScreenY - 170.0f,
        6.0f,
        170.0f
    };

    SDL_RenderFillRect(
        renderer,
        &pole
    );


    // =============================================
    // CHECKERED FLAG
    // =============================================

    const float flagWidth = 90.0f;
    const float flagHeight = 60.0f;

    const int columns = 6;
    const int rows = 4;

    const float cellWidth =
        flagWidth / columns;

    const float cellHeight =
        flagHeight / rows;


    for (int row = 0; row < rows; ++row)
    {
        for (int column = 0;
            column < columns;
            ++column)
        {
            bool blackSquare =
                (row + column) % 2 == 0;

            if (blackSquare)
            {
                SDL_SetRenderDrawColor(
                    renderer,
                    20,
                    20,
                    20,
                    255
                );
            }
            else
            {
                SDL_SetRenderDrawColor(
                    renderer,
                    245,
                    245,
                    245,
                    255
                );
            }


            SDL_FRect square =
            {
                finishScreenX + 6.0f +
                    column * cellWidth,

                groundScreenY - 170.0f +
                    row * cellHeight,

                cellWidth,
                cellHeight
            };

            SDL_RenderFillRect(
                renderer,
                &square
            );
        }
    }

    
}


struct SaveData
{
    float bestTime = -1.0f;

    bool hauntedRunCompleted = false;
    bool hauntedRunWon = false;

    int selectedRider = 0;

    int highestUnlockedLevel = 1;
};


bool SaveGame(const SaveData& data)
{
    std::ofstream file("save.txt");

    if (!file.is_open())
    {
        SDL_Log("Failed to open save file for writing.");
        return false;
    }

    file << "TORQRIFT_SAVE_V1\n";

    file << data.bestTime << '\n';
    file << data.hauntedRunCompleted << '\n';
    file << data.hauntedRunWon << '\n';
    file << data.selectedRider << '\n';
    file << data.highestUnlockedLevel << '\n';

    file.flush();

    if (!file.good())
    {
        SDL_Log("Failed to write save data.");
        return false;
    }

    return true;
}


SaveData LoadGame()
{
    SaveData data;

    std::ifstream file("save.txt");

    if (!file.is_open())
    {
        return data;
    }

    std::string header;

    if (!(file >> header))
    {
        return data;
    }

    // Load our new save format.
    if (header == "TORQRIFT_SAVE_V1")
    {
        SaveData loaded;

        int completed = 0;
        int won = 0;

        if (!(file >> loaded.bestTime
            >> completed
            >> won
            >> loaded.selectedRider
            >> loaded.highestUnlockedLevel))
        {
            return data;
        }

        if (!std::isfinite(loaded.bestTime) ||
            (loaded.bestTime != -1.0f &&
                loaded.bestTime <= 0.0f))
        {
            return data;
        }

        loaded.hauntedRunCompleted = completed != 0;
        loaded.hauntedRunWon = won != 0;

        // Only two playable riders exist currently.
        if (loaded.selectedRider < 0 ||
            loaded.selectedRider > 1)
        {
            loaded.selectedRider = 0;
        }

        // Only Haunted Run is playable for now.
        loaded.highestUnlockedLevel = 1;

        return loaded;
    }

    // Support the original save.txt format,
    // which contained only a best-time number.
    std::istringstream oldSave(header);

    float previousBestTime = -1.0f;

    if (oldSave >> previousBestTime &&
        std::isfinite(previousBestTime) &&
        previousBestTime > 0.0f)
    {
        data.bestTime = previousBestTime;
        data.hauntedRunCompleted = true;
    }

    return data;
}


void SaveBestTime(float bestTime)
{
    std::ofstream saveFile("save.txt");

    if (!saveFile.is_open())
    {
        SDL_Log("Failed to open save file!");
        return;
    }

    saveFile << bestTime << '\n';

    saveFile.close();

    SDL_Log("Best time saved!");
}


float LoadBestTime()
{
    std::ifstream saveFile("save.txt");

    // No existing save file.
    if (!saveFile.is_open())
    {
        return -1.0f;
    }

    float savedTime = -1.0f;

    // Read and validate the saved time.
    if (saveFile >> savedTime &&
        std::isfinite(savedTime) &&
        savedTime > 0.0f)
    {
        return savedTime;
    }

    // Invalid or empty save file.
    return -1.0f;
}


void DrawText(
    SDL_Renderer* renderer,
    TTF_Font* font,
    const char* text,
    float x,
    float y,
    SDL_Color color)
{
    SDL_Surface* textSurface =
        TTF_RenderText_Blended(
            font,
            text,
            0,
            color
        );

    if (!textSurface)
    {
        SDL_Log(
            "Text surface creation failed: %s",
            SDL_GetError()
        );

        return;
    }


    SDL_Texture* textTexture =
        SDL_CreateTextureFromSurface(
            renderer,
            textSurface
        );

    if (!textTexture)
    {
        SDL_Log(
            "Text texture creation failed: %s",
            SDL_GetError()
        );

        SDL_DestroySurface(textSurface);
        return;
    }


    SDL_FRect destination =
    {
        x,
        y,
        static_cast<float>(textSurface->w),
        static_cast<float>(textSurface->h)
    };


    SDL_RenderTexture(
        renderer,
        textTexture,
        nullptr,
        &destination
    );


    SDL_DestroyTexture(textTexture);
    SDL_DestroySurface(textSurface);
}

SDL_Color GetComboColor(int comboCount)
{
    if (comboCount <= 1)
    {
        return { 255, 255, 255, 255 };
    }

    if (comboCount == 2)
    {
        return { 255, 220, 90, 255 };
    }

    if (comboCount == 3)
    {
        return { 255, 150, 70, 255 };
    }

    return { 255, 90, 160, 255 };
}

void DrawUI(
    SDL_Renderer* renderer,
    TTF_Font* font,
    TTF_Font* stuntFont,
    bool levelComplete,
    float levelTime,
    const char* stuntText,
    float stuntTextTimer,
    const char* landingText,
    float landingTextTimer,
    int comboCount,
    float comboTimer,
    int score,
    float bestTime,
    bool newBestTime,
    float boostMeter,
    bool boostActive,
    int playerRacePosition)
{
    // =====================================================
    // TEXT DATA
    // =====================================================

    char timerText[64];

    int minutes =
        static_cast<int>(levelTime) / 60;

    float seconds =
        levelTime -
        static_cast<float>(minutes * 60);

    SDL_snprintf(
        timerText,
        sizeof(timerText),
        "TIME  %02d:%05.2f",
        minutes,
        seconds
    );


    char scoreText[64];

    SDL_snprintf(
        scoreText,
        sizeof(scoreText),
        "SCORE  %d",
        score
    );


    // Keep boost percentage safe between 0 and 100.

    float displayBoost =
        boostMeter;

    if (displayBoost < 0.0f)
    {
        displayBoost = 0.0f;
    }

    if (displayBoost > 100.0f)
    {
        displayBoost = 100.0f;
    }


    char boostText[64];

    SDL_snprintf(
        boostText,
        sizeof(boostText),
        boostActive
        ? "BOOST ACTIVE!  %d%%"
        : "BOOST  %d%%",
        static_cast<int>(displayBoost)
    );


    // =====================================================
    // COLORS
    // =====================================================

    SDL_Color white =
    {
        255,
        255,
        255,
        255
    };


    // =====================================================
// NORMAL GAME HUD
// =====================================================

    if (!levelComplete)
    {
        SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_BLEND
        );


        // =====================================================
        // CLEAN TOP HUD
        // =====================================================

        const float hudMargin = 20.0f;
        const float topY = 16.0f;


        // =====================================================
        // TIMER - TOP LEFT
        // =====================================================

        const float timerPanelWidth = 175.0f;
        const float timerPanelHeight = 54.0f;


        SDL_FRect timerPanel =
        {
            hudMargin,
            topY,
            timerPanelWidth,
            timerPanelHeight
        };


        SDL_SetRenderDrawColor(
            renderer,
            30,
            35,
            40,
            220
        );

        SDL_RenderFillRect(
            renderer,
            &timerPanel
        );


        SDL_SetRenderDrawColor(
            renderer,
            210,
            210,
            210,
            220
        );

        SDL_RenderRect(
            renderer,
            &timerPanel
        );


        DrawText(
            renderer,
            font,
            timerText,
            timerPanel.x + 14.0f,
            timerPanel.y + 12.0f,
            white
        );


        // =====================================================
        // BOOST METER - TOP CENTER
        // =====================================================

        const float boostWidth = 260.0f;
        const float boostHeight = 16.0f;

        const float boostX =
            SCREEN_WIDTH / 2.0f -
            boostWidth / 2.0f;

        const float boostLabelY =
            topY - 2.0f;

        const float boostBarY =
            topY + 34.0f;


        DrawText(
            renderer,
            font,
            "BOOST",
            SCREEN_WIDTH / 2.0f - 35.0f,
            boostLabelY,
            white
        );


        float boostPercent =
            displayBoost /
            100.0f;


        if (boostPercent < 0.0f)
        {
            boostPercent = 0.0f;
        }

        if (boostPercent > 1.0f)
        {
            boostPercent = 1.0f;
        }


        SDL_FRect boostBackground =
        {
            boostX,
            boostBarY,
            boostWidth,
            boostHeight
        };


        SDL_SetRenderDrawColor(
            renderer,
            30,
            30,
            30,
            230
        );

        SDL_RenderFillRect(
            renderer,
            &boostBackground
        );


        SDL_FRect boostFill =
        {
            boostX + 2.0f,
            boostBarY + 2.0f,

            (boostWidth - 4.0f) *
            boostPercent,

            boostHeight - 4.0f
        };


        if (boostActive)
        {
            SDL_SetRenderDrawColor(
                renderer,
                255,
                225,
                60,
                255
            );
        }
        else
        {
            SDL_SetRenderDrawColor(
                renderer,
                75,
                170,
                255,
                255
            );
        }


        SDL_RenderFillRect(
            renderer,
            &boostFill
        );


        SDL_SetRenderDrawColor(
            renderer,
            210,
            210,
            210,
            255
        );

        SDL_RenderRect(
            renderer,
            &boostBackground
        );


        // =====================================================
        // SCORE - TOP RIGHT
        // =====================================================

        const float scorePanelWidth = 150.0f;
        const float scorePanelHeight = 54.0f;

        const float scorePanelX =
            SCREEN_WIDTH -
            hudMargin -
            scorePanelWidth;


        SDL_FRect scorePanel =
        {
            scorePanelX,
            topY,
            scorePanelWidth,
            scorePanelHeight
        };


        SDL_SetRenderDrawColor(
            renderer,
            30,
            35,
            40,
            220
        );

        SDL_RenderFillRect(
            renderer,
            &scorePanel
        );


        SDL_SetRenderDrawColor(
            renderer,
            210,
            210,
            210,
            220
        );

        SDL_RenderRect(
            renderer,
            &scorePanel
        );


        DrawText(
            renderer,
            font,
            scoreText,
            scorePanelX + 18.0f,
            topY + 12.0f,
            white
        );
    }


    // =====================================================
    // LEVEL COMPLETE SCREEN
    // =====================================================

    else
    {
        SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_BLEND
        );


        SDL_FRect completePanel =
        {
            SCREEN_WIDTH / 2.0f - 220.0f,
            55.0f,
            440.0f,
            250.0f
        };


        SDL_SetRenderDrawColor(
            renderer,
            20,
            30,
            40,
            190
        );

        SDL_RenderFillRect(
            renderer,
            &completePanel
        );


        SDL_SetRenderDrawColor(
            renderer,
            255,
            255,
            255,
            100
        );

        SDL_RenderRect(
            renderer,
            &completePanel
        );


        DrawText(
            renderer,
            stuntFont,
            "LEVEL COMPLETE!",
            SCREEN_WIDTH / 2.0f - 135.0f,
            80.0f,
            white
        );


        char finishTimeText[64];
        char bestTimeText[64];


        int finishMinutes =
            static_cast<int>(levelTime) / 60;

        float finishSeconds =
            levelTime -
            static_cast<float>(
                finishMinutes * 60
                );


        int bestMinutes =
            static_cast<int>(bestTime) / 60;

        float bestSeconds =
            bestTime -
            static_cast<float>(
                bestMinutes * 60
                );


        SDL_snprintf(
            finishTimeText,
            sizeof(finishTimeText),
            "FINISH  %02d:%05.2f",
            finishMinutes,
            finishSeconds
        );


        SDL_snprintf(
            bestTimeText,
            sizeof(bestTimeText),
            "BEST    %02d:%05.2f",
            bestMinutes,
            bestSeconds
        );


        DrawText(
            renderer,
            font,
            finishTimeText,
            SCREEN_WIDTH / 2.0f - 115.0f,
            145.0f,
            white
        );


        DrawText(
            renderer,
            font,
            bestTimeText,
            SCREEN_WIDTH / 2.0f - 115.0f,
            180.0f,
            white
        );


        if (newBestTime)
        {
            DrawText(
                renderer,
                stuntFont,
                "NEW BEST!",
                SCREEN_WIDTH / 2.0f - 80.0f,
                215.0f,
                white
            );
        }


        DrawText(
            renderer,
            font,
            "R - RESTART",
            SCREEN_WIDTH / 2.0f - 85.0f,
            265.0f,
            white
        );
    }

    // =====================================================
    // COMBO HUD
    // =====================================================

    if (!levelComplete &&
        comboCount > 0 &&
        comboTimer > 0.0f)
    {
        char comboText[64];

        SDL_Color comboColor =
        {
            255,
            220,
            90,
            255
        };

        SDL_snprintf(
            comboText,
            sizeof(comboText),
            "COMBO x%d",
            comboCount,
            comboColor
        );


        DrawText(
            renderer,
            font,
            comboText,
            SCREEN_WIDTH / 2.0f - 65.0f,
            215.0f,
            comboColor
        );

        float comboPercent =
            comboTimer /
            COMBO_WINDOW;

        if (comboPercent < 0.30f)
        {
            comboColor =
            {
                255,
                80,
                80,
                255
            };
        }

        if (comboPercent < 0.0f)
        {
            comboPercent = 0.0f;
        }


        if (comboPercent > 1.0f)
        {
            comboPercent = 1.0f;
        }

        const float comboBarWidth =
            160.0f;

        const float comboBarHeight =
            8.0f;


        SDL_FRect comboBarBackground =
        {
            SCREEN_WIDTH / 2.0f -
                comboBarWidth / 2.0f,

            250.0f,

            comboBarWidth,

            comboBarHeight
        };


        SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_BLEND
        );


        SDL_SetRenderDrawColor(
            renderer,
            25,
            25,
            25,
            180
        );


        SDL_RenderFillRect(
            renderer,
            &comboBarBackground
        );

        SDL_FRect comboBarFill =
        {
            comboBarBackground.x,

            comboBarBackground.y,

            comboBarWidth *
                comboPercent,

            comboBarHeight
        };


        SDL_SetRenderDrawColor(
            renderer,
            comboColor.r,
            comboColor.g,
            comboColor.b,
            comboColor.a
        );


        SDL_RenderFillRect(
            renderer,
            &comboBarFill
        );
    }

    SDL_Color comboColor =
        GetComboColor(
            comboCount
        );

    // =====================================================
    // STUNT POPUP
    // =====================================================

    if (stuntTextTimer > 0.0f)
    {
        // Shadow
        DrawText(
            renderer,
            stuntFont,
            stuntText,
            SCREEN_WIDTH / 2.0f - 107.0f,
            108.0f,
            { 20, 20, 20, 200 }
        );


        // Main text
        DrawText(
            renderer,
            stuntFont,
            stuntText,
            SCREEN_WIDTH / 2.0f - 110.0f,
            105.0f,
            comboColor
        );
    }

    if (landingTextTimer > 0.0f)
    {
        SDL_Color landingColor =
        {
            120,
            255,
            170,
            255
        };

        DrawText(
            renderer,
            font,
            landingText,
            SCREEN_WIDTH / 2.0f - 112.0f,
            168.0f,
            { 20, 20, 20, 180 }
        );


        DrawText(
            renderer,
            font,
            landingText,
            SCREEN_WIDTH / 2.0f - 115.0f,
            165.0f,
            landingColor
        );
    }

}

void DrawCheckpoint(
    SDL_Renderer* renderer,
    float cameraX,
    bool checkpointReached)
{
    // Convert checkpoint world X into screen X.
    const float checkpointScreenX =
        CAMERA_TARGET_X +
        (CHECKPOINT_X - cameraX) *
        PIXELS_PER_METER;

    const float checkpointGroundY =
        SCREEN_CENTER_Y -
        (-9.0f * PIXELS_PER_METER);

    // Yellow before reaching it, green afterwards.
    if (checkpointReached)
    {
        SDL_SetRenderDrawColor(
            renderer,
            50,
            220,
            80,
            255
        );
    }
    else
    {
        SDL_SetRenderDrawColor(
            renderer,
            255,
            200,
            40,
            255
        );
    }

    SDL_FRect pole =
    {
        checkpointScreenX,
        checkpointGroundY - 110.0f,
        6.0f,
        110.0f
    };

    SDL_RenderFillRect(
        renderer,
        &pole
    );

    // Small flag.
    SDL_FRect flag =
    {
        checkpointScreenX + 6.0f,
        checkpointGroundY - 110.0f,
        55.0f,
        30.0f
    };

    SDL_RenderFillRect(
        renderer,
        &flag
    );
}


void DrawGroundFog(
    SDL_Renderer* renderer,
    float cameraX,
    const std::vector<TerrainSegment>& terrainSegments)
{
   

    const float time =
        static_cast<float>(SDL_GetTicksNS()) /
        1000000000.0f;

    constexpr int SEGMENTS = 16;
    constexpr float TWO_PI = 6.2831853f;

    SDL_SetRenderDrawBlendMode(
        renderer, SDL_BLENDMODE_BLEND
    );


    for (float worldX = 0.0f;
        worldX <= FINISH_X;
        worldX += 10.0f)

    {
        float groundY = 0.0f;
        bool foundGround = false;

        // Find the height of the track.
        for (const TerrainSegment& segment : terrainSegments)
        {
            float dx = segment.end.x - segment.start.x;

            if (std::fabs(dx) < 0.001f)
                continue;

            float t = (worldX - segment.start.x) / dx;

            if (t >= 0.0f && t <= 1.0f)
            {
                groundY =
                    segment.start.y +
                    (segment.end.y - segment.start.y) * t;

                foundGround = true;
                break;
            }
        }

        if (!foundGround)
            continue;

        float screenX =
            CAMERA_TARGET_X +
            (worldX - cameraX) * PIXELS_PER_METER;

        float screenY =
            SCREEN_CENTER_Y -
            groundY * PIXELS_PER_METER;

        if (screenX < -170.0f ||
            screenX > SCREEN_WIDTH + 170.0f)
            continue;

        // Give each fog patch a different motion.
        float phase = worldX * 0.17f;

        float drift =
            std::sin(time * 0.45f + phase) * 18.0f;

        float breathing =
            std::sin(time * 0.7f + phase) * 0.12f;

        // Multiple overlapping wisps.
        for (int layer = 0; layer < 3; ++layer)
        {
            float centerX =
                screenX + drift + layer * 25.0f;

            float centerY =
                screenY - 12.0f - layer * 9.0f;

            float radiusX =
                65.0f + layer * 15.0f;

            float radiusY =
                12.0f + layer * 5.0f +
                breathing * 5.0f;

            SDL_Vertex vertices[SEGMENTS + 1]{};
            int indices[SEGMENTS * 3]{};

            // Soft blue-gray fog center.
            vertices[0].position =
            { centerX, centerY };

            vertices[0].color =
            { 0.65f, 0.74f, 0.88f, 0.16f };

            // Transparent edges.
            for (int i = 0; i < SEGMENTS; ++i)
            {
                float angle =
                    TWO_PI * i / SEGMENTS;

                vertices[i + 1].position =
                {
                    centerX + std::cos(angle) * radiusX,
                    centerY + std::sin(angle) * radiusY
                };

                vertices[i + 1].color =
                { 0.65f, 0.74f, 0.88f, 0.0f };

                indices[i * 3] = 0;
                indices[i * 3 + 1] = i + 1;
                indices[i * 3 + 2] =
                    ((i + 1) % SEGMENTS) + 1;
            }

            SDL_RenderGeometry(
                renderer,
                nullptr,
                vertices,
                SEGMENTS + 1,
                indices,
                SEGMENTS * 3
            );
        }
    }
}


void DrawHauntedLanterns(
    SDL_Renderer* renderer,
    float cameraX,
    const std::vector<TerrainSegment>& terrainSegments)
{
    const float lanternPositions[] =
    {
        29.0f, 85.0f, 132.0f,
        177.0f, 225.0f, 273.0f,
        309.0f
    };

    SDL_SetRenderDrawBlendMode(
        renderer, SDL_BLENDMODE_BLEND
    );

    for (float worldX : lanternPositions)
    {
        for (const TerrainSegment& segment : terrainSegments)
        {
            float minX = std::min(
                segment.start.x, segment.end.x);

            float maxX = std::max(
                segment.start.x, segment.end.x);

            float dx = segment.end.x - segment.start.x;

            if (worldX < minX || worldX > maxX ||
                std::fabs(dx) < 0.001f)
                continue;

            float t =
                (worldX - segment.start.x) / dx;

            float worldY =
                segment.start.y +
                (segment.end.y - segment.start.y) * t;

            float x = CAMERA_TARGET_X +
                (worldX - cameraX) * PIXELS_PER_METER;

            float y = SCREEN_CENTER_Y -
                worldY * PIXELS_PER_METER;

            if (x < -70.0f || x > SCREEN_WIDTH + 70.0f)
                break;

            // Soft warm glow.
            SDL_SetRenderDrawColor(
                renderer, 255, 148, 40, 30);

            SDL_FRect outerGlow =
            {
                x - 28.0f, y - 94.0f,
                56.0f, 56.0f
            };

            SDL_RenderFillRect(renderer, &outerGlow);

            SDL_SetRenderDrawColor(
                renderer, 255, 175, 60, 55);

            SDL_FRect innerGlow =
            {
                x - 17.0f, y - 83.0f,
                34.0f, 34.0f
            };

            SDL_RenderFillRect(renderer, &innerGlow);

            // Wooden post.
            SDL_SetRenderDrawColor(
                renderer, 48, 34, 32, 255);

            SDL_FRect post =
            {
                x - 3.0f, y - 70.0f,
                6.0f, 70.0f
            };

            SDL_RenderFillRect(renderer, &post);

            // Lantern frame.
            SDL_SetRenderDrawColor(
                renderer, 22, 20, 28, 255);

            SDL_FRect frame =
            {
                x - 11.0f, y - 78.0f,
                22.0f, 28.0f
            };

            SDL_RenderFillRect(renderer, &frame);

            // Glowing glass.
            SDL_SetRenderDrawColor(
                renderer, 255, 171, 48, 255);

            SDL_FRect glass =
            {
                x - 7.0f, y - 74.0f,
                14.0f, 19.0f
            };

            SDL_RenderFillRect(renderer, &glass);

            // Bright flame core.
            SDL_SetRenderDrawColor(
                renderer, 255, 237, 145, 255);

            SDL_FRect flame =
            {
                x - 3.0f, y - 69.0f,
                6.0f, 12.0f
            };

            SDL_RenderFillRect(renderer, &flame);
            break;
        }
    }
}



void DrawBrokenFences(
    SDL_Renderer* renderer,
    float cameraX,
    const std::vector<TerrainSegment>& terrainSegments)
{
    // Four small fence groups across the level.

    const float fencePositions[] =
    {
        20.0f, 46.0f, 72.0f,
        96.0f, 119.0f, 140.0f,
        164.0f, 187.0f, 212.0f,
        238.0f, 261.0f, 297.0f
    };


    // Find the terrain height at any world X.
    auto GetGroundY = [&](float worldX, float& worldY)
        {
            for (const TerrainSegment& segment : terrainSegments)
            {
                float minX = std::min(
                    segment.start.x, segment.end.x);

                float maxX = std::max(
                    segment.start.x, segment.end.x);

                float dx = segment.end.x - segment.start.x;

                if (worldX < minX || worldX > maxX ||
                    std::fabs(dx) < 0.001f)
                    continue;

                float t = (worldX - segment.start.x) / dx;

                worldY = segment.start.y +
                    (segment.end.y - segment.start.y) * t;

                return true;
            }

            return false;
        };


    const int fenceGroupCount =
        static_cast<int>(
            sizeof(fencePositions) /
            sizeof(fencePositions[0])
            );

    for (int group = 0; group < fenceGroupCount; ++group)

    {
        SDL_FPoint posts[4]{};
        bool valid = true;

        // Four posts per fence group.
        for (int i = 0; i < 4; ++i)
        {
            float worldX =
                fencePositions[group] + i * 0.9f;

            float worldY = 0.0f;

            if (!GetGroundY(worldX, worldY))
            {
                valid = false;
                break;
            }

            posts[i].x = CAMERA_TARGET_X +
                (worldX - cameraX) * PIXELS_PER_METER;

            posts[i].y = SCREEN_CENTER_Y -
                worldY * PIXELS_PER_METER;
        }

        if (!valid)
            continue;

        if (posts[3].x < -100.0f ||
            posts[0].x > SCREEN_WIDTH + 100.0f)
            continue;

        // =====================================
        // BROKEN HORIZONTAL PLANKS
        // =====================================

        SDL_SetRenderDrawColor(
            renderer, 83, 58, 45, 255);

        for (int i = 0; i < 3; ++i)
        {
            // Some boards are missing.
            bool broken =
                (group + i) % 4 == 1;

            if (!broken)
            {
                SDL_RenderLine(
                    renderer,
                    posts[i].x,
                    posts[i].y - 28.0f,
                    posts[i + 1].x,
                    posts[i + 1].y - 28.0f
                );
            }

            // Lower plank.
            if ((group + i) % 3 != 2)
            {
                SDL_RenderLine(
                    renderer,
                    posts[i].x,
                    posts[i].y - 15.0f,
                    posts[i + 1].x,
                    posts[i + 1].y - 15.0f
                );
            }
        }

        // =====================================
        // CROOKED FENCE POSTS
        // =====================================

        SDL_SetRenderDrawColor(
            renderer, 52, 38, 34, 255);

        for (int i = 0; i < 4; ++i)
        {
            float postHeight =
                32.0f + (i % 2) * 7.0f;

            float lean =
                (i % 2 == 0) ? -4.0f : 3.0f;

            for (int thickness = -2;
                thickness <= 2;
                ++thickness)
            {
                SDL_RenderLine(
                    renderer,
                    posts[i].x + thickness,
                    posts[i].y,
                    posts[i].x + lean + thickness,
                    posts[i].y - postHeight
                );
            }
        }
    }
}


void DrawDeadTrees(
    SDL_Renderer* renderer,
    float cameraX,
    const std::vector<TerrainSegment>& terrainSegments)
{

    const float treePositions[] =
    {
        12.0f, 34.0f, 56.0f, 78.0f,
        103.0f, 125.0f, 148.0f,
        174.0f, 196.0f, 220.0f,
        244.0f, 266.0f, 288.0f,
        307.0f
    };


    for (float worldX : treePositions)
    {
        for (const TerrainSegment& segment : terrainSegments)
        {
            float minX = std::min(segment.start.x, segment.end.x);
            float maxX = std::max(segment.start.x, segment.end.x);

            if (worldX < minX || worldX > maxX)
                continue;

            float dx = segment.end.x - segment.start.x;

            if (std::fabs(dx) < 0.001f)
                continue;

            float t = (worldX - segment.start.x) / dx;

            float worldY =
                segment.start.y +
                (segment.end.y - segment.start.y) * t;

            float x =
                CAMERA_TARGET_X +
                (worldX - cameraX) * PIXELS_PER_METER;

            float y =
                SCREEN_CENTER_Y - worldY * PIXELS_PER_METER;

            if (x < -100.0f || x > SCREEN_WIDTH + 100.0f)
                break;

            // Dark tree silhouette.
            SDL_SetRenderDrawColor(
                renderer, 13, 16, 31, 255
            );

            // Main trunk, slightly crooked.
            for (int i = -3; i <= 3; ++i)
            {
                SDL_RenderLine(
                    renderer,
                    x + i, y,
                    x - 9.0f + i, y - 48.0f
                );

                SDL_RenderLine(
                    renderer,
                    x - 9.0f + i, y - 48.0f,
                    x + 3.0f + i, y - 100.0f
                );
            }

            // Left branches.
            SDL_RenderLine(renderer,
                x - 9, y - 52,
                x - 37, y - 74);

            SDL_RenderLine(renderer,
                x - 37, y - 74,
                x - 53, y - 102);

            SDL_RenderLine(renderer,
                x - 37, y - 74,
                x - 67, y - 80);

            // Right branches.
            SDL_RenderLine(renderer,
                x - 3, y - 70,
                x + 29, y - 93);

            SDL_RenderLine(renderer,
                x + 29, y - 93,
                x + 52, y - 121);

            SDL_RenderLine(renderer,
                x + 29, y - 93,
                x + 63, y - 95);

            // Higher twisted branches.
            SDL_RenderLine(renderer,
                x + 3, y - 100,
                x - 18, y - 126);

            SDL_RenderLine(renderer,
                x + 3, y - 100,
                x + 15, y - 132);

            break;
        }
    }
}



void DrawGraveyardProps(
    SDL_Renderer* renderer,
    float cameraX,
    const std::vector<TerrainSegment>& terrainSegments)
{
    // World positions of our gravestones.
    const float gravePositions[] =
    {
        22.0f, 61.0f, 97.0f,
        161.0f, 199.0f, 263.0f,
        301.0f
    };

    for (float worldX : gravePositions)
    {
        // Find the ground height at this location.
        for (const TerrainSegment& segment : terrainSegments)
        {
            if (worldX < segment.start.x ||
                worldX > segment.end.x)
            {
                continue;
            }

            float t =
                (worldX - segment.start.x) /
                (segment.end.x - segment.start.x);

            float worldY =
                segment.start.y +
                (segment.end.y - segment.start.y) * t;

            // Convert Box2D coordinates to screen.
            float x =
                CAMERA_TARGET_X +
                (worldX - cameraX) * PIXELS_PER_METER;

            float y =
                SCREEN_CENTER_Y -
                worldY * PIXELS_PER_METER;

            // Don't draw off-screen props.
            if (x < -60.0f || x > SCREEN_WIDTH + 60.0f)
                break;

            // Dark outline.
            SDL_SetRenderDrawColor(
                renderer, 16, 19, 32, 255
            );

            SDL_FRect outline =
            {
                x - 16.0f,
                y - 42.0f,
                32.0f,
                42.0f
            };

            SDL_RenderFillRect(renderer, &outline);

            // Stone body.
            SDL_SetRenderDrawColor(
                renderer, 78, 85, 105, 255
            );

            SDL_FRect stone =
            {
                x - 12.0f,
                y - 38.0f,
                24.0f,
                35.0f
            };

            SDL_RenderFillRect(renderer, &stone);

            // Rounded-looking top.
            SDL_FRect stoneTop =
            {
                x - 8.0f,
                y - 44.0f,
                16.0f,
                10.0f
            };

            SDL_RenderFillRect(renderer, &stoneTop);

            // Engraved cross.
            SDL_SetRenderDrawColor(
                renderer, 35, 40, 57, 255
            );

            SDL_FRect vertical =
            {
                x - 2.0f,
                y - 32.0f,
                4.0f,
                20.0f
            };

            SDL_FRect horizontal =
            {
                x - 8.0f,
                y - 26.0f,
                16.0f,
                4.0f
            };

            SDL_RenderFillRect(renderer, &vertical);
            SDL_RenderFillRect(renderer, &horizontal);

            break;
        }
    }
}


void DrawWheelSprite(
    SDL_Renderer* renderer,
    SDL_Texture* wheelTexture,
    b2BodyId wheelBodyId,
    const SDL_FPoint& wheelScreen,
    float wheelVisualScale = 1.55f)
{
    float wheelAngleRadians =
        b2Rot_GetAngle(
            b2Body_GetRotation(wheelBodyId)
        );

    constexpr float RAD_TO_DEG =
        57.2957795f;

    float wheelAngleDegrees =
        -wheelAngleRadians * RAD_TO_DEG;

    // Physical Box2D wheel diameter.
    float physicsDiameter =
        BIKE_WHEEL_RADIUS *
        2.0f *
        PIXELS_PER_METER;


    float wheelSize =
        physicsDiameter *
        wheelVisualScale;

    SDL_FRect wheelRect =
    {
        wheelScreen.x - wheelSize / 2.0f,
        wheelScreen.y - wheelSize / 2.0f,
        wheelSize,
        wheelSize
    };

    SDL_RenderTextureRotated(
        renderer,
        wheelTexture,
        nullptr,
        &wheelRect,
        wheelAngleDegrees,
        nullptr,
        SDL_FLIP_NONE
    );
}

struct RacerVisual
{
    SDL_Texture* bikeTexture = nullptr;
    SDL_Texture* wheelTexture = nullptr;

    float artWidth = 0.0f;
    float artHeight = 0.0f;

    float rearAxleX = 0.0f;
    float rearAxleY = 0.0f;

    float frontAxleX = 0.0f;
    float frontAxleY = 0.0f;

    float bodyHeightScale = 1.0f;

    const char* name = "";
};

void DrawBikeSprite(
    SDL_Renderer* renderer,
    const RacerVisual& visual,
    const SDL_FPoint& rearWheelScreen,
    const SDL_FPoint& frontWheelScreen)
{
    constexpr float RAD_TO_DEG =
        57.2957795f;


    // =====================================================
    // PHYSICS WHEEL DISTANCE
    // =====================================================

    float screenDX =
        frontWheelScreen.x -
        rearWheelScreen.x;

    float screenDY =
        frontWheelScreen.y -
        rearWheelScreen.y;


    float screenWheelDistance =
        std::sqrt(
            screenDX * screenDX +
            screenDY * screenDY
        );


    // =====================================================
    // ARTWORK AXLE DISTANCE
    // =====================================================

    float artDX =
        visual.frontAxleX -
        visual.rearAxleX;

    float artDY =
        (visual.frontAxleY - visual.rearAxleY) *
        visual.bodyHeightScale;


    float artWheelDistance =
        std::sqrt(
            artDX * artDX +
            artDY * artDY
        );


    if (artWheelDistance <= 0.001f)
    {
        return;
    }


    float scale =
        screenWheelDistance /
        artWheelDistance;


    // =====================================================
    // SIZE IMAGE
    // =====================================================

    SDL_FRect bikeRect;

    bikeRect.w =
        visual.artWidth *
        scale;

    bikeRect.h =
        visual.artHeight *
        scale *
        visual.bodyHeightScale;


    // =====================================================
    // ART AXLE MIDPOINT
    // =====================================================

    float artMidX =
        (
            visual.rearAxleX +
            visual.frontAxleX
            ) /
        2.0f;


    float artMidY =
        (
            visual.rearAxleY +
            visual.frontAxleY
            ) /
        2.0f;


    // =====================================================
    // PHYSICS WHEEL MIDPOINT
    // =====================================================

    float screenMidX =
        (
            rearWheelScreen.x +
            frontWheelScreen.x
            ) /
        2.0f;


    float screenMidY =
        (
            rearWheelScreen.y +
            frontWheelScreen.y
            ) /
        2.0f;


    // =====================================================
    // ROTATION PIVOT
    // =====================================================

    SDL_FPoint pivot =
    {
        artMidX * scale,
        artMidY * scale * visual.bodyHeightScale
    };

    bikeRect.x =
        screenMidX -
        pivot.x;

    bikeRect.y =
        screenMidY -
        pivot.y;


    // =====================================================
    // MATCH ART ANGLE TO PHYSICS
    // =====================================================

    float screenAngle =
        std::atan2(
            screenDY,
            screenDX
        ) *
        RAD_TO_DEG;


    float artAngle =
        std::atan2(
            artDY,
            artDX
        ) *
        RAD_TO_DEG;


    float finalAngle =
        screenAngle -
        artAngle;


    // =====================================================
    // DRAW
    // =====================================================

    SDL_RenderTextureRotated(
        renderer,
        visual.bikeTexture,
        nullptr,
        &bikeRect,
        finalAngle,
        &pivot,
        SDL_FLIP_NONE
    );
}

void DrawRacePositionBadge(
    SDL_Renderer* renderer,
    TTF_Font* font,
    float centerX,
    float centerY,
    int position)
{
    if (position <= 0)
    {
        return;
    }

    // =====================================================
    // BADGE DIMENSIONS
    // =====================================================

    constexpr float width = 66.0f;
    constexpr float height = 48.0f;
    constexpr float cornerCut = 9.0f;

    constexpr float borderSize = 4.0f;

    constexpr float pointerWidth = 18.0f;
    constexpr float pointerHeight = 13.0f;


    // =====================================================
    // HELPER: DRAW ANGLED RECTANGLE
    // =====================================================

    auto DrawAngledBox =
        [&](float x,
            float y,
            float w,
            float h,
            float cut,
            SDL_FColor color)
        {
            SDL_Vertex vertices[9]{};

            // Center vertex.
            vertices[0].position =
            {
                x + w / 2.0f,
                y + h / 2.0f
            };

            // Top-left angled point.
            vertices[1].position =
            {
                x + cut,
                y
            };

            // Top-right.
            vertices[2].position =
            {
                x + w - cut,
                y
            };

            vertices[3].position =
            {
                x + w,
                y + cut
            };

            // Bottom-right.
            vertices[4].position =
            {
                x + w,
                y + h - cut
            };

            vertices[5].position =
            {
                x + w - cut,
                y + h
            };

            // Bottom-left.
            vertices[6].position =
            {
                x + cut,
                y + h
            };

            vertices[7].position =
            {
                x,
                y + h - cut
            };

            vertices[8].position =
            {
                x,
                y + cut
            };


            for (SDL_Vertex& vertex : vertices)
            {
                vertex.color = color;
            }


            const int indices[] =
            {
                0, 1, 2,
                0, 2, 3,
                0, 3, 4,
                0, 4, 5,
                0, 5, 6,
                0, 6, 7,
                0, 7, 8,
                0, 8, 1
            };


            SDL_RenderGeometry(
                renderer,
                nullptr,
                vertices,
                9,
                indices,
                24
            );
        };


    // =====================================================
    // OUTER DARK BORDER
    // =====================================================

    const float outerX =
        centerX - width / 2.0f;

    const float outerY =
        centerY - height / 2.0f;


    SDL_FColor darkColor =
    {
        20.0f / 255.0f,
        25.0f / 255.0f,
        30.0f / 255.0f,
        1.0f
    };


    DrawAngledBox(
        outerX,
        outerY,
        width,
        height,
        cornerCut,
        darkColor
    );


    // =====================================================
    // INNER WHITE BADGE
    // =====================================================

    SDL_FColor whiteColor =
    {
        245.0f / 255.0f,
        245.0f / 255.0f,
        245.0f / 255.0f,
        1.0f
    };


    DrawAngledBox(
        outerX + borderSize,
        outerY + borderSize,
        width - borderSize * 2.0f,
        height - borderSize * 2.0f,
        cornerCut - 2.0f,
        whiteColor
    );


    // =====================================================
    // POINTER BORDER
    // =====================================================

    SDL_Vertex pointerBorder[3]{};

    pointerBorder[0].position =
    {
        centerX - pointerWidth / 2.0f - 3.0f,
        outerY + height - 1.0f
    };

    pointerBorder[1].position =
    {
        centerX + pointerWidth / 2.0f + 3.0f,
        outerY + height - 1.0f
    };

    pointerBorder[2].position =
    {
        centerX,
        outerY + height +
        pointerHeight + 4.0f
    };


    for (SDL_Vertex& vertex : pointerBorder)
    {
        vertex.color = darkColor;
    }


    const int triangleIndices[] =
    {
        0, 1, 2
    };


    SDL_RenderGeometry(
        renderer,
        nullptr,
        pointerBorder,
        3,
        triangleIndices,
        3
    );


    // =====================================================
    // WHITE POINTER
    // =====================================================

    SDL_Vertex pointer[3]{};

    pointer[0].position =
    {
        centerX - pointerWidth / 2.0f,
        outerY + height - 1.0f
    };

    pointer[1].position =
    {
        centerX + pointerWidth / 2.0f,
        outerY + height - 1.0f
    };

    pointer[2].position =
    {
        centerX,
        outerY + height +
        pointerHeight
    };


    for (SDL_Vertex& vertex : pointer)
    {
        vertex.color = whiteColor;
    }


    SDL_RenderGeometry(
        renderer,
        nullptr,
        pointer,
        3,
        triangleIndices,
        3
    );


    // =====================================================
    // POSITION NUMBER
    // =====================================================

    char positionText[16];

    SDL_snprintf(
        positionText,
        sizeof(positionText),
        "%d",
        position
    );


    SDL_Color numberColor =
    {
        20,
        25,
        30,
        255
    };


    DrawText(
        renderer,
        font,
        positionText,

        centerX - 11.0f,
        centerY - 24.0f,

        numberColor
    );
}


void DrawArcadeMenuBackground(SDL_Renderer* renderer)
{
    auto FillRect = [&](float x, float y,
        float w, float h, SDL_Color color)
        {
            SDL_SetRenderDrawColor(
                renderer,
                color.r, color.g, color.b, color.a
            );

            SDL_FRect rect = { x, y, w, h };
            SDL_RenderFillRect(renderer, &rect);
        };

    auto Triangle = [&](SDL_FPoint a,
        SDL_FPoint b, SDL_FPoint c,
        SDL_FColor color)
        {
            SDL_Vertex vertices[3]{};

            vertices[0].position = a;
            vertices[1].position = b;
            vertices[2].position = c;

            for (auto& vertex : vertices)
                vertex.color = color;

            const int indices[] = { 0, 1, 2 };

            SDL_RenderGeometry(
                renderer, nullptr,
                vertices, 3, indices, 3
            );
        };

    // ==========================================
    // BLUE SKY
    // ==========================================

    SDL_SetRenderDrawColor(
        renderer, 82, 190, 235, 255
    );
    SDL_RenderClear(renderer);

    FillRect(0, 0, 1280, 230,
        { 62, 169, 228, 255 });

    FillRect(0, 230, 1280, 180,
        { 103, 205, 235, 255 });

    // ==========================================
    // CARTOON CLOUDS
    // ==========================================

    const float clouds[][2] =
    {
        { 380, 85 },
        { 890, 60 },
        { 1100, 135 }
    };

    for (const auto& cloud : clouds)
    {
        FillRect(cloud[0], cloud[1] + 20,
            125, 22, { 247, 251, 255, 255 });

        FillRect(cloud[0] + 20, cloud[1],
            55, 35, { 247, 251, 255, 255 });

        FillRect(cloud[0] + 65, cloud[1] + 10,
            45, 27, { 247, 251, 255, 255 });
    }

    // ==========================================
    // DISTANT MOUNTAINS
    // ==========================================

    const SDL_FColor mountain =
    { 0.54f, 0.72f, 0.82f, 1.0f };

    const SDL_FColor snow =
    { 0.94f, 0.97f, 1.0f, 1.0f };

    for (int i = 0; i < 5; ++i)
    {
        float x = 60.0f + i * 290.0f;
        float top = 185.0f + (i % 2) * 35.0f;

        Triangle(
            { x - 180, 485 },
            { x, top },
            { x + 180, 485 },
            mountain
        );

        Triangle(
            { x - 38, top + 64 },
            { x, top },
            { x + 38, top + 64 },
            snow
        );
    }

    // ==========================================
    // ROLLING GREEN HILLS
    // ==========================================

    const SDL_FColor distantGreen =
    { 0.31f, 0.64f, 0.39f, 1.0f };

    const SDL_FColor nearGreen =
    { 0.17f, 0.44f, 0.28f, 1.0f };

    Triangle(
        { -100, 500 }, { 300, 350 },
        { 700, 500 }, distantGreen
    );

    Triangle(
        { 500, 500 }, { 930, 340 },
        { 1400, 500 }, distantGreen
    );

    FillRect(0, 480, 1280, 125,
        { 55, 126, 77, 255 });

    Triangle(
        { -100, 595 }, { 300, 450 },
        { 700, 595 }, nearGreen
    );

    Triangle(
        { 500, 595 }, { 1080, 450 },
        { 1450, 595 }, nearGreen
    );

    // ==========================================
    // SIMPLE PINE TREES
    // ==========================================

    const float treeX[] =
    {
        45, 110, 675, 730, 1180, 1240
    };

    for (float x : treeX)
    {
        FillRect(x - 4, 455, 8, 60,
            { 76, 54, 37, 255 });

        Triangle(
            { x - 29, 477 },
            { x, 388 },
            { x + 29, 477 },
            { 0.07f, 0.32f, 0.20f, 1.0f }
        );
    }

    // ==========================================
    // ORANGE STUNT TRACK IN DISTANCE
    // ==========================================

    for (int thickness = 0; thickness < 9; ++thickness)
    {
        SDL_SetRenderDrawColor(
            renderer, 240, 123, 36, 255
        );

        for (int x = 655; x < 1280; x += 5)
        {
            float t = (x - 655.0f) / 625.0f;

            float y =
                400.0f -
                65.0f * std::sin(t * 6.2831853f);

            float nextT = (x + 5 - 655.0f) / 625.0f;

            float nextY =
                400.0f -
                65.0f * std::sin(nextT * 6.2831853f);

            SDL_RenderLine(
                renderer,
                static_cast<float>(x),
                y + thickness,
                static_cast<float>(x + 5),
                nextY + thickness
            );
        }
    }
}





void DrawGarageMenu(
    SDL_Renderer* renderer,
    TTF_Font* font,
    TTF_Font* titleFont,
    SDL_Texture* bikeTexture,
    SDL_Texture* wheelTexture,
    int selectedOption)
{
    SDL_SetRenderDrawColor(
        renderer, 0, 0, 0, 255
    );
    SDL_RenderClear(renderer);

    const SDL_Color white =
    { 255, 255, 255, 255 };

    // Center text based on its actual width.
    auto DrawCenteredText =
        [&](TTF_Font* textFont,
            const char* text,
            float y)
        {
            int textWidth = 0;
            int textHeight = 0;

            if (!TTF_GetStringSize(
                textFont,
                text,
                0,
                &textWidth,
                &textHeight))
            {
                return;
            }

            float x =
                (SCREEN_WIDTH - textWidth) / 2.0f;

            DrawText(
                renderer,
                textFont,
                text,
                x,
                y,
                white
            );
        };

    // ==========================================
    // TITLE
    // ==========================================

    DrawCenteredText(
        titleFont,
        "TORQRIFT",
        115.0f
    );

    // ==========================================
    // MENU OPTIONS
    // ==========================================

    const char* menuOptions[] =
    {
        "PLAY",
        "RIDERS",
        "GARAGE",
        "AWARDS",
        "CONTROLS",
        "QUIT"
    };

    for (int i = 0; i < 6; ++i)
    {
        float y = 245.0f + i * 55.0f;

        DrawCenteredText(
            titleFont,
            menuOptions[i],
            y
        );

        if (selectedOption == i)
        {
            int textWidth = 0;
            int textHeight = 0;

            if (TTF_GetStringSize(
                titleFont,
                menuOptions[i],
                0,
                &textWidth,
                &textHeight))
            {
                float textX =
                    (SCREEN_WIDTH - textWidth) / 2.0f;

                DrawText(
                    renderer,
                    titleFont,
                    ">",
                    textX - 35.0f,
                    y,
                    white
                );
            }
        }
    }

    // ==========================================
    // INSTRUCTIONS
    // ==========================================

    DrawCenteredText(
        font,
        "UP / DOWN - SELECT     ENTER - CONFIRM",
        625.0f
    );
}



void DrawRiderSelect(
    SDL_Renderer* renderer,
    TTF_Font* font,
    TTF_Font* titleFont,
    int menuSelection,
    int selectedRider)
{
    SDL_SetRenderDrawColor(
        renderer, 0, 0, 0, 255
    );
    SDL_RenderClear(renderer);

    const SDL_Color white =
    { 255, 255, 255, 255 };

    // Title.
    DrawText(
        renderer, titleFont,
        "SELECT RIDER",
        515.0f, 150.0f, white
    );


    for (int i = 0; i < RIDER_COUNT; ++i)
    {
        char text[64];

        SDL_snprintf(
            text,
            sizeof(text),
            "%s %s%s",
            menuSelection == i ? ">" : " ",
            RIDERS[i].name,
            selectedRider == i ? "  *" : ""
        );

        DrawText(
            renderer,
            titleFont,
            text,
            520.0f,
            300.0f + i * 75.0f,
            white
        );
    }

    DrawText(
        renderer, font,
        "* CURRENT RIDER",
        520.0f, 490.0f, white
    );

    DrawText(
        renderer, font,
        "UP / DOWN - SELECT   ENTER - CONFIRM   ESC - BACK",
        300.0f, 620.0f, white
    );
}




void DrawLevelSelect(
    SDL_Renderer* renderer,
    TTF_Font* font,
    TTF_Font* titleFont)
{
    SDL_SetRenderDrawColor(
        renderer, 0, 0, 0, 255
    );

    SDL_RenderClear(renderer);

    const SDL_Color white =
    { 255, 255, 255, 255 };

    const SDL_Color gray =
    { 130, 130, 130, 255 };

    // =====================================
    // TITLE
    // =====================================

    DrawText(
        renderer,
        titleFont,
        "SELECT LEVEL",
        485.0f, 90.0f,
        white
    );

    // =====================================
    // LEVEL GRID
    // =====================================

    for (int i = 0; i < 10; ++i)
    {
        int column = i % 5;
        int row = i / 5;

        float x = 100.0f + column * 220.0f;
        float y = 220.0f + row * 160.0f;

        bool unlocked = (i == 0);

        SDL_FRect card =
        {
            x, y, 190.0f, 125.0f
        };

        // White outline for unlocked level.
        // Gray outline for locked levels.
        SDL_SetRenderDrawColor(
            renderer,
            unlocked ? 255 : 90,
            unlocked ? 255 : 90,
            unlocked ? 255 : 90,
            255
        );

        SDL_RenderRect(renderer, &card);

        char number[16];

        SDL_snprintf(
            number,
            sizeof(number),
            "LEVEL %02d",
            i + 1
        );

        DrawText(
            renderer,
            font,
            number,
            x + 20.0f,
            y + 18.0f,
            unlocked ? white : gray
        );

        DrawText(
            renderer,
            font,
            unlocked ? "HAUNTED RUN" : "LOCKED",
            x + 20.0f,
            y + 68.0f,
            unlocked ? white : gray
        );
    }

    // =====================================
    // CONTROLS
    // =====================================

    DrawText(
        renderer,
        font,
        "ENTER - PLAY     ESC - BACK",
        435.0f, 630.0f,
        white
    );
}




void DrawRaceResults(
    SDL_Renderer* renderer,
    TTF_Font* font,
    TTF_Font* titleFont,
    const Racer& playerRacer,
    const std::vector<Racer>& aiRacers,
    float bestTime,
    bool newBestTime,
    int resultsMenuSelection)
{
    if (!renderer || !font || !titleFont)
        return;

    // =============================================
    // TORQRIFT RACE CONTROL - COLORS
    // =============================================

    const SDL_Color white = { 242, 247, 251, 255 };
    const SDL_Color muted = { 164, 184, 198, 255 };
    const SDL_Color teal = { 54, 214, 194, 255 };
    const SDL_Color gold = { 245, 197, 86, 255 };

    auto fillRect = [&](float x, float y,
        float w, float h,
        Uint8 r, Uint8 g,
        Uint8 b, Uint8 a)
        {
            SDL_SetRenderDrawColor(
                renderer, r, g, b, a
            );

            SDL_FRect area = { x, y, w, h };
            SDL_RenderFillRect(renderer, &area);
        };

    auto formatTime = [](float seconds,
        char* buffer,
        size_t bufferSize)
        {
            if (seconds < 0.0f)
            {
                SDL_snprintf(
                    buffer, bufferSize, "--:--.--"
                );
                return;
            }

            int centiseconds =
                static_cast<int>(seconds * 100.0f + 0.5f);

            SDL_snprintf(
                buffer,
                bufferSize,
                "%02d:%02d.%02d",
                centiseconds / 6000,
                (centiseconds / 100) % 60,
                centiseconds % 100
            );
        };

    // =============================================
    // COLLECT AND SORT RACERS
    // =============================================

    struct ResultRow
    {
        const Racer* racer;
        const char* name;
        float progressX;
        bool isPlayer;
    };

    std::vector<ResultRow> rows;

    rows.push_back({
        &playerRacer,
        "YOU",
        b2Body_GetPosition(
            playerRacer.bike.chassisBodyId
        ).x,
        true
        });

    const char* aiNames[] = {
    "VOLT",
    "GRAVEL",
    "RIOT"
    };

    bool allFinished = playerRacer.finished;

    for (size_t i = 0; i < aiRacers.size(); ++i)
    {
        const Racer& ai = aiRacers[i];

        rows.push_back({
            &ai,
            i < 3 ? aiNames[i] : "AI RACER",
            b2Body_GetPosition(
                ai.bike.chassisBodyId
            ).x,
            false
            });

        if (!ai.finished)
            allFinished = false;
    }

    std::sort(
        rows.begin(), rows.end(),
        [](const ResultRow& a, const ResultRow& b)
        {
            if (a.racer->finished != b.racer->finished)
                return a.racer->finished;

            if (a.racer->finished)
                return a.racer->finishPlace <
                b.racer->finishPlace;

            return a.progressX > b.progressX;
        }
    );

    // Find the winning time for GAP calculation.
    float leaderTime = -1.0f;

    for (const ResultRow& row : rows)
    {
        if (row.racer->finished &&
            row.racer->finishPlace == 1 &&
            row.racer->finishTime >= 0.0f)
        {
            leaderTime = row.racer->finishTime;
            break;
        }
    }

    // =============================================
    // BACKGROUND
    // =============================================

    SDL_SetRenderDrawBlendMode(
        renderer, SDL_BLENDMODE_BLEND
    );

    fillRect(
        0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
        9, 15, 24, 237
    );

    // =============================================
    // TOP RACE CONTROL HEADER
    // =============================================

    fillRect(
        0, 0, SCREEN_WIDTH, 92,
        19, 29, 42, 255
    );

    fillRect(
        0, 88, SCREEN_WIDTH, 4,
        54, 214, 194, 255
    );

    DrawText(
        renderer, titleFont,
        "TORQRIFT",
        47, 16, white
    );

    DrawText(
        renderer, font,
        "RACE CONTROL / RESULTS",
        365, 29, teal
    );

    DrawText(
        renderer, font,
        allFinished ? "FINAL" : "LIVE",
        1120, 29,
        allFinished ? teal : gold
    );

    // =============================================
    // LEFT PANEL - PLAYER SUMMARY
    // =============================================

    const float leftX = 44.0f;
    const float leftY = 117.0f;
    const float leftW = 340.0f;
    const float panelH = 500.0f;

    fillRect(
        leftX, leftY, leftW, panelH,
        28, 41, 56, 255
    );

    fillRect(
        leftX, leftY, 5, panelH,
        54, 214, 194, 255
    );

    DrawText(
        renderer, font,
        "YOUR RESULT",
        leftX + 27, 144, muted
    );

    // Big position indicator.
    char positionText[16];

    SDL_snprintf(
        positionText,
        sizeof(positionText),
        "P%02d",
        playerRacer.finishPlace
    );

    SDL_Color positionColor =
        playerRacer.finishPlace == 1
        ? gold : teal;

    DrawText(
        renderer, titleFont,
        positionText,
        leftX + 27, 188,
        positionColor
    );

    const char* positionStatus = "RACE COMPLETE";

    if (playerRacer.finishPlace == 1)
        positionStatus = "RACE WINNER";
    else if (playerRacer.finishPlace <= 3)
        positionStatus = "PODIUM FINISH";

    DrawText(
        renderer, font,
        positionStatus,
        leftX + 27, 257, white
    );

    // Separation line.
    fillRect(
        leftX + 27, 313,
        leftW - 54, 2,
        58, 76, 93, 255
    );

    DrawText(
        renderer, font,
        "RACE TIME",
        leftX + 27, 340, muted
    );

    char playerTime[32];
    formatTime(
        playerRacer.finishTime,
        playerTime,
        sizeof(playerTime)
    );

    DrawText(
        renderer, titleFont,
        playerTime,
        leftX + 27, 379, white
    );

    fillRect(
        leftX + 27, 449,
        leftW - 54, 2,
        58, 76, 93, 255
    );

    DrawText(
        renderer, font,
        "PERSONAL BEST",
        leftX + 27, 467, muted
    );

    char bestTimeText[32];
    formatTime(
        bestTime,
        bestTimeText,
        sizeof(bestTimeText)
    );

    DrawText(
        renderer, font,
        bestTimeText,
        leftX + 27, 510, white
    );

    if (newBestTime)
    {
        DrawText(
            renderer, font,
            "NEW RECORD",
            leftX + 27, 558, teal
        );
    }

    // =============================================
    // RIGHT PANEL - CLASSIFICATION
    // =============================================

    const float tableX = 405.0f;
    const float tableY = 117.0f;
    const float tableW = 830.0f;

    fillRect(
        tableX, tableY,
        tableW, panelH,
        28, 41, 56, 255
    );

    DrawText(
        renderer, titleFont,
        "RACE CLASSIFICATION",
        tableX + 23, 131, white
    );

    fillRect(
        tableX + 22, 187,
        tableW - 44, 2,
        58, 76, 93, 255
    );

    // Table column headings.
    DrawText(
        renderer, font,
        "POS", 444, 192, muted
    );

    DrawText(
        renderer, font,
        "RIDER", 547, 192, muted
    );

    DrawText(
        renderer, font,
        "TIME", 925, 192, muted
    );

    DrawText(
        renderer, font,
        "GAP", 1101, 192, muted
    );

    // =============================================
    // RESULTS ROWS
    // =============================================

    const float rowStart = 235.0f;
    const float rowSpacing = 85.0f;

    for (size_t i = 0; i < rows.size(); ++i)
    {
        const ResultRow& row = rows[i];

        float y =
            rowStart +
            static_cast<float>(i) * rowSpacing;

        // Row background.
        if (row.isPlayer)
        {
            fillRect(
                tableX + 17, y,
                tableW - 34, 75,
                34, 77, 78, 255
            );
        }
        else
        {
            fillRect(
                tableX + 17, y,
                tableW - 34, 75,
                38, 54, 70, 255
            );
        }

        // Accent stripe.
        SDL_Color stripeColor =
            row.isPlayer ? teal :
            (row.racer->finishPlace == 1
                ? gold : muted);

        fillRect(
            tableX + 17, y,
            5, 75,
            stripeColor.r,
            stripeColor.g,
            stripeColor.b,
            255
        );

        // Position.
        char rankText[16] = "--";

        if (row.racer->finished)
        {
            SDL_snprintf(
                rankText,
                sizeof(rankText),
                "P%02d",
                row.racer->finishPlace
            );
        }

        DrawText(
            renderer, font,
            rankText,
            445, y + 18,
            row.isPlayer ? teal : white
        );

        // Racer name.
        DrawText(
            renderer, font,
            row.name,
            547, y + 10,
            row.isPlayer ? teal : white
        );

        DrawText(
            renderer, font,
            row.racer->finished
            ? "CLASSIFIED"
            : "ON TRACK",
            547, y + 42, muted
        );

        // Finishing time.
        char timeText[32];

        formatTime(
            row.racer->finishTime,
            timeText,
            sizeof(timeText)
        );

        DrawText(
            renderer, font,
            row.racer->finished
            ? timeText
            : "RACING",
            925, y + 18,
            white
        );

        // Gap to the winner.
        char gapText[32] = "--";

        if (row.racer->finished &&
            leaderTime >= 0.0f)
        {
            if (row.racer->finishPlace == 1)
            {
                SDL_snprintf(
                    gapText,
                    sizeof(gapText),
                    "LEADER"
                );
            }
            else
            {
                SDL_snprintf(
                    gapText,
                    sizeof(gapText),
                    "+%.2f",
                    std::max(
                        0.0f,
                        row.racer->finishTime -
                        leaderTime
                    )
                );
            }
        }

        DrawText(
            renderer, font,
            gapText,
            1101, y + 18,
            row.racer->finishPlace == 1
            ? gold : muted
        );
    }

    // Bottom line within classification panel.
    fillRect(
        tableX + 22, 576,
        tableW - 44, 2,
        58, 76, 93, 255
    );

    DrawText(
        renderer, font,
        allFinished
        ? "ALL RACERS CLASSIFIED"
        : "STANDINGS UPDATE AS RACERS FINISH",
        tableX + 25, 583,
        muted
    );


    // =============================================
    // BOTTOM ACTION BAR
    // =============================================

    fillRect(
        0, 649,
        SCREEN_WIDTH, 71,
        0, 0, 0, 255
    );

    DrawText(
        renderer,
        font,
        "RACE RESULTS",
        45.0f, 670.0f,
        white
    );

    DrawText(
        renderer,
        font,
        resultsMenuSelection == 0
        ? "> RACE AGAIN"
        : "  RACE AGAIN",
        675.0f, 670.0f,
        white
    );

    DrawText(
        renderer,
        font,
        resultsMenuSelection == 1
        ? "> MAIN MENU"
        : "  MAIN MENU",
        975.0f, 670.0f,
        white
    );
}



void DrawRaceMinimap(
    SDL_Renderer* renderer,
    const std::vector<TerrainSegment>& terrainSegments,
    const Racer& playerRacer,
    const std::vector<Racer>& aiRacers)
{
    if (terrainSegments.empty())
        return;


    constexpr float mapWidth = 200.0f;
    constexpr float mapHeight = 65.0f;

    constexpr float mapX = 20.0f;
    constexpr float mapY = 85.0f;

    constexpr float padding = 7.0f;


    // Find the terrain's vertical range.
    float minY = terrainSegments.front().start.y;
    float maxY = minY;

    for (const TerrainSegment& segment : terrainSegments)
    {
        minY = std::min(minY,
            std::min(segment.start.y, segment.end.y));

        maxY = std::max(maxY,
            std::max(segment.start.y, segment.end.y));
    }

    // Avoid dividing by zero on flat terrain.
    float heightRange = std::max(maxY - minY, 1.0f);

    // Convert world coordinates into minimap coordinates.
    auto ToMapX = [&](float worldX)
        {
            float progress =
                std::clamp(worldX / FINISH_X, 0.0f, 1.0f);

            return mapX + padding +
                progress * (mapWidth - padding * 2.0f);
        };

    auto ToMapY = [&](float worldY)
        {
            float height =
                std::clamp(
                    (worldY - minY) / heightRange,
                    0.0f, 1.0f
                );

            return mapY + mapHeight - padding -
                height * (mapHeight - padding * 2.0f);
        };

    // Find the track height underneath a racer.
    auto GetTrackY = [&](float worldX)
        {
            for (const TerrainSegment& segment : terrainSegments)
            {
                float left = std::min(
                    segment.start.x, segment.end.x);

                float right = std::max(
                    segment.start.x, segment.end.x);

                if (worldX < left || worldX > right)
                    continue;

                float dx = segment.end.x - segment.start.x;

                if (std::fabs(dx) < 0.001f)
                    continue;

                float t = (worldX - segment.start.x) / dx;

                return segment.start.y +
                    (segment.end.y - segment.start.y) * t;
            }

            return minY;
        };

    // ========================================
    // BACKGROUND
    // ========================================

    SDL_SetRenderDrawBlendMode(
        renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(
        renderer, 0, 0, 0, 95);

    SDL_FRect background =
    {
        mapX, mapY, mapWidth, mapHeight
    };

    SDL_RenderFillRect(renderer, &background);

    SDL_SetRenderDrawColor(
        renderer, 140, 140, 140, 130);

    SDL_RenderRect(renderer, &background);

    // ========================================
    // TERRAIN PROFILE
    // ========================================

    SDL_SetRenderDrawColor(
        renderer, 245, 245, 245, 255);

    for (const TerrainSegment& segment : terrainSegments)
    {
        // Skip segments outside the race.
        if (segment.end.x < 0.0f ||
            segment.start.x > FINISH_X)
            continue;

        float startX = std::clamp(
            segment.start.x, 0.0f, FINISH_X);

        float endX = std::clamp(
            segment.end.x, 0.0f, FINISH_X);

        // Interpolate heights if we clipped the segment.
        float dx = segment.end.x - segment.start.x;

        if (std::fabs(dx) < 0.001f)
            continue;

        float t1 = (startX - segment.start.x) / dx;
        float t2 = (endX - segment.start.x) / dx;

        float startY = segment.start.y +
            (segment.end.y - segment.start.y) * t1;

        float endY = segment.start.y +
            (segment.end.y - segment.start.y) * t2;

        SDL_RenderLine(
            renderer,
            ToMapX(startX), ToMapY(startY),
            ToMapX(endX), ToMapY(endY)
        );
    }

    // ========================================
    // FINISH LINE
    // ========================================

    SDL_SetRenderDrawColor(
        renderer, 180, 180, 180, 255);

    SDL_RenderLine(
        renderer,
        ToMapX(FINISH_X),
        mapY + 6.0f,
        ToMapX(FINISH_X),
        mapY + mapHeight - 6.0f
    );

    // ========================================
    // RACER MARKERS
    // ========================================

    auto DrawMarker = [&](float worldX, SDL_Color color,
        float markerSize)
        {
            SDL_SetRenderDrawColor(
                renderer,
                color.r, color.g, color.b, color.a
            );

            float x = ToMapX(worldX);
            float y = ToMapY(GetTrackY(worldX)) - 5.0f;

            SDL_FRect marker =
            {
                x - markerSize / 2.0f,
                y - markerSize / 2.0f,
                markerSize,
                markerSize
            };

            SDL_RenderFillRect(renderer, &marker);
        };

    // AI racers.
    const SDL_Color aiColors[] =
    {
        { 255, 180, 70, 255 },
        { 180, 120, 255, 255 },
        { 255, 100, 110, 255 }
    };

    for (size_t i = 0; i < aiRacers.size(); ++i)
    {
        b2Vec2 position = b2Body_GetPosition(
            aiRacers[i].bike.chassisBodyId);

        DrawMarker(
            position.x,
            aiColors[i % 3],
            6.0f
        );
    }

    // Player marker - slightly larger.
    b2Vec2 playerPosition = b2Body_GetPosition(
        playerRacer.bike.chassisBodyId);

    DrawMarker(
        playerPosition.x,
        { 55, 210, 255, 255 },
        9.0f
    );
}




// ---------------------------------------------------------
// RENDER
// ---------------------------------------------------------

void Render(
    SDL_Renderer* renderer,
    TTF_Font* font,
    TTF_Font* stuntFont,
    TTF_Font* countdownFont,
    SDL_Texture* bikeTexture,
    SDL_Texture* wheelTexture,
    SDL_Texture* secondRiderBikeTexture,
    SDL_Texture* secondRiderWheelTexture,
    b2BodyId chassisBodyId,
    b2BodyId rearWheelBodyId,
    b2BodyId frontWheelBodyId,
    const Racer& playerRacer,
    const std::vector<Racer>& aiRacers,
    const std::vector<RacerVisual>& aiVisuals,
    int selectedRider,
    float cameraX,
    Environment& environment,
    const SDL_FRect& groundRect,
    const SDL_FPoint& rearWheelScreen,
    const SDL_FPoint& frontWheelScreen,
    const std::vector<TerrainSegment>& terrainSegments,
    bool levelComplete,
    bool checkpointReached,
    float levelTime,
    float airTime,
    const char* stuntText,
    float stuntTextTimer,
    const char* landingText,
    float landingTextTimer,
    int comboCount,
    float comboTimer,
    int score,
    float bestTime,
    bool newBestTime,
    int resultsMenuSelection,
    float boostMeter,
    bool boostActive,
    int playerRacePosition,
    const ParticleSystem& particleSystem,
    float countdownTimer,
    bool raceStarted)

{

    // =====================================================
    // ENVIRONMENT
    // =====================================================

    environment.Render(
        renderer,
        cameraX
    );

    // =====================================================
    // BASE DIRT GROUND
    // =====================================================

    environment.SetTerrainDrawColor(
        renderer,
        83, 70, 76
    );

    SDL_FRect visualGround =
    {
        groundRect.x,
        groundRect.y,
        groundRect.w,
        SCREEN_HEIGHT - groundRect.y
    };

    SDL_RenderFillRect(
        renderer,
        &visualGround
    );

    // Light dirt near surface.
    SDL_FRect topDirt =
    {
        groundRect.x,
        groundRect.y,
        groundRect.w,
        25.0f
    };

    environment.SetTerrainDrawColor(
        renderer,
        112, 94, 91
    );

    SDL_RenderFillRect(
        renderer,
        &topDirt
    );


    // Medium dirt layer.
    SDL_FRect middleDirt =
    {
        groundRect.x,
        groundRect.y + 25.0f,
        groundRect.w,
        25.0f
    };

    environment.SetTerrainDrawColor(
        renderer,
        94, 77, 82
    );

    SDL_RenderFillRect(
        renderer,
        &middleDirt
    );


    // =====================================================
    // TERRAIN HILLS
    // =====================================================

    for (const TerrainSegment& terrainSegment : terrainSegments)
    {
        SDL_FPoint startScreen;
        SDL_FPoint endScreen;


        // Box2D world -> SDL screen.
        startScreen.x =
            CAMERA_TARGET_X +
            (terrainSegment.start.x - cameraX) *
            PIXELS_PER_METER;

        startScreen.y =
            SCREEN_CENTER_Y -
            terrainSegment.start.y *
            PIXELS_PER_METER;


        endScreen.x =
            CAMERA_TARGET_X +
            (terrainSegment.end.x - cameraX) *
            PIXELS_PER_METER;

        endScreen.y =
            SCREEN_CENTER_Y -
            terrainSegment.end.y *
            PIXELS_PER_METER;


        // -------------------------------------------------
        // FILL DIRT UNDER THE HILL
        // -------------------------------------------------

        environment.SetTerrainDrawColor(
            renderer,
            83, 70, 76
        );


        float differenceX =
            endScreen.x - startScreen.x;

        if (differenceX != 0.0f)
        {
            int startX =
                static_cast<int>(startScreen.x);

            int endX =
                static_cast<int>(endScreen.x);


            if (startX > endX)
            {
                int temporary = startX;
                startX = endX;
                endX = temporary;
            }


            for (int x = startX; x <= endX; ++x)
            {
                float percentage =
                    (static_cast<float>(x) - startScreen.x)
                    /
                    differenceX;

                float surfaceY =
                    startScreen.y +
                    (endScreen.y - startScreen.y) *
                    percentage;


                const float baseGroundY =
                    groundRect.y;


                // =====================================================
                // CONTINUOUS HILL DIRT
                // =====================================================

                // Deep earth.
                // This goes from the hill surface all the way down,
                // so the hill visually becomes part of the ground.
                environment.SetTerrainDrawColor(
                    renderer,
                    67, 58, 69
                );

                SDL_RenderLine(
                    renderer,
                    static_cast<float>(x),
                    surfaceY,
                    static_cast<float>(x),
                    groundRect.y
                );


                float middleBottom =
                    std::min(
                        surfaceY + 40.0f,
                        groundRect.y
                    );

                environment.SetTerrainDrawColor(
                    renderer,
                    92, 76, 83
                );

                SDL_RenderLine(
                    renderer,
                    static_cast<float>(x),
                    surfaceY,
                    static_cast<float>(x),
                    middleBottom
                );


                float topBottom =
                    std::min(
                        surfaceY + 15.0f,
                        groundRect.y
                    );

                environment.SetTerrainDrawColor(
                    renderer,
                    119, 98, 94
                );

                SDL_RenderLine(
                    renderer,
                    static_cast<float>(x),
                    surfaceY,
                    static_cast<float>(x),
                    topBottom
                );
                
            }
        }


        // -------------------------------------------------
        // GRASS SURFACE
        // -------------------------------------------------

        // Dark grass shadow.
        environment.SetTerrainDrawColor(
            renderer,
            66, 70, 58
        );

        SDL_RenderLine(
            renderer,
            startScreen.x,
            startScreen.y + 4.0f,
            endScreen.x,
            endScreen.y + 4.0f
        );


        // Bright grass surface.
        environment.SetTerrainDrawColor(
            renderer,
            139, 131, 92
        );

        for (int thickness = -1;
            thickness <= 1;
            ++thickness)
        {
            SDL_RenderLine(
                renderer,
                startScreen.x,
                startScreen.y + thickness,
                endScreen.x,
                endScreen.y + thickness
            );
        }
            // =====================================================
            // NATURAL GRASS TUFTS
            // =====================================================

        float terrainDX =
            endScreen.x - startScreen.x;

        float terrainDY =
            endScreen.y - startScreen.y;

        float terrainLength =
            std::sqrt(
                terrainDX * terrainDX +
                terrainDY * terrainDY
            );

        if (terrainLength > 0.0f)
        {
            // Direction pointing away from the terrain surface.
            float normalX =
                terrainDY / terrainLength;

            float normalY =
                -terrainDX / terrainLength;

            environment.SetTerrainDrawColor(
                renderer,
                109, 107, 80
            );

            // Add several small grass clumps.
            for (float t = 0.06f;
                t < 1.0f;
                t += 0.05f)
            {
                float grassX =
                    startScreen.x +
                    terrainDX * t;

                float grassY =
                    startScreen.y +
                    terrainDY * t;

                float grassHeight =
                    3.0f +
                    static_cast<int>(t * 1000.0f) % 5;

                // Middle blade.
                SDL_RenderLine(
                    renderer,
                    grassX,
                    grassY,
                    grassX + normalX * grassHeight,
                    grassY + normalY * grassHeight
                );

                // Left blade.
                SDL_RenderLine(
                    renderer,
                    grassX,
                    grassY,
                    grassX +
                    normalX * grassHeight -
                    2.0f,
                    grassY +
                    normalY * grassHeight
                );

                // Right blade.
                SDL_RenderLine(
                    renderer,
                    grassX,
                    grassY,
                    grassX +
                    normalX * grassHeight +
                    2.0f,
                    grassY +
                    normalY * grassHeight
                );
            }
        }
        
    }



    // =====================================================
    // FLAT GROUND GRASS
    // =====================================================

    environment.SetTerrainDrawColor(
        renderer,
        129, 123, 88
    );

    for (int thickness = 0;
        thickness < 5;
        ++thickness)
    {
        SDL_RenderLine(
            renderer,
            groundRect.x,
            groundRect.y + thickness,
            groundRect.x + groundRect.w,
            groundRect.y + thickness
        );
    }

        // =====================================================
        // FLAT GROUND GRASS TUFTS
        // =====================================================

    environment.SetTerrainDrawColor(
        renderer,
        109, 107, 80
    );

    int grassIndex = 0;

    for (float x = groundRect.x + 10.0f;
        x < groundRect.x + groundRect.w;
        x += 12.0f, ++grassIndex)
    {
        float grassY = groundRect.y;

        float grassHeight =
            3.0f +
            grassIndex % 5;

        SDL_RenderLine(
            renderer,
            x,
            grassY,
            x,
            grassY - grassHeight
        );

        SDL_RenderLine(
            renderer,
            x,
            grassY,
            x - 2.0f,
            grassY - grassHeight
        );

        SDL_RenderLine(
            renderer,
            x,
            grassY,
            x + 2.0f,
            grassY - grassHeight
        );
    }


    DrawGroundFog(
        renderer,
        cameraX,
        terrainSegments
    );

    DrawHauntedLanterns(
        renderer,
        cameraX,
        terrainSegments
    );




    DrawBrokenFences(
        renderer,
        cameraX,
        terrainSegments
    );

    DrawDeadTrees(
        renderer,
        cameraX,
        terrainSegments
    );

    DrawGraveyardProps(
        renderer,
        cameraX,
        terrainSegments
    );


        // =====================================================
        // FINISH LINE
        // =====================================================

    DrawFinishLine(
        renderer,
        cameraX
    );

    DrawCheckpoint(
        renderer,
        cameraX,
        checkpointReached
    );

        // =====================================================
        // PARTICLE EFFECTS
        // =====================================================

    particleSystem.Render(
        renderer,
        cameraX
    );


        
     // BIKE VISUALS
     // Chassis follows chassis physics.
     // Wheels follow wheel physics independently.
     // =====================================================

     // -----------------------------------------------------
     // CHASSIS PHYSICS POSITION + ROTATION
     // -----------------------------------------------------

    b2Vec2 chassisPosition =
        b2Body_GetPosition(chassisBodyId);

    float chassisAngleRadians =
        b2Rot_GetAngle(
            b2Body_GetRotation(chassisBodyId)
        );

    constexpr float RAD_TO_DEG =
        57.2957795f;


    // Convert chassis Box2D position -> SDL screen position.

    float chassisScreenX =
        CAMERA_TARGET_X +
        (chassisPosition.x - cameraX) *
        PIXELS_PER_METER;

    float chassisScreenY =
        SCREEN_CENTER_Y -
        chassisPosition.y *
        PIXELS_PER_METER;


    // =====================================================
    // RIDER + FRAME
    // =====================================================

    // Dimensions of rider_chassis.png.
    constexpr float ART_WIDTH = 1448.0f;
    constexpr float ART_HEIGHT = 1086.0f;


    // Axle positions inside rider_chassis.png.
    constexpr float REAR_AXLE_X = 249.0f;
    constexpr float REAR_AXLE_Y = 889.0f;

    constexpr float FRONT_AXLE_X = 1208.0f;
    constexpr float FRONT_AXLE_Y = 946.0f;


    // -----------------------------------------------------
    // AXLE LINE INSIDE THE ARTWORK
    // -----------------------------------------------------

    float artDX =
        FRONT_AXLE_X -
        REAR_AXLE_X;

    float artDY =
        FRONT_AXLE_Y -
        REAR_AXLE_Y;


    float artAxleDistance =
        std::sqrt(
            artDX * artDX +
            artDY * artDY
        );


    float artAxleAngleDegrees =
        std::atan2(
            artDY,
            artDX
        ) * RAD_TO_DEG;


    // -----------------------------------------------------
    // FIXED ART SCALE
    // -----------------------------------------------------

    // In Bike.cpp:
    //
    // rear chassis anchor  = -0.8
    // front chassis anchor = +0.8
    //
    // Distance between them = 1.6 Box2D meters.

    constexpr float PHYSICS_AXLE_DISTANCE =
        1.6f;


    // Convert the physical 1.6 meter wheelbase into pixels,
    // then determine how much the PNG should be scaled.

    float bikeScale =
        (PHYSICS_AXLE_DISTANCE *
            PIXELS_PER_METER)
        /
        artAxleDistance;


    // -----------------------------------------------------
    // FIND THE CHASSIS CENTER INSIDE THE PNG
    // -----------------------------------------------------

    // Midpoint between artwork axle holes.

    float artAxleMidX =
        (REAR_AXLE_X +
            FRONT_AXLE_X)
        / 2.0f;

    float artAxleMidY =
        (REAR_AXLE_Y +
            FRONT_AXLE_Y)
        / 2.0f;


    // Direction of axle line.

    float artDirectionX =
        artDX /
        artAxleDistance;

    float artDirectionY =
        artDY /
        artAxleDistance;


    // Perpendicular direction pointing upward
    // in PNG coordinates.
    //
    // PNG Y increases downward.

    float artUpX =
        artDirectionY;

    float artUpY =
        -artDirectionX;


    // In Bike.cpp the wheel-joint chassis anchors
    // are 0.65 meters below the chassis center.

    constexpr float CHASSIS_TO_AXLE_DISTANCE =
        0.65f;


    // Convert that physical distance back into
    // unscaled artwork pixels.

    float chassisOffsetInArt =
        (CHASSIS_TO_AXLE_DISTANCE *
            PIXELS_PER_METER)
        /
        bikeScale;


    // This is approximately where the Box2D chassis center
    // lives inside rider_chassis.png.

    float chassisArtX =
        artAxleMidX +
        artUpX *
        chassisOffsetInArt;

    float chassisArtY =
        artAxleMidY +
        artUpY *
        chassisOffsetInArt;


    // -----------------------------------------------------
    // RIDER / FRAME RECTANGLE
    // -----------------------------------------------------

    SDL_FRect bikeRect =
    {
        0.0f,
        0.0f,

        ART_WIDTH *
        bikeScale,

        ART_HEIGHT *
        bikeScale
    };


    // The rotation pivot is now the REAL chassis center,
    // not one of the wheels.

    SDL_FPoint bikePivot =
    {
        chassisArtX *
        bikeScale,

        chassisArtY *
        bikeScale
    };


    // Put that chassis point directly over the
    // Box2D chassis body's screen position.

    bikeRect.x =
        chassisScreenX -
        bikePivot.x;

    bikeRect.y =
        chassisScreenY -
        bikePivot.y;


    // -----------------------------------------------------
    // CHASSIS ROTATION
    // -----------------------------------------------------

    // Box2D positive rotation is counter-clockwise.
    //
    // SDL screen Y points downward, so visually we use
    // the negative Box2D angle.
    //
    // artAxleAngleDegrees removes the small built-in
    // axle angle from the PNG itself.

    float bikeVisualAngle =
        -chassisAngleRadians *
        RAD_TO_DEG
        -
        artAxleAngleDegrees;


    // ============================================
    // PLAYER RIDER - GHOST CHASSIS
    // ============================================


    if (selectedRider == 0)
    {
        // Ghost - original player artwork.
        SDL_RenderTextureRotated(
            renderer,
            bikeTexture,
            nullptr,
            &bikeRect,
            bikeVisualAngle,
            &bikePivot,
            SDL_FLIP_NONE
        );
    }

    else if (selectedRider == 1)
    {
        // Second playable rider.
        // Uses its own textures, not an AI racer.

        RacerVisual secondRiderVisual =
        {
            secondRiderBikeTexture,
            secondRiderWheelTexture,

            // PNG dimensions.
            1254.0f,
            1254.0f,

            // Rear axle location in the new PNG.
            68.0f,
            1128.0f,

            // Front axle location in the new PNG.
            1190.0f,
            1128.0f,

            1.0f,

            "Rider 02"
        };

        DrawBikeSprite(
            renderer,
            secondRiderVisual,
            rearWheelScreen,
            frontWheelScreen
        );
    }





    // =====================================================
    // WHEELS
    // =====================================================

    // Physical wheel diameter:
    //
    // radius 0.4 m
    // diameter 0.8 m.
    //
    // Extra scale compensates for transparent padding
    // around the wheel PNG.

    constexpr float WHEEL_VISUAL_SCALE =
        1.80f;

    float wheelSize =
        BIKE_WHEEL_RADIUS *
        2.0f *
        PIXELS_PER_METER *
        WHEEL_VISUAL_SCALE;


    // -----------------------------------------------------
    // REAR WHEEL
    // -----------------------------------------------------

    SDL_FRect rearWheelRect =
    {
        rearWheelScreen.x -
        wheelSize / 2.0f,

        rearWheelScreen.y -
        wheelSize / 2.0f,

        wheelSize,
        wheelSize
    };


    // -----------------------------------------------------
    // FRONT WHEEL
    // -----------------------------------------------------

    SDL_FRect frontWheelRect =
    {
        frontWheelScreen.x -
        wheelSize / 2.0f,

        frontWheelScreen.y -
        wheelSize / 2.0f,

        wheelSize,
        wheelSize
    };


    // Temporary visual alignment adjustment
    constexpr float FIRE_WHEEL_Y_OFFSET = 8.0f;

    rearWheelRect.y += FIRE_WHEEL_Y_OFFSET;
    frontWheelRect.y += FIRE_WHEEL_Y_OFFSET;

    // -----------------------------------------------------
    // WHEEL ROTATION
    // -----------------------------------------------------

    float rearWheelAngle =
        -b2Rot_GetAngle(
            b2Body_GetRotation(
                rearWheelBodyId
            )
        ) * RAD_TO_DEG;


    float frontWheelAngle =
        -b2Rot_GetAngle(
            b2Body_GetRotation(
                frontWheelBodyId
            )
        ) * RAD_TO_DEG;



    
        // ============================================
        // ANIMATED GHOST FIRE WHEELS
        // ============================================

        constexpr int FRAME_COUNT = 8;
        constexpr float FRAME_SIZE = 256.0f;

        // Remember animation progress between frames.
        static float flameFramePosition = 0.0f;
        static Uint64 lastFlameTick = SDL_GetTicksNS();

        Uint64 currentFlameTick = SDL_GetTicksNS();

        float animationDeltaTime =
            static_cast<float>(
                currentFlameTick - lastFlameTick
                ) / 1000000000.0f;

        lastFlameTick = currentFlameTick;

        // Avoid huge jumps after pauses.
        animationDeltaTime =
            std::min(animationDeltaTime, 0.05f);

        // Read actual Box2D horizontal speed.
        float speed = std::fabs(
            b2Body_GetLinearVelocity(
                chassisBodyId
            ).x
        );

        // Flames animate faster when riding faster.
        float framesPerSecond =
            5.0f + std::min(speed, 15.0f) * 0.8f;

        if (boostActive)
        {
            framesPerSecond = 24.0f;
        }

        flameFramePosition +=
            animationDeltaTime * framesPerSecond;

        int frameIndex =
            static_cast<int>(flameFramePosition)
            % FRAME_COUNT;

        // Select one frame from the spritesheet.
        SDL_FRect sourceRect =
        {
            frameIndex * FRAME_SIZE,
            0.0f,
            FRAME_SIZE,
            FRAME_SIZE
        };


        if (selectedRider == 0)
        {
            // Ghost - animated fire wheels.
            SDL_RenderTexture(
                renderer,
                wheelTexture,
                &sourceRect,
                &rearWheelRect
            );

            SDL_RenderTexture(
                renderer,
                wheelTexture,
                &sourceRect,
                &frontWheelRect
            );
        }

        else if (selectedRider == 1)
        {
            // Second rider's black-and-white wheels.

            DrawWheelSprite(
                renderer,
                secondRiderWheelTexture,
                rearWheelBodyId,
                rearWheelScreen,
                1.25f
            );

            DrawWheelSprite(
                renderer,
                secondRiderWheelTexture,
                frontWheelBodyId,
                frontWheelScreen,
                1.25f
            );
        }




    // =====================================================
    // AI RACERS
    // =====================================================

    for (size_t i = 0;
        i < aiRacers.size();
        ++i)
    {
        const Racer& aiRacer =
            aiRacers[i];

        const RacerVisual& visual =
            aiVisuals[i];


        // =================================================
        // AI WHEEL PHYSICS POSITIONS
        // =================================================

        b2Vec2 aiRearWheelPosition =
            b2Body_GetPosition(
                aiRacer.bike.rearWheelBodyId
            );


        b2Vec2 aiFrontWheelPosition =
            b2Body_GetPosition(
                aiRacer.bike.frontWheelBodyId
            );


        // =================================================
        // REAR WHEEL -> SCREEN
        // =================================================

        SDL_FPoint aiRearWheelScreen;

        aiRearWheelScreen.x =
            CAMERA_TARGET_X +
            (
                aiRearWheelPosition.x -
                cameraX
                ) *
            PIXELS_PER_METER;

        aiRearWheelScreen.y =
            SCREEN_CENTER_Y -
            aiRearWheelPosition.y *
            PIXELS_PER_METER;


        // =================================================
        // FRONT WHEEL -> SCREEN
        // =================================================

        SDL_FPoint aiFrontWheelScreen;

        aiFrontWheelScreen.x =
            CAMERA_TARGET_X +
            (
                aiFrontWheelPosition.x -
                cameraX
                ) *
            PIXELS_PER_METER;

        aiFrontWheelScreen.y =
            SCREEN_CENTER_Y -
            aiFrontWheelPosition.y *
            PIXELS_PER_METER;


        // =================================================
        // BIKE / RIDER BODY
        // =================================================

        DrawBikeSprite(
            renderer,
            visual,
            aiRearWheelScreen,
            aiFrontWheelScreen
        );



        float aiWheelScale = 1.55f;

        // Rear wheel
        DrawWheelSprite(
            renderer,
            visual.wheelTexture,
            aiRacer.bike.rearWheelBodyId,
            aiRearWheelScreen,
            aiWheelScale
        );

        // Front wheel
        DrawWheelSprite(
            renderer,
            visual.wheelTexture,
            aiRacer.bike.frontWheelBodyId,
            aiFrontWheelScreen,
            aiWheelScale
        );

    }


    // =====================================================
    // PLAYER POSITION BADGE
    // =====================================================

    DrawRacePositionBadge(
        renderer,
        stuntFont,

        chassisScreenX,
        chassisScreenY - 120.0f,

        playerRacePosition
    );

    // =====================================================
    // UI
    // =====================================================



    if (levelComplete)
    {
        DrawRaceResults(
            renderer,
            font,
            stuntFont,
            playerRacer,
            aiRacers,
            bestTime,
            newBestTime,
            resultsMenuSelection
        );
    }
    else
    {
        DrawUI(
            renderer,
            font,
            stuntFont,
            levelComplete,
            levelTime,
            stuntText,
            stuntTextTimer,
            landingText,
            landingTextTimer,
            comboCount,
            comboTimer,
            score,
            bestTime,
            newBestTime,
            boostMeter,
            boostActive,
            playerRacePosition
        );
    }

    // ============================================
    // RACE MINIMAP
    // ============================================

    if (!levelComplete)
    {
        DrawRaceMinimap(
            renderer,
            terrainSegments,
            playerRacer,
            aiRacers
        );
    }

    // ============================================
    // RACE START COUNTDOWN
    // ============================================

    const char* countdownText = nullptr;

    if (!raceStarted)
    {
        if (countdownTimer > 2.0f)
            countdownText = "3";
        else if (countdownTimer > 1.0f)
            countdownText = "2";
        else
            countdownText = "1";
    }
    else if (!levelComplete && levelTime < 0.7f)
    {
        countdownText = "GO!";
    }

    if (countdownText)
    {
        int textWidth = 0;
        int textHeight = 0;

        if (TTF_GetStringSize(
            stuntFont,
            countdownText,
            0,
            &textWidth,
            &textHeight))
        {
            SDL_Color countdownColor =
                raceStarted
                ? SDL_Color{ 80, 255, 130, 255 }
            : SDL_Color{ 255, 220, 70, 255 };

            DrawText(
                renderer,
                countdownFont,
                countdownText,
                (SCREEN_WIDTH - textWidth) / 2.0f,
                (SCREEN_HEIGHT - textHeight) / 2.0f,
                countdownColor
            );
        }
    }


    // =====================================================
    // PRESENT
    // =====================================================

    SDL_RenderPresent(renderer);
}

void ApplyBunnyHop(
    const Bike& bike,
    float jumpVelocityChange)
{
    const float chassisMass =
        b2Body_GetMass(
            bike.chassisBodyId
        );

    const float rearWheelMass =
        b2Body_GetMass(
            bike.rearWheelBodyId
        );

    const float frontWheelMass =
        b2Body_GetMass(
            bike.frontWheelBodyId
        );


    b2Body_ApplyLinearImpulseToCenter(
        bike.chassisBodyId,
        {
            0.0f,
            chassisMass *
            jumpVelocityChange
        },
        true
    );

    b2Body_ApplyLinearImpulseToCenter(
        bike.rearWheelBodyId,
        {
            0.0f,
            rearWheelMass *
            jumpVelocityChange
        },
        true
    );

    b2Body_ApplyLinearImpulseToCenter(
        bike.frontWheelBodyId,
        {
            0.0f,
            frontWheelMass *
            jumpVelocityChange
        },
        true
    );
}




// ---------------------------------------------------------
// MAIN
// ---------------------------------------------------------

int main(int argc, char* argv[])
{
    // -----------------------------------------------------
    // INITIALIZE SDL
    // -----------------------------------------------------

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log(
            "SDL initialization failed: %s",
            SDL_GetError()
        );

        return 1;
    }

    if (!TTF_Init())
    {
        SDL_Log(
            "SDL_ttf initialization failed: %s",
            SDL_GetError()
        );

        SDL_Quit();
        return 1;
    }


    // -----------------------------------------------------
    // CREATE WINDOW
    // -----------------------------------------------------

    SDL_Window* window = SDL_CreateWindow(
        "TrailTorque",
        static_cast<int>(SCREEN_WIDTH),
        static_cast<int>(SCREEN_HEIGHT),
        0
    );

    if (!window)
    {
        SDL_Log(
            "Window creation failed: %s",
            SDL_GetError()
        );

        SDL_Quit();
        return 1;
    }


    // -----------------------------------------------------
    // CREATE RENDERER
    // -----------------------------------------------------

    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, nullptr);

    SDL_Texture* bikeTexture =
        IMG_LoadTexture(
            renderer,
            "assets/Bike/rider_chassis__.png"
        );


    SDL_Texture* wheelTexture =
        IMG_LoadTexture(
            renderer,
            "assets/Bike/fire_wheel_strip.png"
        );

    // ============================================
// SECOND PLAYABLE RIDER
// ============================================

    SDL_Texture* secondRiderBikeTexture =
        IMG_LoadTexture(
            renderer,
            "assets/Bike/rider_chassis_.png"
        );

    SDL_Texture* secondRiderWheelTexture =
        IMG_LoadTexture(
            renderer,
            "assets/Bike/bike_wheel_.png"
        );

    if (!secondRiderBikeTexture ||
        !secondRiderWheelTexture)
    {
        SDL_Log(
            "Second rider textures failed: %s",
            SDL_GetError()
        );

        return 1;
    }

    SDL_SetTextureScaleMode(
        secondRiderBikeTexture,
        SDL_SCALEMODE_LINEAR
    );

    SDL_SetTextureScaleMode(
        secondRiderWheelTexture,
        SDL_SCALEMODE_LINEAR
    );

    SDL_SetTextureScaleMode(
        bikeTexture,
        SDL_SCALEMODE_LINEAR
    );

    SDL_SetTextureScaleMode(
        wheelTexture,
        SDL_SCALEMODE_LINEAR
    );

    // =====================================================
 // GHOST TEXTURES
 // =====================================================

    const char* basePath =
        SDL_GetBasePath();

    SDL_Log(
        "GAME BASE PATH: %s",
        basePath ? basePath : "NULL"
    );


    // =====================================================
    // GHOST BIKE
    // =====================================================

    std::string ghostBikePath =
        std::string(
            basePath ? basePath : ""
        ) +
        "assets/Bike/ai3_ghost_bike.png";


    SDL_Log(
        "Trying Ghost bike: %s",
        ghostBikePath.c_str()
    );


    SDL_Texture* ghostBikeTexture =
        IMG_LoadTexture(
            renderer,
            ghostBikePath.c_str()
        );


    if (!ghostBikeTexture)
    {
        SDL_Log(
            "GHOST BIKE FAILED: %s",
            SDL_GetError()
        );
    }
    else
    {
        SDL_Log(
            "GHOST BIKE LOADED"
        );

        SDL_SetTextureScaleMode(
            ghostBikeTexture,
            SDL_SCALEMODE_LINEAR
        );
    }


    // =====================================================
    // GHOST WHEEL
    // =====================================================

    std::string ghostWheelPath =
        std::string(
            basePath ? basePath : ""
        ) +
        "assets/Bike/ai3_ghost_wheel.png";


    SDL_Log(
        "Trying Ghost wheel: %s",
        ghostWheelPath.c_str()
    );


    SDL_Texture* ghostWheelTexture =
        IMG_LoadTexture(
            renderer,
            ghostWheelPath.c_str()
        );


    if (!ghostWheelTexture)
    {
        SDL_Log(
            "GHOST WHEEL FAILED: %s",
            SDL_GetError()
        );
    }
    else
    {
        SDL_Log(
            "GHOST WHEEL LOADED"
        );

        SDL_SetTextureScaleMode(
            ghostWheelTexture,
            SDL_SCALEMODE_LINEAR
        );
    }

    // ============================================
// AI 2 - GRAVEL
// ============================================

    SDL_Texture* gravelBikeTexture =
        IMG_LoadTexture(
            renderer,
            "assets/Bike/ai1_ghost_bike.png"
        );

    SDL_Texture* gravelWheelTexture =
        IMG_LoadTexture(
            renderer,
            "assets/Bike/ai1_ghost_wheel.png"
        );

    // ============================================
    // AI 3 - RIOT
    // ============================================

    SDL_Texture* riotBikeTexture =
        IMG_LoadTexture(
            renderer,
            "assets/Bike/ai2_ghost_bike.png"
        );

    SDL_Texture* riotWheelTexture =
        IMG_LoadTexture(
            renderer,
            "assets/Bike/ai2_ghost_wheel.png"
        );

    if (!gravelBikeTexture ||
        !gravelWheelTexture ||
        !riotBikeTexture ||
        !riotWheelTexture)
    {
        SDL_Log(
            "AI asset loading failed: %s",
            SDL_GetError()
        );
        return 1;
    }

    SDL_SetTextureScaleMode(
        gravelBikeTexture, SDL_SCALEMODE_LINEAR
    );

    SDL_SetTextureScaleMode(
        gravelWheelTexture, SDL_SCALEMODE_LINEAR
    );

    SDL_SetTextureScaleMode(
        riotBikeTexture, SDL_SCALEMODE_LINEAR
    );

    SDL_SetTextureScaleMode(
        riotWheelTexture, SDL_SCALEMODE_LINEAR
    );


    // =====================================================
    // AI VISUALS
    // =====================================================

    std::vector<RacerVisual> aiVisuals;

    // ============================================
    // AI 1 - EXISTING RACER
    // ============================================

    aiVisuals.push_back(
        {
            ghostBikeTexture,
            ghostWheelTexture,

            1448.0f,
            1086.0f,

            290.0f,
            825.0f,

            1205.0f,
            830.0f,

            1.0f,
            "Ghost"
        }
    );

    // ============================================
    // AI 2 - GRAVEL
    // ============================================

    aiVisuals.push_back(
        {
            gravelBikeTexture,
            gravelWheelTexture,

            1448.0f,
            1086.0f,

            // Rear axle
            249.0f,
            869.0f,

            // Front axle
            1176.0f,
            964.0f,

            1.0f,
            "Gravel"
        }
    );

    // ============================================
    // AI 3 - RIOT
    // ============================================

    aiVisuals.push_back(
        {
            riotBikeTexture,
            riotWheelTexture,

            1448.0f,
            1086.0f,

            // Rear axle
            260.0f,
            895.0f,

            // Front axle
            1177.0f,
            965.0f,

            1.0f,
            "Riot"
        }
    );


   



    if (!bikeTexture)
    {
        SDL_Log(
            "Failed to load bike texture: %s",
            SDL_GetError()
        );
    }

    if (!wheelTexture)
    {
        SDL_Log(
            "Failed to load wheel texture: %s",
            SDL_GetError()
        );
    }

    if (!renderer)
    {
        SDL_Log(
            "Renderer creation failed: %s",
            SDL_GetError()
        );

        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }



    TTF_Font* font =
        TTF_OpenFont(
            "assets/Fonts/Bangers-Regular.ttf",
            28.0f
        );

    TTF_Font* stuntFont =
        TTF_OpenFont(
            "assets/Fonts/Bangers-Regular.ttf",
            48.0f
        );

    TTF_Font* countdownFont = TTF_OpenFont(
        "assets/Fonts/Bangers-Regular.ttf",
        110.0f
    );

    if (!countdownFont)
    {
        SDL_Log("Countdown font failed: %s", SDL_GetError());
    }
        

    if (!font)
    {
        SDL_Log(
            "Font loading failed: %s",
            SDL_GetError()
        );

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        TTF_Quit();
        SDL_Quit();

        return 1;
    }
    // =====================================================
    // BOX2D WORLD
    // =====================================================

    b2WorldDef worldDef =
        b2DefaultWorldDef();

    // Keep physics bodies awake while testing the bike controls.
    worldDef.enableSleep = false;

    // Box2D uses +Y upward.
    worldDef.gravity =
        b2Vec2{ 0.0f, -10.0f };

    b2WorldId worldId =
        b2CreateWorld(&worldDef);


    Terrain terrain =
        CreateTerrain(worldId);

    const b2Vec2 playerSpawnPosition =
    {
        0.0f,
        -7.0f
    };

    const b2Vec2 aiSpawnPosition =
    {
        -3.0f,
        -7.0f
    };


    Racer playerRacer;

    playerRacer.isPlayer = true;

    playerRacer.spawnPosition =
    {
        0.0f,
        -7.0f
    };

    playerRacer.bike =
        CreateBike(
            worldId,
            playerRacer.spawnPosition
        );


    std::vector<Racer> aiRacers;

    aiRacers.reserve(3);

    const b2Vec2 aiSpawnPositions[3] =
    {
        { -1.8f, -7.0f },
        { -3.6f, -7.0f },
        { -5.4f, -7.0f }
    };


    for (const b2Vec2& spawnPosition : aiSpawnPositions)
    {
        Racer aiRacer;

        aiRacer.isPlayer = false;

        aiRacer.spawnPosition =
            spawnPosition;

        aiRacer.bike =
            CreateBike(
                worldId,
                aiRacer.spawnPosition
            );

        aiRacers.push_back(
            aiRacer
        );
    }


    // =====================================================
    // AI STATES
    // =====================================================

    std::vector<AIState> aiStates(
        aiRacers.size()
    );


    // =====================================================
    // PHYSICS SETTINGS
    // =====================================================

    const float physicsTimeStep =
        1.0f / 60.0f;

    const int subStepCount =
        4;

    float physicsAccumulator =
        0.0f;


    // =====================================================
    // GAME VARIABLES
    // =====================================================

    bool running = true;

    SaveData saveData = LoadGame();

    GameScreen currentScreen = GameScreen::MainMenu;

    int mainMenuSelection = 0; // 0 = Play, 1 = Quit
    int pauseMenuSelection = 0;

    bool restartRaceRequested = false;
    int resultsMenuSelection = 0; // 0 = Race Again, 1 = Main Menu

    int riderMenuSelection = 0;
    int selectedRider = saveData.selectedRider;

    bool menuUpHeld = false;
    bool menuDownHeld = false;
    bool enterHeld = false;
    bool escapeHeld = false;

    int selectedLevel = 1;

    constexpr int MAX_LEVELS = 10;


    float cameraX = 0.0f;

    Uint64 previousTime =
        SDL_GetTicksNS();

    InputState input;
    
    bool levelComplete = false; 

    bool checkpointReached = false;

    float levelTime = 0.0f;

    float raceElapsedTime = 0.0f;

    // Race start countdown
    float countdownTimer = 3.0f;
    bool raceStarted = false;

    float airTime = 0.0f;
    bool wasBikeGrounded = true;

    float wheelieTime = 0.0f;
    bool wheelieAwarded = false;

    float previousBikeAngle = 0.0f;
    float accumulatedRotation = 0.0f;

    bool positiveFlipCompleted = false;
    bool negativeFlipCompleted = false;
    int flipCount = 0;

    int nextFinishPlace = 1;

    ParticleSystem particleSystem;

    Environment environment;

    // Load the previous best time when the game starts.
    float bestTime = saveData.bestTime;
    bool newBestTime = false;

    bool hasWonHauntedRun = saveData.hauntedRunWon;

    char stuntText[64] = "";
    float stuntTextTimer = 0.0f;

    char landingText[64] = "";
    float landingTextTimer = 0.0f;

    int score = 0;

    int comboCount = 0;

    float comboTimer = 0.0f;


    constexpr float PERFECT_LANDING_MIN_AIRTIME =
        0.50f;

    constexpr float PERFECT_LANDING_MAX_ANGLE =
        0.40f;

    constexpr float PERFECT_LANDING_MAX_ANGULAR_SPEED =
        2.0f;

    constexpr int PERFECT_LANDING_BASE_SCORE =
        250;

    int checkpointScore = 0;

    b2Vec2 respawnPosition =
        b2Vec2{ 0.0f, -7.0f };

    // =====================================================
    // BOOST SYSTEM
    // =====================================================

    float boostMeter = 0.0f;
    bool boostActive = false;

    constexpr float MAX_BOOST = 100.0f;
    constexpr float BOOST_DRAIN_RATE = 20.0f;

    // =====================================================
    // MAIN GAME LOOP
    // =====================================================

    while (running)
    {
        // -------------------------------------------------
        // DELTA TIME
        // -------------------------------------------------

        const Uint64 currentTime =
            SDL_GetTicksNS();

        const float deltaTime =
            static_cast<float>(
                currentTime - previousTime
                )
            / 1000000000.0f;

        previousTime = currentTime;


        // ============================================
        // MAIN GARAGE MENU
        // ============================================

        if (currentScreen == GameScreen::MainMenu)
        {
            ProcessInput(running, input);

            if (!running)
                break;

            const bool* keys =
                SDL_GetKeyboardState(nullptr);




            if (keys[SDL_SCANCODE_UP] && !menuUpHeld)
            {
                mainMenuSelection =
                    (mainMenuSelection + 5) % 6;
            }

            if (keys[SDL_SCANCODE_DOWN] && !menuDownHeld)
            {
                mainMenuSelection =
                    (mainMenuSelection + 1) % 6;
            }

            menuUpHeld = keys[SDL_SCANCODE_UP];
            menuDownHeld = keys[SDL_SCANCODE_DOWN];



            bool enterDown =
                keys[SDL_SCANCODE_RETURN] ||
                keys[SDL_SCANCODE_KP_ENTER];

            bool enterPressed = enterDown && !enterHeld;

            enterHeld = enterDown;



            if (enterPressed)
            {
                switch (mainMenuSelection)
                {
                case 0:
                    currentScreen = GameScreen::LevelSelect;
                    break;

                case 1:
                    currentScreen = GameScreen::RiderSelect;
                    break;

                case 2:
                    currentScreen = GameScreen::Garage;
                    break;

                case 3:
                    currentScreen = GameScreen::Achievements;
                    break;

                case 4:
                    currentScreen = GameScreen::Controls;
                    break;

                case 5:
                    running = false;
                    break;
                }

                continue;
            }





            bool escapeDown =
                keys[SDL_SCANCODE_ESCAPE];

            bool escapePressed =
                escapeDown && !escapeHeld;

            escapeHeld = escapeDown;

            if (escapePressed)
            {
                running = false;
                break;
            }




            DrawGarageMenu(
                renderer,
                font,
                stuntFont,
                bikeTexture,
                wheelTexture,
                mainMenuSelection
            );


            SDL_RenderPresent(renderer);

            SDL_Delay(16);

            // Never run race physics on menu screens.
            continue;
        }


        // ============================================
        // GARAGE SCREEN
        // ============================================

        if (currentScreen == GameScreen::Garage)
        {
            ProcessInput(running, input);

            if (!running)
                break;

            const bool* keys =
                SDL_GetKeyboardState(nullptr);

            bool enterDown =
                keys[SDL_SCANCODE_RETURN] ||
                keys[SDL_SCANCODE_KP_ENTER];

            bool enterPressed =
                enterDown && !enterHeld;

            enterHeld = enterDown;

            bool escapeDown =
                keys[SDL_SCANCODE_ESCAPE];

            bool escapePressed =
                escapeDown && !escapeHeld;

            escapeHeld = escapeDown;

            if (enterPressed)
            {
                riderMenuSelection = selectedRider;
                currentScreen = GameScreen::RiderSelect;
                continue;
            }

            if (escapePressed)
            {
                currentScreen = GameScreen::MainMenu;
                continue;
            }

            SDL_SetRenderDrawColor(
                renderer, 0, 0, 0, 255
            );

            SDL_RenderClear(renderer);

            const SDL_Color white =
            { 255, 255, 255, 255 };

            // Draw text centered horizontally.
            auto DrawCentered =
                [&](TTF_Font* textFont,
                    const char* text,
                    float y)
                {
                    int w = 0;
                    int h = 0;

                    if (TTF_GetStringSize(
                        textFont, text, 0, &w, &h))
                    {
                        DrawText(
                            renderer,
                            textFont,
                            text,
                            (SCREEN_WIDTH - w) / 2.0f,
                            y,
                            white
                        );
                    }
                };

            DrawCentered(stuntFont, "GARAGE", 150.0f);

            DrawCentered(
                font,
                "CURRENT RIDER",
                290.0f
            );

            DrawCentered(
                stuntFont,
                selectedRider == 0 ? "GHOST" : "RIDER 02",
                350.0f
            );

            DrawCentered(
                font,
                "ENTER - CHANGE RIDER",
                480.0f
            );

            DrawCentered(
                font,
                "ESC - BACK",
                620.0f
            );

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }


        // ============================================
        // CONTROLS SCREEN
        // ============================================

        if (currentScreen == GameScreen::Controls)
        {
            ProcessInput(running, input);

            if (!running)
                break;

            const bool* keys =
                SDL_GetKeyboardState(nullptr);

            // Keep Enter state updated.
            enterHeld =
                keys[SDL_SCANCODE_RETURN] ||
                keys[SDL_SCANCODE_KP_ENTER];

            // Escape returns to Main Menu.
            bool escapeDown =
                keys[SDL_SCANCODE_ESCAPE];

            bool escapePressed =
                escapeDown && !escapeHeld;

            escapeHeld = escapeDown;

            if (escapePressed)
            {
                currentScreen = GameScreen::MainMenu;
                continue;
            }

            SDL_SetRenderDrawColor(
                renderer, 0, 0, 0, 255
            );
            SDL_RenderClear(renderer);

            const SDL_Color white =
            { 255, 255, 255, 255 };

            // Center the title.
            int titleWidth = 0;
            int titleHeight = 0;

            TTF_GetStringSize(
                stuntFont, "CONTROLS", 0,
                &titleWidth, &titleHeight
            );

            DrawText(
                renderer, stuntFont,
                "CONTROLS",
                (SCREEN_WIDTH - titleWidth) / 2.0f,
                100.0f,
                white
            );

            // Keyboard controls.
            const char* controls[] =
            {
                "W  -  ACCELERATE",
                "S  -  REVERSE",
                "A  -  LEAN BACKWARD",
                "D  -  LEAN FORWARD",
                "SPACE  -  BUNNY HOP",
                "R  -  RESTART / RESPAWN"
            };

            for (int i = 0; i < 6; ++i)
            {
                DrawText(
                    renderer,
                    font,
                    controls[i],
                    430.0f,
                    225.0f + i * 48.0f,
                    white
                );
            }

            DrawText(
                renderer, font,
                "BOOST ACTIVATES AUTOMATICALLY",
                430.0f, 530.0f, white
            );

            DrawText(
                renderer, font,
                "ESC - BACK",
                535.0f, 630.0f, white
            );

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }


        // ============================================
        // OTHER MENU SCREENS
        // ============================================


        if (currentScreen == GameScreen::Achievements)
        {
            ProcessInput(running, input);

            if (!running)
                break;

            const bool* keys =
                SDL_GetKeyboardState(nullptr);

            enterHeld =
                keys[SDL_SCANCODE_RETURN] ||
                keys[SDL_SCANCODE_KP_ENTER];

            bool escapeDown =
                keys[SDL_SCANCODE_ESCAPE];

            bool escapePressed =
                escapeDown && !escapeHeld;

            escapeHeld = escapeDown;

            if (escapePressed)
            {
                currentScreen = GameScreen::MainMenu;
                continue;
            }

            SDL_SetRenderDrawColor(
                renderer, 0, 0, 0, 255
            );
            SDL_RenderClear(renderer);

            const SDL_Color white =
            { 255, 255, 255, 255 };

            const SDL_Color gray =
            { 150, 150, 150, 255 };

            DrawText(
                renderer, stuntFont,
                "AWARDS",
                560.0f, 125.0f, white
            );

            DrawText(
                renderer, font,
                "COMPLETE HAUNTED RUN",
                385.0f, 285.0f, white
            );

            DrawText(
                renderer, font,
                bestTime >= 0.0f
                ? "COMPLETED"
                : "NOT COMPLETED",
                720.0f, 285.0f,
                bestTime >= 0.0f ? white : gray
            );

            DrawText(
                renderer, font,
                "WIN HAUNTED RUN",
                385.0f, 355.0f, white
            );

            DrawText(
                renderer, font,
                hasWonHauntedRun
                ? "COMPLETED"
                : "NOT COMPLETED",
                720.0f, 355.0f,
                hasWonHauntedRun ? white : gray
            );

            DrawText(
                renderer, font,
                "ESC - BACK",
                535.0f, 620.0f, white
            );

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }



        // ============================================
        // RIDER SELECT
        // ============================================

        if (currentScreen == GameScreen::RiderSelect)
        {
            ProcessInput(running, input);

            if (!running)
                break;

            const bool* keys =
                SDL_GetKeyboardState(nullptr);

            if (keys[SDL_SCANCODE_UP] && !menuUpHeld)
            {
                riderMenuSelection =
                    (riderMenuSelection + RIDER_COUNT - 1)
                    % RIDER_COUNT;
            }

            if (keys[SDL_SCANCODE_DOWN] && !menuDownHeld)
            {
                riderMenuSelection =
                    (riderMenuSelection + 1) % 2;
            }

            menuUpHeld = keys[SDL_SCANCODE_UP];
            menuDownHeld = keys[SDL_SCANCODE_DOWN];

            bool enterDown =
                keys[SDL_SCANCODE_RETURN] ||
                keys[SDL_SCANCODE_KP_ENTER];

            bool enterPressed =
                enterDown && !enterHeld;

            enterHeld = enterDown;

            

                if (enterPressed)
                {
                    selectedRider = riderMenuSelection;

                    // Update saved rider selection.
                    saveData.selectedRider = selectedRider;

                    // Write progress to disk.
                    SaveGame(saveData);
                }


            // Return to main menu.
            // Use the escapeHeld variable from our
            // previous menu-navigation fix.
            bool escapeDown =
                keys[SDL_SCANCODE_ESCAPE];

            bool escapePressed =
                escapeDown && !escapeHeld;

            escapeHeld = escapeDown;

            if (escapePressed)
            {
                currentScreen = GameScreen::MainMenu;
                continue;
            }

            DrawRiderSelect(
                renderer,
                font,
                stuntFont,
                riderMenuSelection,
                selectedRider
            );

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }


        // ============================================
        // LEVEL SELECT SCREEN
        // ============================================

        if (currentScreen == GameScreen::LevelSelect)
        {
            // Handle keyboard and window events.
            ProcessInput(running, input);

            if (!running)
                break;

            const bool* keys =
                SDL_GetKeyboardState(nullptr);


            bool enterDown =
                keys[SDL_SCANCODE_RETURN] ||
                keys[SDL_SCANCODE_KP_ENTER];

            bool enterPressed =
                enterDown && !enterHeld;

            enterHeld = enterDown;

            if (enterPressed)
            {
                selectedLevel = 1;

                currentScreen = GameScreen::Race;

                SDL_Log(
                    "Starting Level %d",
                    selectedLevel
                );

                continue;
            }

            // Escape returns to Main Menu.
            if (keys[SDL_SCANCODE_ESCAPE])
            {
                currentScreen = GameScreen::MainMenu;
                continue;
            }


            // Draw our Level Select menu.
            DrawLevelSelect(
                renderer,
                font,
                stuntFont
            );

            SDL_RenderPresent(renderer);

            // Prevent unnecessary CPU usage.
            SDL_Delay(16);

            // Do not run race physics in the menu.
            continue;
        }



        // ============================================
        // PAUSE SCREEN
        // ============================================


        if (currentScreen == GameScreen::Pause)
        {
            ProcessInput(running, input);

            if (!running)
                break;

            const bool* keys =
                SDL_GetKeyboardState(nullptr);

            // --------------------------------
            // UP / DOWN SELECTION
            // --------------------------------

            if (keys[SDL_SCANCODE_UP] && !menuUpHeld)
            {
                pauseMenuSelection =
                    (pauseMenuSelection + 2) % 3;
            }

            if (keys[SDL_SCANCODE_DOWN] && !menuDownHeld)
            {
                pauseMenuSelection =
                    (pauseMenuSelection + 1) % 3;
            }

            menuUpHeld = keys[SDL_SCANCODE_UP];
            menuDownHeld = keys[SDL_SCANCODE_DOWN];

            // --------------------------------
            // ENTER
            // --------------------------------

            bool enterDown =
                keys[SDL_SCANCODE_RETURN] ||
                keys[SDL_SCANCODE_KP_ENTER];

            bool enterPressed =
                enterDown && !enterHeld;

            enterHeld = enterDown;

            // --------------------------------
            // ESCAPE
            // --------------------------------

            bool escapeDown =
                keys[SDL_SCANCODE_ESCAPE];

            bool escapePressed =
                escapeDown && !escapeHeld;

            escapeHeld = escapeDown;


            if (enterPressed)
            {
                if (pauseMenuSelection == 0)
                {
                    // Resume.
                    currentScreen = GameScreen::Race;
                }
                else if (pauseMenuSelection == 1)
                {
                    // Request a fresh race.
                    restartRaceRequested = true;
                    currentScreen = GameScreen::Race;
                }
                else
                {
                    // Start fresh next time Play is chosen.
                    restartRaceRequested = true;
                    currentScreen = GameScreen::MainMenu;
                    mainMenuSelection = 0;
                }

                physicsAccumulator = 0.0f;
                continue;
            }

            // --------------------------------
            // DRAW PAUSE MENU
            // --------------------------------

            SDL_SetRenderDrawColor(
                renderer, 0, 0, 0, 255
            );
            SDL_RenderClear(renderer);

            const SDL_Color white =
            { 255, 255, 255, 255 };

            const char* options[] =
            {
                "RESUME",
                "RESTART",
                "MAIN MENU"
            };

            auto DrawCentered =
                [&](const char* text, float y)
                {
                    int width = 0;
                    int height = 0;

                    if (TTF_GetStringSize(
                        stuntFont, text, 0,
                        &width, &height))
                    {
                        DrawText(
                            renderer,
                            stuntFont,
                            text,
                            (SCREEN_WIDTH - width) / 2.0f,
                            y,
                            white
                        );
                    }
                };

            DrawCentered("PAUSED", 150.0f);

            for (int i = 0; i < 3; ++i)
            {
                int width = 0;
                int height = 0;

                TTF_GetStringSize(
                    stuntFont,
                    options[i],
                    0,
                    &width,
                    &height
                );

                float x =
                    (SCREEN_WIDTH - width) / 2.0f;

                float y = 300.0f + i * 70.0f;

                DrawText(
                    renderer,
                    stuntFont,
                    options[i],
                    x, y, white
                );

                if (pauseMenuSelection == i)
                {
                    DrawText(
                        renderer,
                        stuntFont,
                        ">",
                        x - 35.0f,
                        y,
                        white
                    );
                }
            }

            DrawText(
                renderer,
                font,
                "UP / DOWN - SELECT    ENTER - CONFIRM",
                390.0f,
                630.0f,
                white
            );

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }



        // ============================================
        // RACE INPUT / PAUSE DETECTION
        // ============================================


        if (currentScreen == GameScreen::Race)
        {
            ProcessInput(running, input);

            if (!running)
                break;

            const bool* keys =
                SDL_GetKeyboardState(nullptr);

            bool escapeDown =
                keys[SDL_SCANCODE_ESCAPE];

            bool escapePressed =
                escapeDown && !escapeHeld;

            escapeHeld = escapeDown;

            bool enterDown =
                keys[SDL_SCANCODE_RETURN] ||
                keys[SDL_SCANCODE_KP_ENTER];

            bool enterPressed =
                enterDown && !enterHeld;

            enterHeld = enterDown;

            if (levelComplete)
            {
                if (keys[SDL_SCANCODE_UP] && !menuUpHeld)
                {
                    resultsMenuSelection =
                        (resultsMenuSelection + 1) % 2;
                }

                if (keys[SDL_SCANCODE_DOWN] && !menuDownHeld)
                {
                    resultsMenuSelection =
                        (resultsMenuSelection + 1) % 2;
                }

                menuUpHeld = keys[SDL_SCANCODE_UP];
                menuDownHeld = keys[SDL_SCANCODE_DOWN];

                if (enterPressed)
                {
                    restartRaceRequested = true;
                    physicsAccumulator = 0.0f;

                    if (resultsMenuSelection == 1)
                    {
                        currentScreen = GameScreen::MainMenu;
                        mainMenuSelection = 0;
                    }

                    continue;
                }

                if (escapePressed)
                {
                    restartRaceRequested = true;
                    currentScreen = GameScreen::MainMenu;
                    mainMenuSelection = 0;
                    physicsAccumulator = 0.0f;
                    continue;
                }
            }
            else
            {
                menuUpHeld = keys[SDL_SCANCODE_UP];
                menuDownHeld = keys[SDL_SCANCODE_DOWN];

                if (escapePressed)
                {
                    currentScreen = GameScreen::Pause;
                    pauseMenuSelection = 0;
                    physicsAccumulator = 0.0f;
                    continue;
                }
            }
        }


        // ---------------------------------------------
        // STUNT POPUP TIMER
        // ---------------------------------------------

        if (stuntTextTimer > 0.0f)
        {
            stuntTextTimer -= deltaTime;

            if (stuntTextTimer < 0.0f)
            {
                stuntTextTimer = 0.0f;
            }
        }

        // ---------------------------------------------
        // LANDING POPUP TIMER
        // ---------------------------------------------

        if (landingTextTimer > 0.0f)
        {
            landingTextTimer -=
                deltaTime;

            if (landingTextTimer < 0.0f)
            {
                landingTextTimer = 0.0f;
            }
        }


        // ---------------------------------------------
        // COMBO TIMER
        // ---------------------------------------------

        if (comboTimer > 0.0f)
        {
            comboTimer -=
                deltaTime;

            if (comboTimer <= 0.0f)
            {
                comboTimer = 0.0f;

                comboCount = 0;
            }
        }


        // ---------------------------------------------
        // LEVEL TIMER
        // ---------------------------------------------

        // Countdown before the race
        if (!raceStarted)
        {
            countdownTimer -= deltaTime;

            if (countdownTimer <= 0.0f)
            {
                countdownTimer = 0.0f;
                raceStarted = true;
            }
        }

        // Shared race clock for all racers
        if (raceStarted &&
            nextFinishPlace <= static_cast<int>(aiRacers.size()) + 1)
        {
            raceElapsedTime += deltaTime;
        }

        // Player's personal race time
        if (raceStarted && !levelComplete)
        {
            levelTime += deltaTime;
        }



        if (input.resetPressed || restartRaceRequested)
        {
            if (levelComplete || restartRaceRequested)
            {
                // =============================================
                // FULL NEW RACE
                // =============================================

                checkpointReached = false;

                respawnPosition =
                    playerRacer.spawnPosition;

                levelTime = 0.0f;

                raceElapsedTime = 0.0f;

                countdownTimer = 3.0f;
                raceStarted = false;

                score = 0;

                checkpointScore = 0;


                ResetBike(
                    playerRacer.bike,
                    playerRacer.spawnPosition
                );

                for (Racer& aiRacer : aiRacers)
                {
                    ResetBike(
                        aiRacer.bike,
                        aiRacer.spawnPosition
                    );
                }

                // =============================================
                // RESET RACER STATE
                // =============================================

                playerRacer.finished = false;
                playerRacer.finishPlace = 0;
                playerRacer.finishTime = -1.0f;
                playerRacer.currentPosition = 0;

                for (Racer& aiRacer : aiRacers)
                {
                    aiRacer.finished = false;
                    aiRacer.finishPlace = 0;
                    aiRacer.finishTime = -1.0f;
                    aiRacer.currentPosition = 0;
                    aiRacer.grounded = true;
                }
                nextFinishPlace = 1;

                for (AIState& aiState : aiStates)
                {
                    aiState = AIState{};
                }


                playerRacer.grounded = true;
                

                cameraX =
                    playerRacer.spawnPosition.x;

                levelComplete = false;

                resultsMenuSelection = 0;
            }
            else
            {
                // =============================================
                // NORMAL CRASH / CHECKPOINT RESPAWN
                // =============================================

                ResetBike(
                    playerRacer.bike,
                    respawnPosition
                );

                playerRacer.grounded = true;

                cameraX =
                    respawnPosition.x;
            }


            // Restore score from checkpoint.
            score =
                checkpointScore;


            // Clear player stunt state.
            airTime =
                0.0f;

            wasBikeGrounded =
                true;

            accumulatedRotation =
                0.0f;

            positiveFlipCompleted =
                false;

            negativeFlipCompleted =
                false;

            stuntText[0] =
                '\0';

            stuntTextTimer =
                0.0f;

            landingText[0] =
                '\0';

            landingTextTimer =
                0.0f;

            particleSystem.Clear();


            if (restartRaceRequested)
            {
                boostMeter = 0.0f;
                boostActive = false;

                comboCount = 0;
                comboTimer = 0.0f;

                wheelieTime = 0.0f;
                wheelieAwarded = false;

                previousBikeAngle = 0.0f;
                flipCount = 0;

                newBestTime = false;

                physicsAccumulator = 0.0f;

                restartRaceRequested = false;
            }

        }


        // -------------------------------------------------
        // FIXED PHYSICS UPDATE
        // -------------------------------------------------

        physicsAccumulator += deltaTime;

        while (physicsAccumulator >= physicsTimeStep)
        {

            InputState raceInput =
                raceStarted ? input : InputState{};

            UpdateBikeControls(
                playerRacer.bike,
                raceInput,
                playerRacer.grounded,
                playerRacer.finished,
                boostActive
            );


            
                
            // =====================================================
            // AI CONTROLS
            // =====================================================

            for (size_t i = 0;
                i < aiRacers.size();
                ++i)
            {
                Racer& aiRacer =
                    aiRacers[i];

                AIState& aiState =
                    aiStates[i];

                AIProfile profile =
                    GetAIProfile(
                        static_cast<int>(i)
                    );



                InputState aiInput{};

                if (raceStarted)
                {
                    aiInput = BuildAIInput(
                        aiRacer.bike,
                        aiRacer.grounded,
                        aiRacer.finished,
                        static_cast<int>(i),
                        physicsTimeStep,
                        aiState,
                        terrain.segments
                    );
                }



                // =============================================
                // TERRAIN-AWARE BUNNY HOP
                // =============================================

                if (aiInput.jumpPressed &&
                    aiRacer.grounded)
                {
                    ApplyBunnyHop(
                        aiRacer.bike,
                        profile.jumpStrength
                    );
                }


                // =============================================
                // DRIVE / STUNT / REAL EARNED BOOST
                // =============================================

                UpdateBikeControls(
                    aiRacer.bike,
                    aiInput,
                    aiRacer.grounded,
                    aiRacer.finished,
                    aiState.boostActive,
                    profile.speedMultiplier,
                    profile.airControlMultiplier
                );
            }

            // ---------------------------------------------
            // BUNNY HOP
            // ---------------------------------------------


            if (raceStarted &&
                raceInput.jumpPressed &&
                playerRacer.grounded &&
                !playerRacer.finished)
            {
                ApplyBunnyHop(
                    playerRacer.bike,
                    3.5f
                );
            }

            // ---------------------------------------------
            // RUN BOX2D
            // ---------------------------------------------

            b2World_Step(
                worldId,
                physicsTimeStep,
                subStepCount
            );


            playerRacer.grounded =
                IsBikeGrounded(
                    playerRacer.bike
                );


            for (size_t i = 0;
                i < aiRacers.size();
                ++i)
            {
                Racer& aiRacer =
                    aiRacers[i];

                AIState& aiState =
                    aiStates[i];

                AIProfile profile =
                    GetAIProfile(
                        static_cast<int>(i)
                    );


                aiRacer.grounded =
                    IsBikeGrounded(
                        aiRacer.bike
                    );


                ApplyAIStabilityAssist(
                    aiRacer.bike,
                    aiRacer.grounded,
                    profile,
                    aiState,
                    terrain.segments
                );
            }
            
            // ---------------------------------------------
            // WHEELIE DETECTION
            // ---------------------------------------------

            bool rearWheelGrounded =
                IsRearWheelGrounded(playerRacer.bike);

            bool frontWheelGrounded =
                IsFrontWheelGrounded(playerRacer.bike);


            b2Vec2 chassisVelocity =
                b2Body_GetLinearVelocity(
                    playerRacer.bike.chassisBodyId
                );


            float currentChassisAngle =
                b2Rot_GetAngle(
                    b2Body_GetRotation(
                        playerRacer.bike.chassisBodyId
                    )
                );


            const float minimumWheelieSpeed =
                1.5f;

            const float minimumWheelieAngle =
                0.25f;


            bool doingWheelie =
                rearWheelGrounded &&
                !frontWheelGrounded &&
                chassisVelocity.x >
                minimumWheelieSpeed &&
                currentChassisAngle >
                minimumWheelieAngle &&
                !levelComplete;


            if (doingWheelie)
            {
                wheelieTime +=
                    physicsTimeStep;


                constexpr float WHEELIE_SCORE_TIME =
                    1.00f;


                if (wheelieTime >= WHEELIE_SCORE_TIME &&
                    !wheelieAwarded)
                {
                    comboCount++;

                    comboTimer =
                        COMBO_WINDOW;


                    const int baseWheelieScore =
                        250;


                    const int stuntScore =
                        baseWheelieScore *
                        comboCount;


                    score +=
                        stuntScore;

                    // Wheelie contributes 25% of the boost meter.
                    boostMeter += 25.0f;

                    if (boostMeter > MAX_BOOST)
                    {
                        boostMeter = MAX_BOOST;
                    }

                    wheelieAwarded = true;

                    b2Vec2 rearWheelPosition =
                        b2Body_GetPosition(
                            playerRacer.bike.rearWheelBodyId
                        );

                    particleSystem.SpawnWheelieDust(
                        rearWheelPosition
                    );

                    if (comboCount > 1)
                    {
                        SDL_snprintf(
                            stuntText,
                            sizeof(stuntText),
                            "WHEELIE! +%d   COMBO x%d",
                            stuntScore,
                            comboCount
                        );
                    }
                    else
                    {
                        SDL_snprintf(
                            stuntText,
                            sizeof(stuntText),
                            "WHEELIE! +%d",
                            stuntScore
                        );
                    }
                }
            }
            else
            {
                wheelieTime = 0.0f;
                wheelieAwarded = false;
            }

            bool justLeftGround =
                wasBikeGrounded &&
                !playerRacer.grounded;

            bool justLanded =
                !wasBikeGrounded &&
                playerRacer.grounded;
            if (justLeftGround)
            {

                SDL_Log("LEFT GROUND");
            }

            if (justLanded)
            {
                SDL_Log(
                    "LANDED | AirTime: %.2f | Rotation: %.2f",
                    airTime,
                    accumulatedRotation
                );
            }

            float currentBikeAngle =
                b2Rot_GetAngle(
                    b2Body_GetRotation(
                        playerRacer.bike.chassisBodyId
                    )
                );

            if (justLeftGround)
            {
                accumulatedRotation = 0.0f;
                previousBikeAngle = currentBikeAngle;

                positiveFlipCompleted = false;
                negativeFlipCompleted = false;

                flipCount = 0;
            }
            if (!playerRacer.grounded)
            {
                float angleDifference =
                    currentBikeAngle -
                    previousBikeAngle;

                constexpr float PI =
                    3.14159265f;

                // Fix angle wrap from +PI to -PI.
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

                accumulatedRotation +=
                    angleDifference;

                constexpr float FLIP_THRESHOLD = 5.5f;

                if (accumulatedRotation >= FLIP_THRESHOLD)
                {
                    positiveFlipCompleted = true;
                }

                if (accumulatedRotation <= -FLIP_THRESHOLD)
                {
                    negativeFlipCompleted = true;
                }

                previousBikeAngle =
                    currentBikeAngle;
            }

            // ---------------------------------------------
            // BOOST ACTIVATION / DRAIN
            // ---------------------------------------------

            if (!boostActive &&
                boostMeter >= MAX_BOOST)
            {
                boostActive = true;
                UpdateBikeControls(
                    playerRacer.bike,
                    input,
                    playerRacer.grounded,
                    playerRacer.finished,
                    boostActive
                );

                boostMeter = MAX_BOOST;

                b2Vec2 rearWheelPosition =
                    b2Body_GetPosition(
                        playerRacer.bike.rearWheelBodyId
                    );

                particleSystem.SpawnBoostBurst(
                    rearWheelPosition
                );

                SDL_snprintf(
                    stuntText,
                    sizeof(stuntText),
                    "BOOST!"
                );

                stuntTextTimer = 1.2f;

                SDL_Log("BOOST ACTIVATED");
            }


            if (boostActive)
            {
                boostMeter -=
                    BOOST_DRAIN_RATE *
                    physicsTimeStep;

                if (boostMeter <= 0.0f)
                {
                    boostMeter = 0.0f;
                    boostActive = false;

                    SDL_Log("BOOST ENDED");
                }
            }
            // ---------------------------------------------
            // AIR TIME
            // ---------------------------------------------

            if (!playerRacer.grounded)
            {
                airTime += physicsTimeStep;
            }

            
                // =========================
                // AIR TIME LANDING
                // =========================

                if (airTime >= 0.5f)
                {
                    SDL_Log(
                        "AIR TIME: %.2f seconds",
                        airTime
                    );
                }
                if (justLanded)
                {
                    SDL_Log(
                        "LANDED | AirTime: %.2f | Rotation: %.2f",
                        airTime,
                        accumulatedRotation
                    );

                    float landingAngularSpeed =
                        b2Body_GetAngularVelocity(
                            playerRacer.bike.chassisBodyId
                        );


                    bool perfectLanding =
                        airTime >= PERFECT_LANDING_MIN_AIRTIME &&

                        std::abs(currentBikeAngle) <=
                        PERFECT_LANDING_MAX_ANGLE &&

                        std::abs(landingAngularSpeed) <=
                        PERFECT_LANDING_MAX_ANGULAR_SPEED;


                    if (positiveFlipCompleted)
                    {
                        comboCount++;

                        comboTimer =
                            COMBO_WINDOW;


                        const int baseFlipScore =
                            500;


                        const int stuntScore =
                            baseFlipScore *
                            comboCount;


                        score +=
                            stuntScore;


                        boostMeter +=
                            50.0f;


                        if (boostMeter > MAX_BOOST)
                        {
                            boostMeter =
                                MAX_BOOST;
                        }


                        SDL_Log(
                            "FRONT FLIP DETECTED | SCORE: %d",
                            score
                        );


                        if (comboCount > 1)
                        {
                            SDL_snprintf(
                                stuntText,
                                sizeof(stuntText),
                                "FRONT FLIP! +%d   COMBO x%d",
                                stuntScore,
                                comboCount
                            );
                        }
                        else
                        {
                            SDL_snprintf(
                                stuntText,
                                sizeof(stuntText),
                                "FRONT FLIP! +%d",
                                stuntScore
                            );
                        }


                        stuntTextTimer =
                            1.2f;
                    }

                    if (negativeFlipCompleted)
                    {
                        comboCount++;

                        comboTimer =
                            COMBO_WINDOW;


                        const int baseFlipScore =
                            500;


                        const int stuntScore =
                            baseFlipScore *
                            comboCount;


                        score +=
                            stuntScore;


                        boostMeter +=
                            50.0f;


                        if (boostMeter > MAX_BOOST)
                        {
                            boostMeter =
                                MAX_BOOST;
                        }


                        SDL_Log(
                            "BACKFLIP DETECTED | SCORE: %d",
                            score
                        );


                        if (comboCount > 1)
                        {
                            SDL_snprintf(
                                stuntText,
                                sizeof(stuntText),
                                "BACKFLIP! +%d   COMBO x%d",
                                stuntScore,
                                comboCount
                            );
                        }
                        else
                        {
                            SDL_snprintf(
                                stuntText,
                                sizeof(stuntText),
                                "BACKFLIP! +%d",
                                stuntScore
                            );
                        }


                        stuntTextTimer =
                            1.2f;
                    }

                    if (perfectLanding)
                    {
                        // If the player currently has no combo,
                        // a clean landing starts one.
                        if (comboCount == 0)
                        {
                            comboCount = 1;
                        }


                        // Keep the current combo alive.
                        comboTimer =
                            COMBO_WINDOW;


                        const int landingBonus =
                            PERFECT_LANDING_BASE_SCORE *
                            comboCount;


                        score +=
                            landingBonus;


                        // Small boost reward for landing cleanly.
                        boostMeter +=
                            10.0f;


                        if (boostMeter > MAX_BOOST)
                        {
                            boostMeter =
                                MAX_BOOST;
                        }


                        SDL_Log(
                            "PERFECT LANDING | BONUS: %d | COMBO: %d",
                            landingBonus,
                            comboCount
                        );


                        // If there was no flip on this landing,
                        // show Perfect Landing as the main stunt text.
                        if (!positiveFlipCompleted &&
                            !negativeFlipCompleted)
                        {
                            if (comboCount > 1)
                            {
                                SDL_snprintf(
                                    landingText,
                                    sizeof(landingText),
                                    "PERFECT LANDING! +%d   COMBO x%d",
                                    landingBonus,
                                    comboCount
                                );
                            }
                            else
                            {
                                SDL_snprintf(
                                    landingText,
                                    sizeof(landingText),
                                    "PERFECT LANDING! +%d",
                                    landingBonus
                                );
                            }


                            landingTextTimer =
                                1.2f;
                        }
                    }

                    // Reset jump data.
                    airTime = 0.0f;
                    accumulatedRotation = 0.0f;
                }

            wasBikeGrounded = playerRacer.grounded;

            if (levelComplete)
            {
                SDL_SetWindowTitle(
                    window,
                    "TorqRift - LEVEL COMPLETE!"
                );
            }

            else if (playerRacer.grounded)
            {
                SDL_SetWindowTitle(
                    window,
                    "TorqRift - GROUND"
                );
            }
            else
            {
                SDL_SetWindowTitle(
                    window,
                    "TorqRift - AIR"
                );
            
            }

            LimitBikeAngularSpeed(
                playerRacer.bike,
                playerRacer.grounded
            );


            for (Racer& aiRacer : aiRacers)
            {
                LimitBikeAngularSpeed(
                    aiRacer.bike,
                    aiRacer.grounded
                );
            }


            physicsAccumulator -= physicsTimeStep;
        }
        

        b2Vec2 playerPosition =
            b2Body_GetPosition(
                playerRacer.bike.chassisBodyId
            );


        environment.Update(
            playerPosition.x,
            deltaTime
        );

        // =================================================
        // FINISH ORDER
        // =================================================

        if (!playerRacer.finished &&
            playerPosition.x >= FINISH_X)
        {
            playerRacer.finished = true;

            playerRacer.finishPlace =
                nextFinishPlace;

            if (playerRacer.finishPlace == 1)
            {
                hasWonHauntedRun = true;
            }
            playerRacer.finishTime = levelTime;

            ++nextFinishPlace;


            // Keep existing UI working for now.
            levelComplete = true;


            newBestTime = false;

            if (bestTime < 0.0f ||
                levelTime < bestTime)
            {
                bestTime = levelTime;
                newBestTime = true;

                SaveBestTime(bestTime);
            }


            // ============================================
            // SAVE COMPLETED RACE PROGRESS
            // ============================================

            saveData.bestTime = bestTime;
            saveData.hauntedRunCompleted = true;
            saveData.hauntedRunWon = hasWonHauntedRun;
            saveData.selectedRider = selectedRider;

            SaveGame(saveData);

        }


        // =================================================
        // AI FINISH ORDER
        // =================================================

        for (Racer& aiRacer : aiRacers)
        {
            b2Vec2 aiPosition =
                b2Body_GetPosition(
                    aiRacer.bike.chassisBodyId
                );


            if (!aiRacer.finished &&
                aiPosition.x >= FINISH_X)
            {
                aiRacer.finished = true;

                aiRacer.finishPlace =
                    nextFinishPlace;


                aiRacer.finishTime = raceElapsedTime;
                ++nextFinishPlace;
            }
        }

        if (!checkpointReached &&
            playerPosition.x >= CHECKPOINT_X)
        {
            checkpointReached = true;

            respawnPosition =
                b2Vec2{ CHECKPOINT_X, -1.5f };

            checkpointScore = score;
        }

        // =================================================
        // LIVE RACE POSITION
        // =================================================

        // -------------------------------------------------
        // PLAYER POSITION
        // -------------------------------------------------

        if (playerRacer.finished)
        {
            playerRacer.currentPosition =
                playerRacer.finishPlace;
        }
        else
        {
            int position = 1;

            for (Racer& aiRacer : aiRacers)
            {
                b2Vec2 aiPosition =
                    b2Body_GetPosition(
                        aiRacer.bike.chassisBodyId
                    );

                // A finished racer is automatically ahead.
                // Otherwise compare track progress.
                if (aiRacer.finished ||
                    aiPosition.x > playerPosition.x)
                {
                    ++position;
                }
            }

            playerRacer.currentPosition =
                position;
        }


        // -------------------------------------------------
        // AI POSITIONS
        // -------------------------------------------------

        for (size_t i = 0; i < aiRacers.size(); ++i)
        {
            Racer& aiRacer =
                aiRacers[i];


            if (aiRacer.finished)
            {
                aiRacer.currentPosition =
                    aiRacer.finishPlace;

                continue;
            }


            b2Vec2 aiPosition =
                b2Body_GetPosition(
                    aiRacer.bike.chassisBodyId
                );


            int position = 1;


            // Is the player ahead of this AI?
            if (playerRacer.finished ||
                playerPosition.x > aiPosition.x)
            {
                ++position;
            }


            // Compare against every other AI racer.
            for (size_t j = 0;
                j < aiRacers.size();
                ++j)
            {
                // Don't compare a racer against itself.
                if (i == j)
                {
                    continue;
                }


                Racer& otherAI =
                    aiRacers[j];


                b2Vec2 otherPosition =
                    b2Body_GetPosition(
                        otherAI.bike.chassisBodyId
                    );


                if (otherAI.finished ||
                    otherPosition.x > aiPosition.x)
                {
                    ++position;
                }
            }


            aiRacer.currentPosition =
                position;
        }


        // Camera gradually catches up to the bike.
        const float cameraFollowSpeed = 3.0f;

        cameraX +=
            (playerPosition.x - cameraX) *
            cameraFollowSpeed *
            deltaTime;
     

        b2Vec2 rearWheelPosition =
            b2Body_GetPosition(
                playerRacer.bike.rearWheelBodyId
            );


        b2Vec2 frontWheelPosition =
            b2Body_GetPosition(
                playerRacer.bike.frontWheelBodyId
            );


        b2Vec2 groundPosition =
            b2Body_GetPosition(
                terrain.groundBodyId
            );

        // =================================================
        // PARTICLE SYSTEM
        // =================================================

        particleSystem.Update(
            deltaTime,
            boostActive,
            rearWheelPosition
        );


        // =================================================
        // REAR WHEEL -> SDL
        // =================================================

        SDL_FPoint rearWheelScreen;

        rearWheelScreen.x =
            CAMERA_TARGET_X +
            (rearWheelPosition.x - cameraX) *
            PIXELS_PER_METER;

        rearWheelScreen.y =
            SCREEN_CENTER_Y -
            rearWheelPosition.y *
            PIXELS_PER_METER;


        // =================================================
        // FRONT WHEEL -> SDL
        // =================================================

        SDL_FPoint frontWheelScreen;

        frontWheelScreen.x =
            CAMERA_TARGET_X +
            (frontWheelPosition.x - cameraX) *
            PIXELS_PER_METER;

        frontWheelScreen.y =
            SCREEN_CENTER_Y -
            frontWheelPosition.y *
            PIXELS_PER_METER;


        // =================================================
        // GROUND -> SDL
        // =================================================

        SDL_FRect groundRect;

        groundRect.w =
            GROUND_HALF_WIDTH * 2.0f *
            PIXELS_PER_METER;

        groundRect.h =
            GROUND_HEIGHT *
            PIXELS_PER_METER;


        groundRect.x =
            CAMERA_TARGET_X +
            (groundPosition.x - cameraX) *
            PIXELS_PER_METER -
            groundRect.w / 2.0f;


        groundRect.y =
            SCREEN_CENTER_Y -
            groundPosition.y *
            PIXELS_PER_METER -
            groundRect.h / 2.0f;


        // =================================================
        // RENDER
        // =================================================

        Render(
            renderer,
            font,
            stuntFont,
            countdownFont,
            bikeTexture,
            wheelTexture,
            secondRiderBikeTexture,
            secondRiderWheelTexture,
            playerRacer.bike.chassisBodyId,
            playerRacer.bike.rearWheelBodyId,
            playerRacer.bike.frontWheelBodyId,
            playerRacer,
            aiRacers,
            aiVisuals,
            selectedRider,
            cameraX,
            environment,
            groundRect,
            rearWheelScreen,
            frontWheelScreen,
            terrain.segments,
            levelComplete,
            checkpointReached,
            levelTime,
            airTime,
            stuntText,
            stuntTextTimer,
            landingText,
            landingTextTimer,
            comboCount,
            comboTimer,
            score,
            bestTime,
            newBestTime,
            resultsMenuSelection,
            boostMeter,
            boostActive,
            playerRacer.currentPosition,
            particleSystem,
            countdownTimer,
            raceStarted
        );
    }


    // =====================================================
    // CLEANUP
    // =====================================================

    b2DestroyWorld(worldId);

    SDL_DestroyTexture(
        bikeTexture
    );

    SDL_DestroyTexture(
        wheelTexture
    );

    SDL_DestroyTexture(
        ghostBikeTexture
    );

    SDL_DestroyTexture(
        ghostWheelTexture
    );

    SDL_DestroyTexture(gravelBikeTexture);
    SDL_DestroyTexture(gravelWheelTexture);

    SDL_DestroyTexture(riotBikeTexture);
    SDL_DestroyTexture(riotWheelTexture);

    SDL_DestroyTexture(secondRiderBikeTexture);
    SDL_DestroyTexture(secondRiderWheelTexture);

    SDL_DestroyRenderer(
        renderer
    );

    SDL_DestroyWindow(
        window
    );

    TTF_CloseFont(
        font
    );

    TTF_CloseFont(countdownFont);

    TTF_Quit();
    SDL_Quit();

    return 0;
}