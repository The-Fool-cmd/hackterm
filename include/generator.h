/* generator.h - procedural network generation API */
#ifndef INCLUDE_GENERATOR_H_
#define INCLUDE_GENERATOR_H_

#include "game.h"

/* Parameters controlling generation. Fields are optional; caller may set 0
 * or negative values to use sensible defaults.
 */
typedef struct {
    int isp_count;
    int pop_count;
    int neigh_min;
    int neigh_max;
    int areas_min; /* number of areas per PoP (optional extra layer) */
    int areas_max;
    int buildings_min;
    int buildings_max;
    int routers_per_building_min;
    int routers_per_building_max;
    int users_per_router_min;
    int users_per_router_max;
    int floors_per_building_min; /* floors within a building (optional) */
    int floors_per_building_max;
    double inter_router_link_density;
    double public_dmz_fraction;
} GeneratorParams;

/* Generate a network using explicit parameters and an optional seed.
 * If seed == 0 the global RNG is used; otherwise the seed will be applied
 * for deterministic generation.
 */
void generator_generate_with_params(GameState* g, const GeneratorParams* params, unsigned int seed);

/* Convenience: generate a city-like network with sensible defaults. Use
 * seed==0 to rely on the program RNG or provide a seed for reproducible
 * output.
 */
void generator_generate_city(GameState* g, unsigned int seed);

#endif // INCLUDE_GENERATOR_H_
