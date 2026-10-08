# KH Tactics

A Kingdom Hearts: Chain of Memories roguelike tactics ROM hack, US version.
The packaged **0.39 Jump Preview development build** runs in the original 2.5D engine with full
Sora/Donald/Goofy/enemy sprites, world tiles, height, collision, ledges, doors and props.

Rooms are generated from a seed using each world's assets. They are not copies
of the original room layouts. A 12-room graph includes optional branches and
reward rooms; the prototype progresses through Traverse Town, Agrabah and
Castle Oblivion. Movement and attacks have turn budgets, and enemies act when
you end your turn. Combat stays in the field.

Sora, Donald, Goofy, recruited Cloud and original character Rally are selectable party members with separate action and
movement budgets. A shared card deck uses original card artwork and values,
with draw/discard/reload, melee, Fire, Cure, Guard and character bonuses.
Versioned suspend saves preserve native field state and recover from a damaged
latest slot. Party members have individual HP, knockouts and revival; partial encounter
damage and enemy positions persist across backtracking and reboot. This remains a development build;
player walk previews project onto the field and execute through native collision,
including one-point projected diagonals, walkable upper ledges above void,
and solid-prop collision checks. Open previews revalidate animated props.
World-specific groups include Shadow, Red Nocturne, Darkball and Black Fungus.
Traverse Town has original multipart Guard Armor art, warned slams, hand-loss
phases and break effects. Agrabah has a custom sorcerer Jafar encounter using
original field/lamp sprites and a charged single-target spell. Castle Oblivion
has original humanoid Marluxia battle art, a charged scythe sweep and enrage.
The compact field HUD shows available party members and the following enemy phase.
Commands and rewards open contextual menus; End Turn previews incoming damage.
Round-start party assembly uses original character cards; an optional Cloud
challenge offers recruitment or a personal upgrade, and Cloud can replace
either companion. Every second unique room clear grants a personal upgrade.
Original cards support explicit Fire/Cure targets,
matching-type sleights and three-card chest reward choices. Palette budgets
keep party, card artwork and value digits visible beside generated scenery.
The packaged 0.39 ROM passes 79 scoped native checks and three complete
fresh-SRAM campaigns, with native retry, Cloud recruitment/deployment,
suspend/reset and composed descents in each run. The frontend exits0 and
ROM/driver/log hashes are verified. A three-seed all-room/chest sweep remains
running. Prior 0.38's 540 scoped checks and all-room campaign belong to that
version. These tests cover recorded seeds and routes.
See [0.39 patch notes](tactics/JUMP-PREVIEW-0.39.md) and
[release audit](tactics/RELEASE-AUDIT.md) for evidence and remaining work.

The normal HUD occupies 18 pixels at the top and 16 at the bottom. Commands
opens a 40-pixel top panel; Skills and detailed choices use 56 pixels. Ready
party members, the controlled hero and the enemy phase remain visible in
Commands and Skills. Party shows each hero's HP, MOVE and ACT budgets.
Native stairs support several vertical preview segments with total movement
cost, occupancy checks and original climb/descent animation. Composed descent
can continue into walking; arbitrary combined jump/climb path discovery,
additional recruits and final content/visual polish remain unfinished.
Door travel preserves the selected member, every party budget and active Guard;
backtracking cannot refill turn resources. One B press commits a native full-height
jump. Resolved jump previews draw a field landing diamond; stair and ledge
attachments have explicit status labels. Moving jumps have a world-space travel budget and safely hand off to stairs.
Sword hits on open doors preserve the generated room and normal walk-through travel.
Tactical enemies begin on their assigned floor, including the original flying roles.

- [Controls and current scope](tactics/PLAY.md)
- [Build and verification](tactics/BUILD.md)
- [Implementation plan](tactics/PLAN.md)

```sh
.venv/bin/python configure.py --tactics
.venv/bin/ninja
# Open build/tactics-us/kh_tactics.gba in mGBA.
```

The matching build remains available with `configure.py --version us`.
ROMs, extracted assets and build outputs stay local.

