# Breadcaster & Catcher

A paired Windows C++17 experiment for forwarding keyboard and mouse input to another machine. **Breadcaster** captures events in a configured desktop region; **Catcher** validates and optionally applies them.

Both programs now build together, share a documented binary protocol, and have explicit test modes.

## Build

Windows 10 version 1703+, C++17 and CMake 3.16+ are required.

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

With Visual Studio, executables are in `build\Release`; Ninja/MinGW-w64 places them directly in `build`. Existing project files remain available.

## Local protocol check

In one terminal:

```powershell
.\build\Release\catcher.exe --once
```

In another:

```powershell
.\build\Release\breadcaster.exe --send 127.0.0.1 "3 100 200"
```

The sender translates the three CLI integers into one **12-byte binary datagram**. Catcher defaults to dry run on `127.0.0.1:9009`; this example does not move the pointer.

## Forwarding between machines

On the receiving machine, choose its local interface and the sender address:

```powershell
catcher.exe --bind 192.168.1.20 --peer 192.168.1.10 --apply
```

On the sending machine:

```powershell
breadcaster.exe --forward 192.168.1.20 1920 9009
```

The width argument defines a region immediately **left of the primary display**: X from `-width` through `-1`. It defaults to the primary display width, and accepts 1–32768. An actual monitor or virtual desktop region must exist there. Remote X is `local X + width`; Y is unchanged.

Keys/buttons remain active locally by default. Add `--suppress-local` to suppress successfully queued events in the remote region. Stop Breadcaster with **Ctrl+Alt+F12** or Ctrl+C; stop Catcher with Ctrl+C. The sender attempts to release forwarded inputs and restore remote cursors on normal shutdown, and the receiver tracks its own applied state for cleanup.

Both programs accept a chosen port. Catcher's `--port 0` selects and prints an ephemeral port. `breadcaster --check` briefly installs and removes hooks without sending data.

## Binary protocol

The format retains compatibility with the original Windows 32-bit integer layout: exactly **three signed 32-bit little-endian values**, with no terminator or padding.

| First integer | Second integer | Third integer |
| --- | --- | --- |
| 0: mouse button | Win32 flag: 2/4 left, 8/16 right, 32/64 middle | 0 |
| 1: key down | Virtual key 1–254 | 0 |
| 2: key up | Virtual key 1–254 | 0 |
| 3: pointer move | Screen X | Screen Y |
| 4: vertical wheel | Signed delta, conventionally ±120 | 0 |
| 5: restore cursors | 0 | 0 |
| 6: hide cursors | 0 | 0 |

[binary_protocol.hpp](binary_protocol.hpp) encodes endianness explicitly and rejects wrong lengths and invalid commands. This differs from both the three-field *text* Remote_Receiver format and the seven-field Instruction-Receiver format.

The previous downward-wheel packet used the wrong opcode; both wheel directions now use opcode 4 and preserve the actual delta.

## Source layout

- [Breadcaster src](Breadcaster%20src) — CLI, initialized cursor sampling, scoped hooks and one nonblocking UDP sender.
- [Catcher src](Catcher%20src) — exact datagram reception, explicit dry-run/apply selection and Win32 execution.
- [command.hpp](command.hpp), [command.cpp](command.cpp) — shared command validation and CLI text parsing.
- [binary_protocol.hpp](binary_protocol.hpp) — the fixed wire format.
- [tests/protocol_test.cpp](tests/protocol_test.cpp) — binary, signed-value, queue and loopback tests.

Hooks only enqueue fixed-size commands. Network operations run in the message loop outside callbacks. A bounded queue counts overflows. The console stays visible; no background startup mechanism is installed.

## Limits and historical binaries

The protocol is unencrypted and unauthenticated. UDP can lose or reorder key-up packets; IP filtering does not authenticate a sender. Use this only in a controlled environment. Stop/restart the receiver if input state gets stuck. Forced termination or desktop restrictions can prevent cleanup.

[Windows x64 Executables](Windows%20x64%20Executables) contains **historical binaries**, not builds of the current source. Its bundled virtual-display driver and installer are retained with their own license and instructions; they have not been executed or validated during maintenance. Build the current programs from source. They do not require installing that bundled driver.

## Validation and license

Strict C++17 compilation, exact binary loopback tests, hook installation/removal and a paired CLI dry run are checked locally. Sustained interactive forwarding, cursor replacement and driver behavior are not covered.

The existing [GPL version 2 license](LICENSE) is retained. The bundled driver has its own [license](Windows%20x64%20Executables/Virtual%20Desktop%20Screen%20DDI/License.txt).
