#pragma once

#include <SDL3/SDL.h>

class Environment
{
public:

    Environment();

    void Update(float playerX);

    void Render(
        SDL_Renderer* renderer,
        float cameraX
    ) const;


private:

    struct EnvironmentTheme
    {
        SDL_Color skyColor;

        SDL_FColor farMountainColor;

        SDL_FColor nearMountainColor;
    };

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
};