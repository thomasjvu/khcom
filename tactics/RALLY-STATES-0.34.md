# Development patch 0.34

Apply `build/release/kh-tactics-0.34-rally-states-dev.bps` to the original US
ROM SHA1 `10729bd884f8fdca7a310b6d606c52e46657aa48`. Patch96168 bytes;
application reproduces32MiB built ROM byte-identically. ROM SHA256
`5f1a98008c3f8e9e6d27f12466fe4744de8035a34743cd7f543d9563bd3f846b`.

Rally has original recruitment card, walking/strike/air and dedicated front/
rear hurt, casting and climbing poses. Non-Key cards use cast; actual HP
loss selects recoil; native stair/ledge states select a held climb pose.
Original30 frames/palette remain byte-identical. Native49 checks, encoded
asset/golden verifier and host suite pass. Reserved RAM8140/8192 bytes.

Two complete fresh/native-retry campaigns pass288347: first stable victory
123214, native retry123226 seed2658846982, second victory288227 with Sora67
HP stable120 frames. Both require Cloud recruitment/deployment, exact suspend/
reset and composed descent. Generated headers distinguish tactics sRawKeys
from its vanilla duplicate. The independent all-room run ends in party defeat
130336; no all-room0.34 success is claimed. A new full36-room/nine-chest replay
uses Cure for nearby injured/KO companions and remains under300000 frames.
Earlier0.33 all-room proof remains a separate exact-ROM result. This is a
development patch; wider seed reliability, party/combat balance and final
visual/content review remain unfinished.