---

# Kingdom Hearts: Chain of Memories (GBA)

[![Build Status]][actions] [![us]][progress] [![jp]][progress] [![eu]][progress]

[Build Status]: https://github.com/pheenoh/khcom/actions/workflows/build.yml/badge.svg
[actions]: https://github.com/pheenoh/khcom/actions/workflows/build.yml

[us]: https://decomp.dev/pheenoh/khcom/us.svg?mode=shield&label=us
[jp]: https://decomp.dev/pheenoh/khcom/jp.svg?mode=shield&label=jp
[eu]: https://decomp.dev/pheenoh/khcom/eu.svg?mode=shield&label=eu
[progress]: https://decomp.dev/pheenoh/khcom

<!-- markdownlint-disable MD033 -->
[<img src="https://decomp.dev/pheenoh/khcom/us.svg?w=512&h=256" width="512" height="256" alt="Progress graph for the us version">][progress]
<!-- markdownlint-enable MD033 -->

A matching decompilation of *Kingdom Hearts: Chain of Memories*
for the Game Boy Advance.

> [!IMPORTANT]
> This repository does **not** contain any game assets or ROMs. An existing
> copy of the game is required to build.

The project can target the following versions:

| Version | Code | SHA-1 |
|---------|------|-------|
| `us`    | B8CE | `10729bd884f8fdca7a310b6d606c52e46657aa48` |
| `jp`    | B8CJ | `59ec0a0a4ccd1e6acb3bbd7bfb21d63988958cfa` |
| `eu`    | B8CP | `8db73586cdb11b3795907edebf43228dbcd3e6b2` |

## Dependencies

- git
- ninja
- python3
- PyYAML (`python3 -m pip install pyyaml`)
- `binutils-arm-none-eabi`
- [agbcc](https://github.com/pret/agbcc):

  ```sh
  git clone https://github.com/pret/agbcc
  cd agbcc && ./build.sh && ./install.sh ../khcom
  ```

## Building

- Clone the repository:

  ```sh
  git clone https://github.com/pheenoh/khcom.git
  ```

- Copy your legally dumped ROM(s) into `roms/` as `<code>.gba` (e.g. `roms/B8CE.gba`).

- Extract assets:

  ```sh
  python3 tools/extract_assets.py
  ```

- Configure:

  ```sh
  python3 configure.py
  ```

  To use a version other than `us`, specify it with `--version`.

- Build:

  ```sh
  ninja
  ```

## License

This project is released under the [CC0 1.0 Universal](LICENSE.md) license.

### Castle boss work after 0.16

The current source replaces Castle room 7 with humanoid Marluxia using the
original battle idle, scythe windup and attack animation assets. His horizontal
sweep reaches 96 pixels, spans 16 pixels in projected depth and respects a
24-pixel height difference. Damage increases from 10 to 14 at half health;
Guard and the threat preview use the same resolution rules. The encounter
preserves Sora, Donald, Goofy and original card artwork.

`tools/tactics_marluxia_smoke.py` generates an explicit emulator fixture from the
built ELF. Its 28 checks pass, including resource allocation, all-party damage,
range/depth/height boundaries, enrage, Guard, charged suspend/resume, native Fire
defeat and room-exit cleanup. Fixtures write setup state and are not full-run
proof. Evidence: `build/tactics-us/marluxia-verified-evidence/boss.txt`.
The local 0.19 development patch includes this encounter and the verified
all-room/chest full run.

Current source also supports recruiting Cloud in an optional original-sprite
battle and deploying him in either companion slot through the round-start card
screen. Character health, turn resources and upgrades follow identities across
swaps and suspend saves. Sixteen native deployment checks pass. A fresh input-only starter-party run
completes all three worlds with suspend/reset and full roster verification
(PASS at frame 108486). A second fresh input-only run fights, recruits and deploys Cloud, verifies
suspend/reset, and finishes all three worlds (PASS at frame 92877). The packaged 0.19 development patch includes this work. See `tactics/PARTY-ROGUELIKE.md` for remaining scope.
