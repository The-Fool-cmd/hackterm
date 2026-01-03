#ifndef INCLUDE_SERVER_H_
#define INCLUDE_SERVER_H_

#include <stdint.h>
#include "core_result.h"

#define SERVER_NAME_LEN 32  /**< Maximum length of a server name. */
#define SERVER_MAX_LINKS 16 /**< Maximum number of links a server can have. */

/**
 * @brief Type representing a server ID.
 */
typedef int ServerId;

#define SERVER_INVALID_ID (-1) /**< Represents an invalid server ID. */

/**
 * @brief Represents a single connection from one server to another.
 */
typedef struct {
    ServerId to; /**< ID of the linked server. */
} ServerLink;

/**
 * @brief Represents a server in the game network.
 *
 * Stores identity, gameplay stats, network connections, and other info.
 */
typedef struct {
    ServerId id;                /**< Unique identifier for the server. */
    char name[SERVER_NAME_LEN]; /**< Human-readable name of the server. */

    /* Gameplay stats */
    int security; /**< Security level of the server. */
    int money;    /**< Resource amount associated with the server. */

    /* Network topology */
    ServerLink links[SERVER_MAX_LINKS]; /**< Array of links to other servers. */
    int link_count;                     /**< Number of active links. */

    /* Future-proofing */
    int subnet_id; /**< Reserved for future use (currently unused). */
} Server;

/* ---------------- HELPERS ---------------- */

/**
 * @brief Returns a pointer to a linked server.
 *
 * @param s Pointer to the source server.
 * @param all Array of all servers.
 * @param index Index of the link in @p s->links to follow.
 * @return Pointer to the linked Server, or NULL if invalid.
 */
const Server* server_get_linked(const Server* s, const Server* all, int index);

/* ---------------- LIFECYCLE ---------------- */

/**
 * @brief Initializes a server with an ID and name.
 *
 * @param s Pointer to the Server to initialize.
 * @param id ID to assign.
 * @param name Name of the server.
 */
void server_init(Server* s, ServerId id, const char* name);

/* ---------------- NETWORK ---------------- */

/**
 * @brief Adds a unidirectional link from this server to another.
 *
 * @param s Pointer to the Server.
 * @param to ID of the server to link to.
 * @return CORE_OK on success, otherwise a CoreResult error code.
 */
CoreResult server_add_link(Server* s, ServerId to);

/**
 * @brief Adds a bidirectional link between two servers.
 *
 * @param a Pointer to the first server.
 * @param b Pointer to the second server.
 * @return CORE_OK on success, otherwise a CoreResult error code.
 */
CoreResult server_link_bidirectional(Server* a, Server* b);

/* ---------------- RANDOM ---------------- */

/**
 * @brief Generates a new server with a random ID.
 *
 * @param servers Array of existing servers.
 * @param server_count Pointer to the number of servers (updated on success).
 * @param name Name of the new server.
 * @return ID of the generated server.
 */
ServerId server_generate_random(Server* servers, int* server_count, const char* name);

#endif  // INCLUDE_SERVER_H_
