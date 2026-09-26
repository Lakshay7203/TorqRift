#pragma once

#include <SDL3/SDL.h>

class Environment
{
public:

    Environment();

    void Update(
        float playerX,
        float deltaTime
    );

    void Render(
        SDL_Renderer* renderer,
        float cameraX
    ) const;

    void SetTerrainDrawColor(
        SDL_Renderer* renderer,
        Uint8 red,
        Uint8 green,
        Uint8 blue
    ) const;


private:

    struct EnvironmentTheme
    {
        SDL_Color skyColor;

        SDL_Color cloudColor;

        SDL_FColor farHillColor;

        SDL_FColor farMountainColor;

        SDL_FColor nearMountainColor;

        SDL_FColor foregroundColor;

        SDL_FColor terrainTint;
    };

    float animationTime = 0.0f;

    void RenderFireflies(
        SDL_Renderer* renderer
    ) const;

    void RenderMoonGlow(
        SDL_Renderer* renderer
    ) const;

    void RenderClouds(
        SDL_Renderer* renderer,
        float cameraX,
        const EnvironmentTheme& theme
    ) const;


    void DrawCloud(
        SDL_Renderer* renderer,
        float x,
        float y,
        float scale,
        SDL_Color color
    ) const;


    void RenderFarHills(
        SDL_Renderer* renderer,
        float cameraX,
        const EnvironmentTheme& theme
    ) const;

    float playerX = 0.0f;


    EnvironmentTheme dayTheme;
    EnvironmentTheme sunsetTheme;
    EnvironmentTheme nightTheme;


    float Clamp01(float value) const;


    SDL_Color LerpColor(
        const SDL_Color& start,
        const SDL_Color& end,
        float amount
    ) const;


    SDL_FColor LerpColor(
        const SDL_FColor& start,
        const SDL_FColor& end,
        float amount
    ) const;


    EnvironmentTheme GetCurrentTheme() const;

    void RenderSun(
        SDL_Renderer* renderer
    ) const;

    void RenderMoon(
        SDL_Renderer* renderer
    ) const;

    void RenderStars(
        SDL_Renderer* renderer
    ) const;

    void DrawFilledCircle(
        SDL_Renderer* renderer,
        float centerX,
        float centerY,
        float radius,
        SDL_Color color
    ) const;

    void RenderForeground(
        SDL_Renderer* renderer,
        float cameraX,
        const EnvironmentTheme& theme
    ) const;


    void DrawTree(
        SDL_Renderer* renderer,
        float x,
        float y,
        float scale,
        SDL_FColor color
    ) const;


    void DrawBush(
        SDL_Renderer* renderer,
        float x,
        float y,
        float scale,
        SDL_FColor color
    ) const;


    void DrawRock(
        SDL_Renderer* renderer,
        float x,
        float y,
        float scale,
        SDL_FColor color
    ) const;


};