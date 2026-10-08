# KH Tactics

Current patch: [0.41 Aladdin notes](tactics/ALADDIN-0.41.md). Original Aladdin sprites/card, Jafar recruit-or-power reward, skirmish movement recovery and backward-compatible six-hero saves. 655 current-build native checks and three complete all-room campaigns pass.

A Kingdom Hearts: Chain of Memories roguelike tactics ROM hack, US version.
The packaged **0.41 Aladdin release** runs in the original 2.5D engine with full
Sora/Donald/Goofy/enemy sprites, world tiles, height, collision, ledges, doors and props.

Rooms are generated from a seed using each world's assets. They are not copies
of the original room layouts. A 12-room graph includes optional branches and
reward rooms; the run progresses through Traverse Town, Agrabah and
Castle Oblivion. Movement and attacks have turn budgets, and enemies act when
you end your turn. Combat stays in the field.

Sora, Donald, Goofy, recruited Cloud/Aladdin and original character Rally are selectable party members with separate action and
movement budgets. A shared card deck uses original card artwork and values,
with draw/discard/reload, melee, Fire, Cure, Guard and character bonuses.
Versioned suspend saves preserve native field state and recover from a damaged
latest slot. Party members have individual HP, knockouts and revival; partial encounter
damage and enemy positions persist across backtracking and reboot. Player walk previews project onto the field and execute through native collision,
including one-point projected diagonals, walkable upper ledges above void,
and solid-prop collision checks. Open previews revalidate animated props.
World-specific groups include Shadow, Red Nocturne, Darkball and Black Fungus.
Traverse Town has original multipart Guard Armor art, warned slams, hand-loss
phases and break effects. Agrabah has a custom sorcerer Jafar encounter using
original field/lamp sprites and a charged single-target spell. Castle Oblivion
has original humanoid Marluxia battle art, a charged scythe sweep and enrage.
The compact field HUD shows available party members and the following enemy phase.
Commands and rewards open contextual menus; End Turn previews incoming damage.
Round-start party assembly uses original character cards. Cloud’s optional
challenge and Jafar’s boss reward offer recruitment instead of a personal upgrade.
Both recruited allies can replace either companion. Every second unique room clear grants a personal upgrade.
Original cards support explicit Fire/Cure targets,
matching-type sleights and three-card chest reward choices. Palette budgets
keep party, card artwork and value digits visible beside generated scenery.
The current 0.41 ROM passes 655 scoped native checks covering all six heroes,
card combat, bosses, height previews, contextual menus, rewards and native saves.
Its BPS applies byte-identically to the tested ROM. Three consecutive complete
input-only campaigns pass with all 36 rooms, nine chests, both recruits,
suspend/reset, composed descent and native seed retries. Frontend exit and exact
ROM/driver/log hashes are verified.
See [0.41 notes](tactics/ALADDIN-0.41.md) and
[release audit](tactics/RELEASE-AUDIT.md) for version-scoped evidence.

The normal HUD occupies 18 pixels at the top and 16 at the bottom. Commands
opens a 40-pixel top panel; Skills and detailed choices use 56 pixels. Ready
party members, the controlled hero and the enemy phase remain visible in
Commands, Skills and detailed confirmations. Party shows each hero's HP, MOVE and ACT budgets.
Native stairs support several vertical preview segments with total movement
cost, occupancy checks and original climb/descent animation. Composed descent
can continue into walking. Ascent, jumps and prop departure retain their
separate controls and previews.
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
  git clone --branch tactics/bootstrap https://github.com/thomasjvu/khcom.git
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
