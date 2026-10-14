# Fallout 2 co-op project

This branch uses [Fallout 2 Reference Edition](https://github.com/alexbatalov/fallout2-re)
as the engine baseline. The upstream source is kept recognizable so that changes
can be compared against upstream as the co-op work develops.

## Initial direction

The first milestone is a two-player session that can load the same Fallout 2
installation and keep the players' world state consistent. Before adding
replication code, the project needs to settle these boundaries:

- **Authority:** a host-authoritative simulation is the default candidate.
- **Turn model:** decide whether both players can act in real time or whether
  combat and menus pause the shared simulation.
- **State scope:** decide which actors, inventory changes, scripts, map changes,
  combat rolls, and dialog choices are shared.
- **Compatibility:** keep the original save format available unless a separate
  co-op save format becomes necessary.

## Suggested implementation phases

1. Add a transport-independent session layer under `src/coop/`.
2. Define a versioned message format and connection lifecycle without touching
   gameplay state.
3. Add a host-side command queue and deterministic sequencing for player input.
4. Replicate the smallest useful state slice: player identity, map position,
   movement, and disconnect handling.
5. Expand replication to combat, inventory, scripts, dialogs, and save/load.
6. Add a Windows build and a repeatable two-process local test harness.

## Current baseline

- Upstream source: `alexbatalov/fallout2-re`
- Working branch: `codex/fallout2-coop`
- Build system: CMake
- Runtime target: Windows; the original game data files are required
- License: retain `LICENSE.md` and the upstream notices in all distributions

The first code change should follow from the authority and turn-model choices
above. Until those are fixed, networking code would risk being coupled to the
wrong simulation model.

## Milestone 1: shared-map state slice

The initial host-authoritative state model now lives in `src/coop/session.c`.
It supports two connected players, shared map/elevation validation, player
tile and rotation updates, monotonic revisions, immutable snapshots, and
client-side rejection of stale snapshots. The code has no transport or game
loop dependency, so it can be tested before the Windows executable is built.

Build and run the isolated test with:

```sh
cmake -S . -B build -DFALLOUT2_RE_BUILD_COOP_TESTS=ON
cmake --build build --target coop-session-test
ctest --test-dir build -R coop-session-test --output-on-failure
```
