#include "coop/session.h"

#include "game/map_defs.h"
#include "game/object_types.h"

#include <string.h>

static int coop_session_find_player(const CoopSession* session, uint32_t playerId)
{
    for (int index = 0; index < session->playerCount; index++) {
        if (session->players[index].id == playerId) {
            return index;
        }
    }

    return -1;
}

static bool coop_location_is_valid(int elevation, int tile, int rotation)
{
    return elevationIsValid(elevation)
        && hexGridTileIsValid(tile)
        && rotation >= 0
        && rotation < ROTATION_COUNT;
}

static bool coop_snapshot_is_valid(const CoopSnapshot* snapshot)
{
    if (snapshot->revision == 0
        || snapshot->map < 0
        || !elevationIsValid(snapshot->elevation)
        || snapshot->playerCount < 0
        || snapshot->playerCount > COOP_MAX_PLAYERS) {
        return false;
    }

    for (int index = 0; index < snapshot->playerCount; index++) {
        const CoopPlayerState* player = &(snapshot->players[index]);
        if (player->id == 0
            || !player->connected
            || player->map != snapshot->map
            || player->elevation != snapshot->elevation
            || !coop_location_is_valid(player->elevation, player->tile, player->rotation)) {
            return false;
        }

        for (int otherIndex = 0; otherIndex < index; otherIndex++) {
            if (snapshot->players[otherIndex].id == player->id) {
                return false;
            }
        }
    }

    return true;
}

void coop_session_init(CoopSession* session, CoopRole role, uint32_t localPlayerId)
{
    memset(session, 0, sizeof(*session));
    session->role = role;
    session->localPlayerId = localPlayerId;
    session->map = -1;
    session->elevation = -1;
}

int coop_session_host_add_player(CoopSession* session, uint32_t playerId)
{
    if (session == NULL || playerId == 0) {
        return COOP_RESULT_INVALID_ARGUMENT;
    }
    if (session->role != COOP_ROLE_HOST) {
        return COOP_RESULT_HOST_ONLY;
    }
    if (coop_session_find_player(session, playerId) != -1) {
        return COOP_RESULT_DUPLICATE_PLAYER;
    }
    if (session->playerCount == COOP_MAX_PLAYERS) {
        return COOP_RESULT_SESSION_FULL;
    }

    CoopPlayerState* player = &(session->players[session->playerCount]);
    memset(player, 0, sizeof(*player));
    player->id = playerId;
    player->connected = true;
    player->map = session->map;
    player->elevation = session->elevation;
    session->playerCount++;
    session->revision++;

    return COOP_RESULT_OK;
}

int coop_session_host_remove_player(CoopSession* session, uint32_t playerId)
{
    if (session == NULL || playerId == 0) {
        return COOP_RESULT_INVALID_ARGUMENT;
    }
    if (session->role != COOP_ROLE_HOST) {
        return COOP_RESULT_HOST_ONLY;
    }

    int index = coop_session_find_player(session, playerId);
    if (index == -1) {
        return COOP_RESULT_PLAYER_NOT_FOUND;
    }

    for (; index < session->playerCount - 1; index++) {
        session->players[index] = session->players[index + 1];
    }
    memset(&(session->players[session->playerCount - 1]), 0, sizeof(session->players[0]));
    session->playerCount--;
    session->revision++;

    return COOP_RESULT_OK;
}

int coop_session_host_set_map(CoopSession* session, int map, int elevation)
{
    if (session == NULL || map < 0 || !elevationIsValid(elevation)) {
        return COOP_RESULT_INVALID_ARGUMENT;
    }
    if (session->role != COOP_ROLE_HOST) {
        return COOP_RESULT_HOST_ONLY;
    }

    session->map = map;
    session->elevation = elevation;
    for (int index = 0; index < session->playerCount; index++) {
        session->players[index].map = map;
        session->players[index].elevation = elevation;
    }
    session->revision++;

    return COOP_RESULT_OK;
}

int coop_session_host_update_player(CoopSession* session, uint32_t playerId, int map, int elevation, int tile, int rotation)
{
    if (session == NULL || playerId == 0) {
        return COOP_RESULT_INVALID_ARGUMENT;
    }
    if (session->role != COOP_ROLE_HOST) {
        return COOP_RESULT_HOST_ONLY;
    }
    if (session->map != map || session->elevation != elevation) {
        return COOP_RESULT_MAP_MISMATCH;
    }
    if (!coop_location_is_valid(elevation, tile, rotation)) {
        return COOP_RESULT_INVALID_LOCATION;
    }

    int index = coop_session_find_player(session, playerId);
    if (index == -1) {
        return COOP_RESULT_PLAYER_NOT_FOUND;
    }

    CoopPlayerState* player = &(session->players[index]);
    player->map = map;
    player->elevation = elevation;
    player->tile = tile;
    player->rotation = rotation;
    session->revision++;

    return COOP_RESULT_OK;
}

int coop_session_make_snapshot(const CoopSession* session, CoopSnapshot* snapshot)
{
    if (session == NULL || snapshot == NULL || session->map < 0 || session->revision == 0) {
        return COOP_RESULT_INVALID_ARGUMENT;
    }

    snapshot->revision = session->revision;
    snapshot->map = session->map;
    snapshot->elevation = session->elevation;
    snapshot->playerCount = session->playerCount;
    memcpy(snapshot->players, session->players, sizeof(snapshot->players));

    return COOP_RESULT_OK;
}

int coop_session_apply_snapshot(CoopSession* session, const CoopSnapshot* snapshot)
{
    if (session == NULL || snapshot == NULL) {
        return COOP_RESULT_INVALID_ARGUMENT;
    }
    if (session->role != COOP_ROLE_CLIENT) {
        return COOP_RESULT_HOST_ONLY;
    }
    if (snapshot->revision <= session->revision) {
        return COOP_RESULT_STALE_SNAPSHOT;
    }
    if (!coop_snapshot_is_valid(snapshot)) {
        return COOP_RESULT_INVALID_ARGUMENT;
    }

    session->revision = snapshot->revision;
    session->map = snapshot->map;
    session->elevation = snapshot->elevation;
    session->playerCount = snapshot->playerCount;
    memcpy(session->players, snapshot->players, sizeof(session->players));

    return COOP_RESULT_OK;
}

const CoopPlayerState* coop_session_get_player(const CoopSession* session, uint32_t playerId)
{
    if (session == NULL || playerId == 0) {
        return NULL;
    }

    int index = coop_session_find_player(session, playerId);
    return index == -1 ? NULL : &(session->players[index]);
}
