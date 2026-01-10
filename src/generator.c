/* generator.c - procedural network generation implementation */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "generator.h"
#include "server.h"

/* Helper to choose an int in [min..max] (handles min==max) */
static int rand_range(int min, int max) {
    if (max <= min) return min;
    return min + (rand() % (max - min + 1));
}

/* Helper: identify router-like devices at file scope */
static int is_router_like(ServerType t) {
    return (t == SERVER_TYPE_ROUTER || t == SERVER_TYPE_RACK);
}

void generator_generate_with_params(GameState* g, const GeneratorParams* p, unsigned int seed) {
    if (!g) return;

    GeneratorParams params;
    if (p) params = *p; else {
        /* defaults */
        params.isp_count = 1;
        params.areas_min = 1;
        params.areas_max = 2;
        params.neigh_min = 3;
        params.neigh_max = 6;
        params.buildings_min = 4;
        params.buildings_max = 10;
        params.floors_per_building_min = 1;
        params.floors_per_building_max = 3;
        params.routers_per_building_min = 1;
        params.routers_per_building_max = 4;
        params.users_per_router_min = 8;
        params.users_per_router_max = 32;
        params.inter_router_link_density = 0.00;
        params.public_dmz_fraction = 0.02;
    }

    /* apply seed locally if set (0 means don't reseed) */
    unsigned int used_seed = seed;
    if (used_seed != 0) srand(used_seed);

    char name_buf[128];

    /* Create ISP nodes */
    int isp_count = params.isp_count > 0 ? params.isp_count : 1;
    int isp_ids[isp_count];
    for (int i = 0; i < isp_count; i++) {
        snprintf(name_buf, sizeof(name_buf), "isp%d", i + 1);
        int id = server_generate_random(g->servers, &g->server_count, name_buf);
        if (id == SERVER_INVALID_ID) break;
        g->servers[id].type = SERVER_TYPE_ISP;
        isp_ids[i] = id;
    }

    /* For each ISP, create areas -> neighborhoods -> buildings -> floors/routers -> users */
    for (int pidx = 0; pidx < isp_count; pidx++) {
        int areas = rand_range(params.areas_min, params.areas_max);
        for (int a = 0; a < areas; a++) {
            snprintf(name_buf, sizeof(name_buf), "area%d_i%d", a + 1, pidx + 1);
            int aid = server_generate_random(g->servers, &g->server_count, name_buf);
            if (aid == SERVER_INVALID_ID) break;
            g->servers[aid].type = SERVER_TYPE_AREA;
            /* link area to ISP (no PoP layer) */
            server_link_bidirectional(&g->servers[aid], &g->servers[isp_ids[pidx]]);

                int neigh = rand_range(params.neigh_min, params.neigh_max);
            for (int n = 0; n < neigh; n++) {
                snprintf(name_buf, sizeof(name_buf), "neigh%d_a%d_p%d", n + 1, a + 1, pidx + 1);
                int nid = server_generate_random(g->servers, &g->server_count, name_buf);
                if (nid == SERVER_INVALID_ID) break;
                g->servers[nid].type = SERVER_TYPE_NEIGHBORHOOD;
                /* link neighborhood to area */
                server_link_bidirectional(&g->servers[nid], &g->servers[aid]);

                    /* no PoP layer: nothing to record here */

                int blds = rand_range(params.buildings_min, params.buildings_max);
                for (int b = 0; b < blds; b++) {
                    snprintf(name_buf, sizeof(name_buf), "bld%d_n%d_a%d_p%d", b + 1, n + 1, a + 1, pidx + 1);
                    int bid = server_generate_random(g->servers, &g->server_count, name_buf);
                    if (bid == SERVER_INVALID_ID) break;
                    g->servers[bid].type = SERVER_TYPE_BUILDING;
                    /* link building to neighborhood */
                    server_link_bidirectional(&g->servers[bid], &g->servers[nid]);

                    int floors = rand_range(params.floors_per_building_min, params.floors_per_building_max);
                    if (floors <= 1) {
                        /* No explicit floors: create routers directly under building */
                        int rtrs = rand_range(params.routers_per_building_min, params.routers_per_building_max);
                        for (int r = 0; r < rtrs; r++) {
                            snprintf(name_buf, sizeof(name_buf), "rtr_b%d_n%d_a%d_p%d_r%d", b + 1, n + 1, a + 1, pidx + 1, r + 1);
                            int rid = server_generate_random(g->servers, &g->server_count, name_buf);
                            if (rid == SERVER_INVALID_ID) break;
                            g->servers[rid].type = SERVER_TYPE_ROUTER;
                            /* mark router subnet as building id so we can keep links scoped */
                            g->servers[rid].subnet_id = bid;
                            /* link router to building */
                            server_link_bidirectional(&g->servers[rid], &g->servers[bid]);

                            int users = rand_range(params.users_per_router_min, params.users_per_router_max);
                            if (users <= 0) continue;
                            /* Attach users directly to this router (no ToR layer).
                             * This yields: floor -> router -> hosts
                             */
                            for (int u = 0; u < users; u++) {
                                snprintf(name_buf, sizeof(name_buf), "usr%d", g->server_count + 1);
                                int uid = server_generate_random(g->servers, &g->server_count, name_buf);
                                if (uid == SERVER_INVALID_ID) break;
                                server_link_bidirectional(&g->servers[uid], &g->servers[rid]);
                                g->servers[uid].subnet_id = bid;
                                g->servers[uid].type = SERVER_TYPE_USER;
                                g->servers[uid].service_count = 0;
                            }
                        }
                    } else {
                        /* Create floor nodes, attach routers to floors */
                        for (int f = 0; f < floors; f++) {
                            snprintf(name_buf, sizeof(name_buf), "floor%d_b%d_n%d_a%d_p%d", f + 1, b + 1, n + 1, a + 1, pidx + 1);
                            int fid = server_generate_random(g->servers, &g->server_count, name_buf);
                            if (fid == SERVER_INVALID_ID) break;
                            g->servers[fid].type = SERVER_TYPE_FLOOR;
                            server_link_bidirectional(&g->servers[fid], &g->servers[bid]);

                            int rtrs = rand_range(params.routers_per_building_min, params.routers_per_building_max);
                            for (int r = 0; r < rtrs; r++) {
                                snprintf(name_buf, sizeof(name_buf), "rtr_floor%d_b%d_n%d_a%d_p%d_r%d", f + 1, b + 1, n + 1, a + 1, pidx + 1, r + 1);
                                int rid = server_generate_random(g->servers, &g->server_count, name_buf);
                                if (rid == SERVER_INVALID_ID) break;
                                g->servers[rid].type = SERVER_TYPE_ROUTER;
                                    /* mark router subnet as building id so we can keep links scoped */
                                    g->servers[rid].subnet_id = bid;
                                /* link router to floor */
                                server_link_bidirectional(&g->servers[rid], &g->servers[fid]);

                                int users = rand_range(params.users_per_router_min, params.users_per_router_max);
                                if (users <= 0) continue;
                                /* Attach users directly to this router (no ToR layer). */
                                for (int u = 0; u < users; u++) {
                                    snprintf(name_buf, sizeof(name_buf), "usr%d", g->server_count + 1);
                                    int uid = server_generate_random(g->servers, &g->server_count, name_buf);
                                    if (uid == SERVER_INVALID_ID) break;
                                    server_link_bidirectional(&g->servers[uid], &g->servers[rid]);
                                    g->servers[uid].subnet_id = bid;
                                    g->servers[uid].type = SERVER_TYPE_USER;
                                    g->servers[uid].service_count = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

        /* Rare inter-router links to create some mesh within neighbourhoods.
         * Only link router-like devices so we don't connect ISPs/POPs/area nodes
         * directly to users/hosts. This keeps the hierarchical layering intact.
         */
        int total = g->server_count;
        for (int a = 1; a < total; a++) {
            for (int b = a + 1; b < total; b++) {
                if (!is_router_like(g->servers[a].type)) continue;
                if (!is_router_like(g->servers[b].type)) continue;
                if (((double)rand() / (double)RAND_MAX) < params.inter_router_link_density) {
                    /* optional: prefer linking routers in same subnet if known */
                    if (g->servers[a].subnet_id != -1 && g->servers[a].subnet_id == g->servers[b].subnet_id) {
                        server_link_bidirectional(&g->servers[a], &g->servers[b]);
                    } else {
                        server_link_bidirectional(&g->servers[a], &g->servers[b]);
                    }
                }
            }
        }

        /* DMZ/public exposure step removed: keep topology strictly hierarchical
         * (ISP -> Area -> Neighborhood -> Building -> Floor -> Router -> Host).
         */
}

void generator_generate_city(GameState* g, unsigned int seed) {
    GeneratorParams params = {0};
    /* set explicit sensible defaults */
    params.isp_count = 1;
    params.areas_min = 1;
    params.areas_max = 2;
    params.neigh_min = 3;
    params.neigh_max = 6;
    params.buildings_min = 4;
    params.buildings_max = 8;
    params.floors_per_building_min = 1;
    params.floors_per_building_max = 2;
    params.routers_per_building_min = 1;
    params.routers_per_building_max = 3;
    params.users_per_router_min = 8;
    params.users_per_router_max = 24;
    params.inter_router_link_density = 0.01;
    params.public_dmz_fraction = 0.02;

    generator_generate_with_params(g, &params, seed);
}
