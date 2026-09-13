# Deterministic parser fuzz replay

Seeds are repository-authored grammar fragments. `make fuzz-replay` replays all
inputs without libFuzzer; `make fuzz-smoke` is the bounded sanitizer CI command.
Promote each finding as a seed and a normal regression before closing it.
