#include "coop/session.h"
#include "game/map_defs.h"
#include "game/object_types.h"

#include <assert.h>

static void test_host_builds_shared_map_snapshot(void)
{
    CoopSession host;
    CoopSession client;
    CoopSnapshot snapshot;

    coop_session_init(&host, COOP_ROLE_HOST, 1);
    coop_session_init(&client, COOP_ROLE_CLIENT, 2);

    assert(coop_session_host_add_player(&host, 1) == COOP_RESULT_OK);
    assert(coop_session_host_add_player(&host, 2) == COOP_RESULT_OK);
    assert(coop_session_host_set_map(&host, 42, 1) == COOP_RESULT_OK);
    assert(coop_session_host_update_player(&host, 1, 42, 1, 10001, ROTATION_NE) == COOP_RESULT_OK);
    assert(coop_session_host_update_player(&host, 2, 42, 1, 10002, ROTATION_SW) == COOP_RESULT_OK);
    assert(coop_session_make_snapshot(&host, &snapshot) == COOP_RESULT_OK);

    assert(coop_session_apply_snapshot(&client, &snapshot) == COOP_RESULT_OK);
    assert(client.map == 42);
    assert(client.elevation == 1);
    assert(coop_session_get_player(&client, 1)->tile == 10001);
    assert(coop_session_get_player(&client, 2)->rotation == ROTATION_SW);
}

static void test_host_validates_membership_and_location(void)
{
    CoopSession host;

    coop_session_init(&host, COOP_ROLE_HOST, 1);
    assert(coop_session_host_add_player(&host, 1) == COOP_RESULT_OK);
    assert(coop_session_host_add_player(&host, 1) == COOP_RESULT_DUPLICATE_PLAYER);
    assert(coop_session_host_set_map(&host, 42, 1) == COOP_RESULT_OK);
    assert(coop_session_host_update_player(&host, 1, 42, 3, 10001, ROTATION_NE) == COOP_RESULT_MAP_MISMATCH);
    assert(coop_session_host_update_player(&host, 1, 42, 1, HEX_GRID_SIZE, ROTATION_NE) == COOP_RESULT_INVALID_LOCATION);
    assert(coop_session_host_update_player(&host, 99, 42, 1, 10001, ROTATION_NE) == COOP_RESULT_PLAYER_NOT_FOUND);
}

static void test_client_rejects_stale_and_mutating_operations(void)
{
    CoopSession host;
    CoopSession client;
    CoopSnapshot snapshot;

    coop_session_init(&host, COOP_ROLE_HOST, 1);
    coop_session_init(&client, COOP_ROLE_CLIENT, 2);
    assert(coop_session_host_add_player(&host, 1) == COOP_RESULT_OK);
    assert(coop_session_host_add_player(&host, 2) == COOP_RESULT_OK);
    assert(coop_session_host_set_map(&host, 42, 1) == COOP_RESULT_OK);
    assert(coop_session_host_update_player(&host, 1, 42, 1, 10001, ROTATION_NE) == COOP_RESULT_OK);
    assert(coop_session_host_update_player(&host, 2, 42, 1, 10002, ROTATION_NE) == COOP_RESULT_OK);
    assert(coop_session_make_snapshot(&host, &snapshot) == COOP_RESULT_OK);
    assert(coop_session_apply_snapshot(&client, &snapshot) == COOP_RESULT_OK);
    assert(coop_session_apply_snapshot(&client, &snapshot) == COOP_RESULT_STALE_SNAPSHOT);
    assert(coop_session_host_set_map(&client, 43, 1) == COOP_RESULT_HOST_ONLY);
}

int main(void)
{
    test_host_builds_shared_map_snapshot();
    test_host_validates_membership_and_location();
    test_client_rejects_stale_and_mutating_operations();
    return 0;
}
