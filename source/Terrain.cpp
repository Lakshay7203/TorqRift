#include "Terrain.h"

Terrain CreateTerrain(b2WorldId worldId)
{
    Terrain terrain;

    // =====================================================
    // FLAT GROUND
    // =====================================================

    b2BodyDef groundBodyDef =
        b2DefaultBodyDef();

    groundBodyDef.position =
        b2Vec2{ 0.0f, -16.0f };

    terrain.groundBodyId =
        b2CreateBody(
            worldId,
            &groundBodyDef
        );

    b2ShapeDef groundShapeDef =
        b2DefaultShapeDef();

    groundShapeDef.material.friction =
        0.9f;

    b2Polygon groundBox =
        b2MakeBox(
            TERRAIN_GROUND_HALF_WIDTH,
            TERRAIN_GROUND_HEIGHT / 2.0f
        );

    b2ShapeId groundShapeId =
        b2CreatePolygonShape(
            terrain.groundBodyId,
            &groundShapeDef,
            &groundBox
        );

    b2Shape_SetFriction(
        groundShapeId,
        1.0f
    );


    // =====================================================
    // HILL BODY
    // =====================================================

    b2BodyDef hillBodyDef =
        b2DefaultBodyDef();

    terrain.hillBodyId =
        b2CreateBody(
            worldId,
            &hillBodyDef
        );

    b2ShapeDef hillShapeDef =
        b2DefaultShapeDef();

    hillShapeDef.material.friction =
        1.0f;


    // =====================================================
    // COURSE
    // =====================================================


    terrain.segments =
    {
        // =====================================================
        // ZONE 1 - STARTING AREA
        // Flat start so all racers can accelerate together.
        // =====================================================

        { { -20.0f, -9.0f }, { 8.0f, -9.0f } },


        // =====================================================
        // ZONE 2 - THE ROLLERS
        // Small hills to build momentum and early airtime.
        // =====================================================

        { { 8.0f, -9.0f }, { 14.0f, -7.7f } },
        { { 14.0f, -7.7f }, { 20.0f, -8.7f } },

        { { 20.0f, -8.7f }, { 27.0f, -9.2f } },
        { { 27.0f, -9.2f }, { 34.0f, -6.6f } },
        { { 34.0f, -6.6f }, { 40.0f, -6.0f } },
        { { 40.0f, -6.0f }, { 48.0f, -8.8f } },


        // =====================================================
        // ZONE 3 - THE ROCKY RUN
        // Repeated bumps that challenge rider balance.
        // =====================================================

        { { 48.0f, -8.8f }, { 54.0f, -7.6f } },
        { { 54.0f, -7.6f }, { 59.0f, -8.9f } },

        { { 59.0f, -8.9f }, { 64.0f, -7.2f } },
        { { 64.0f, -7.2f }, { 70.0f, -8.8f } },

        // First major hill.
        { { 70.0f, -8.8f }, { 76.0f, -5.2f } },
        { { 76.0f, -5.2f }, { 83.0f, -3.1f } },
        { { 83.0f, -3.1f }, { 89.0f, -4.1f } },

        // Long, fast descent.
        { { 89.0f, -4.1f }, { 100.0f, -9.0f } },


        // =====================================================
        // ZONE 4 - THE BUNNY HOP CHALLENGE
        // Rapid hills create opportunities for short jumps.
        // =====================================================

        { { 100.0f, -9.0f }, { 107.0f, -9.0f } },

        { { 107.0f, -9.0f }, { 112.0f, -7.8f } },
        { { 112.0f, -7.8f }, { 117.0f, -9.0f } },

        { { 117.0f, -9.0f }, { 122.0f, -7.2f } },
        { { 122.0f, -7.2f }, { 128.0f, -9.0f } },

        { { 128.0f, -9.0f }, { 135.0f, -7.0f } },
        { { 135.0f, -7.0f }, { 142.0f, -9.0f } },


        // =====================================================
        // ZONE 5 - CHECKPOINT
        // Keep a safer landing and recovery area around X=155.
        // =====================================================

        { { 142.0f, -9.0f }, { 146.0f, -8.0f } },
        { { 146.0f, -8.0f }, { 150.0f, -9.0f } },
        { { 150.0f, -9.0f }, { 164.0f, -9.0f } },


        // =====================================================
        // ZONE 6 - THE RIDGELINE
        // Big climb followed by a fast downhill.
        // =====================================================

        { { 164.0f, -9.0f }, { 170.0f, -7.0f } },
        { { 170.0f, -7.0f }, { 177.0f, -3.8f } },

        // Mountain top.
        { { 177.0f, -3.8f }, { 185.0f, -4.4f } },

        // Descend into the valley.
        { { 185.0f, -4.4f }, { 193.0f, -8.5f } },
        { { 193.0f, -8.5f }, { 200.0f, -9.0f } },


        // =====================================================
        // ZONE 7 - LAUNCH MOUNTAIN
        // Your previously tested hill, kept unchanged.
        // =====================================================

        // Approach.
        { { 200.0f, -9.0f }, { 208.0f, -8.0f } },

        // Steep climb.
        { { 208.0f, -8.0f }, { 216.0f, -3.5f } },

        // Mountain crest.
        { { 216.0f, -3.5f }, { 223.0f, -3.5f } },

        // Fast downhill.
        { { 223.0f, -3.5f }, { 233.0f, -9.2f } },


        // =====================================================
        // ZONE 8 - THE STUNT RUN
        // A launch-shaped hill and a downhill landing section.
        // =====================================================

        { { 233.0f, -9.2f }, { 239.0f, -8.6f } },

        // Climb toward the launch crest.
        { { 239.0f, -8.6f }, { 244.0f, -6.2f } },
        { { 244.0f, -6.2f }, { 249.0f, -4.1f } },

        // Short elevated crest.
        { { 249.0f, -4.1f }, { 252.0f, -4.1f } },

        // Downhill landing slope.
        { { 252.0f, -4.1f }, { 261.0f, -9.0f } },

        // Second stunt hill.
        { { 261.0f, -9.0f }, { 268.0f, -7.8f } },
        { { 268.0f, -7.8f }, { 275.0f, -5.0f } },
        { { 275.0f, -5.0f }, { 281.0f, -8.9f } },


        // =====================================================
        // ZONE 9 - FINAL TECHNICAL SPRINT
        // More challenging bumps before the finish.
        // =====================================================

        { { 281.0f, -8.9f }, { 286.0f, -8.9f } },

        { { 286.0f, -8.9f }, { 291.0f, -6.7f } },
        { { 291.0f, -6.7f }, { 295.0f, -8.8f } },

        { { 295.0f, -8.8f }, { 299.0f, -5.8f } },
        { { 299.0f, -5.8f }, { 303.0f, -8.9f } },

        { { 303.0f, -8.9f }, { 309.0f, -9.0f } },


        // =====================================================
        // ZONE 10 - FINISH STRAIGHT
        // Flat runway so racers can complete the race cleanly.
        // FINISH_X remains 315.
        // =====================================================

        { { 309.0f, -9.0f }, { 390.0f, -9.0f } }
    };



    // =====================================================
    // CREATE BOX2D SEGMENTS
    // =====================================================


    // =====================================================
    // CREATE CONNECTED TERRAIN COLLISION
    // =====================================================

    std::vector<b2Vec2> chainPoints;

    chainPoints.reserve(
        terrain.segments.size() + 3
    );

    // Box2D chains are one-sided.
    // Right-to-left gives us upward-facing collision.

    const b2Vec2 rightEnd =
        terrain.segments.back().end;

    const b2Vec2 leftEnd =
        terrain.segments.front().start;

    // Extra endpoint for smooth chain boundaries.
    chainPoints.push_back(
        { rightEnd.x + 10.0f, rightEnd.y }
    );

    // Add the complete terrain in reverse order.
    for (auto it = terrain.segments.rbegin();
        it != terrain.segments.rend();
        ++it)
    {
        chainPoints.push_back(it->end);
    }

    chainPoints.push_back(leftEnd);

    // Extra endpoint at the left boundary.
    chainPoints.push_back(
        { leftEnd.x - 10.0f, leftEnd.y }
    );

    b2SurfaceMaterial material =
        b2DefaultSurfaceMaterial();

    material.friction = 1.0f;

    b2ChainDef chainDef =
        b2DefaultChainDef();

    chainDef.points = chainPoints.data();
    chainDef.materials = &material;
    chainDef.materialCount = 1;
    chainDef.isLoop = false;

    // Support chain definitions from different
    // Box2D 3.x versions.
    auto configureChain = [&]<typename T>(T & def)
    {
        if constexpr (requires { def.count; })
        {
            def.count =
                static_cast<int>(chainPoints.size());
        }
        else
        {
            def.pointCount =
                static_cast<int>(chainPoints.size());
        }

        if constexpr (requires { def.ghost1; def.ghost2; })
        {
            def.ghost1 = {
                chainPoints.front().x + 10.0f,
                chainPoints.front().y
            };

            def.ghost2 = {
                chainPoints.back().x - 10.0f,
                chainPoints.back().y
            };
        }
    };

    configureChain(chainDef);

    b2CreateChain(
        terrain.hillBodyId,
        &chainDef
    );

    return terrain;

}