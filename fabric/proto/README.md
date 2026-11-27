# Wire schemas

Versioned Protocol Buffers schemas for the Pluribus wire protocol.

## Files

- `handshake.proto` — bootstrap `Hello`, `Welcome` and `Reject` messages. Compiled into
  `pluribusfabric` as the handshake codec.

## Regeneration

The generated `handshake.pb.h` / `handshake.pb.cc` are committed so the default build does not need
the `protoc` compiler, only `libprotobuf`. Regenerate with Protobuf `v3.21.12` after editing the
schema:

```sh
protoc --cpp_out=. --proto_path=. handshake.proto
```

Keep the committed generated files in sync with the schema; the dependency check does not regenerate
them automatically.
