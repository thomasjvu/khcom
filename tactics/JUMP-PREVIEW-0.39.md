# Jump previews 0.39 development

Apply `build/release/kh-tactics-0.39-jump-preview-dev.bps` to the original US ROM (SHA1 `10729bd884f8fdca7a310b6d606c52e46657aa48`). Patch size is 98,240 bytes. Application matches the current native build byte for byte.

Resolved jumps draw a landing diamond on the original 2.5D field. Stair and ledge transitions display ATTACH TO STAIRS and CATCH LEDGE instead of promising an ordinary landing. Unknown routes withhold the marker. The compact idle HUD and ready-party strip remain unchanged.

ROM SHA256: `35953e2f0493cfd1e6e1c14bef9cda3d10c1e7974210c569ca6f0ca955349572`.

Exact-ROM verification:

- 79 scoped native checks: stair forecast12, ledge forecast7, five-hero area recipe matrix60. Fixtures disclose saved approaches, cards, enemy HP/positions and Cloud unlock; deployment, stocking and execution use native inputs.
- Three fresh native campaigns PASS379750; frontend exited0. Every run includes Cloud recruitment/deployment, suspend/reset, composed descent and a stable terminal victory. ROM/driver/log hashes verified in `attachment-ui-three-runs-evidence`.
- BPS application matches the build; host rules/worldgen/save/party/route/deck/enemy/roster/jump tests pass.

The three-seed all-room/chest sweep failed900001 in second-run Agrabah room11 after completing its first run. The final native snapshot shows busy0, move3, action1 and no enemies; navigation/chest decision investigation remains open. Previous 0.38's 540 checks and all-room result belong to that version. This development patch does not establish universal physical reachability, every dynamic collision case, or final content and visual polish.
