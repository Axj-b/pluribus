# Wire schemas

Versioned Protocol Buffers schemas for the Pluribus wire protocol.

## Files

- `handshake.proto` — bootstrap `Hello`, `Welcome` and `Reject` messages. Compiled into
  `pluribusfabric` as the handshake codec.
- `control.proto` — version-1 control `Envelope`, numeric `MessageType` registry, portable `Error`
  and the initial node bodies. Compiled into `pluribusfabric` as the control codec.

## Regeneration

The generated `handshake.pb.*` and `control.pb.*` files are committed so the default build does not
need the `protoc` compiler, only `libprotobuf`. Regenerate with Protobuf `v3.21.12` after editing a
schema:

```sh
protoc --cpp_out=. --proto_path=. handshake.proto
protoc --cpp_out=. --proto_path=. control.proto
```

Keep the committed generated files in sync with the schema; the dependency check does not regenerate
them automatically.
