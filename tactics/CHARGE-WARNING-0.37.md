# Development patch 0.37

Apply `build/release/kh-tactics-0.37-charge-warning-dev.bps` to the original US
ROM SHA1 `10729bd884f8fdca7a310b6d606c52e46657aa48`. Patch 96230 bytes;
application reproduces the32MiB built ROM byte-identically. SHA256:
`f92bc47131331b1eb168b5b668b6e14c7ebbe8edc6c3457bb1c97c8b8b1e6a2d`.

A charging enemy adds one named warning row to the compact idle HUD. The
ready-party/enemy strip stays visible. Header26/footer16 leave118/160 pixels
unobscured; clearing the charge restores the18-pixel header. Windup names
cover Guardian, all armor phases, Jafar, Cloud and Marluxia normal/rage.

172 scoped native checks pass:58 UI and114 boss/Cloud checks, including
actual rendered VRAM warnings, budgets, resource lifetime and suspend/reset.
Three input-only campaigns PASS434168: stable victories129518 HP76,311898
HP76 and434168 HP80. Each requires actual Cloud recruit/deploy, exact suspend/
reset and composed descent. Emulator exited0; ROM/driver/log hashes verified.

All-room replay ended in Sora defeat137348 in Agrabah optional room10. The
failure is preserved. Revised driver prefers a legal finishing Fire against
the final enemy only when native damage confirms the kill; native rules are
unchanged. Its full all-room/nine-chest replay is pending. This remains a
development patch, with final polish and current all-room verification open.
