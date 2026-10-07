# KH Tactics

A Kingdom Hearts: Chain of Memories roguelike tactics ROM hack, US version.
The current **0.3 native party development build** runs in the original 2.5D engine with full
Sora/Donald/Goofy/enemy sprites, world tiles, height, collision, ledges, doors and props.

Rooms are generated from a seed using each world's assets. They are not copies
of the original room layouts. A 12-room graph includes optional branches and
reward rooms; the prototype progresses through Traverse Town, Agrabah and
Castle Oblivion. Movement and attacks have turn budgets, and enemies act when
you end your turn. Combat stays in the field.

Sora, Donald and Goofy are selectable party members with separate action and
movement budgets. A shared card deck uses original card artwork and values,
with draw/discard/reload, melee, Fire, Cure, Guard and character bonuses.
Versioned suspend saves preserve native field state and recover from a damaged
latest slot. Party HP is currently shared. This remains a development build;
physical route guarantees, previews, sleights and complete-run QA are pending.

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
