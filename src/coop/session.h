#ifndef FALLOUT_COOP_SESSION_H_
#define FALLOUT_COOP_SESSION_H_

#include <stdbool.h>
#include <stdint.h>

#define COOP_MAX_PLAYERS (2)

typedef enum CoopRole {
    COOP_ROLE_HOST,
    COOP_ROLE_CLIENT,
} CoopRole;

typedef enum CoopResult {
    COOP_RESULT_OK = 0,
    COOP_RESULT_INVALID_ARGUMENT = -1,
    COOP_RESULT_INVALID_LOCATION = -2,
    COOP_RESULT_DUPLICATE_PLAYER = -3,
    COOP_RESULT_SESSION_FULL = -4,
    COOP_RESULT_PLAYER_NOT_FOUND = -5,
    COOP_RESULT_HOST_ONLY = -6,
    COOP_RESULT_MAP_MISMATCH = -7,
    COOP_RESULT_STALE_SNAPSHOT = -8,
} CoopResult;

typedef struct CoopPlayerState {
    uint32_t id;
    bool connected;
    int map;
    int elevation;
    int tile;
    int rotation;
} CoopPlayerState;

typedef struct CoopSnapshot {
    uint32_t revision;
    int map;
    int elevation;
    int playerCount;
    CoopPlayerState players[COOP_MAX_PLAYERS];
} CoopSnapshot;

typedef struct CoopSession {
    CoopRole role;
    uint32_t localPlayerId;
    uint32_t revision;
    int map;
    int elevation;
    int playerCount;
    CoopPlayerState players[COOP_MAX_PLAYERS];
} CoopSession;

void coop_session_init(CoopSession* session, CoopRole role, uint32_t localPlayerId);
int coop_session_host_add_player(CoopSession* session, uint32_t playerId);
int coop_session_host_remove_player(CoopSession* session, uint32_t playerId);
int coop_session_host_set_map(CoopSession* session, int map, int elevation);
int coop_session_host_update_player(CoopSession* session, uint32_t playerId, int map, int elevation, int tile, int rotation);
int coop_session_make_snapshot(const CoopSession* session, CoopSnapshot* snapshot);
int coop_session_apply_snapshot(CoopSession* session, const CoopSnapshot* snapshot);
const CoopPlayerState* coop_session_get_player(const CoopSession* session, uint32_t playerId);

#endif /* FALLOUT_COOP_SESSION_H_ */
