# canopenctl

`canopenctl <interface> <nodeID> <command> [command_args...]`

**Command**:

- `reset_app`   - reset Node application
- `reset_comm`  - reset Node communication
- `pre_operational`- set Node pre-operational state
- `operational` - set Node operational state
- `stop` - set Node stop state
- `read <index> <subindex> [type]` - read SDO
- `write <index> <subindex> <type> <value>` - write SDO

**Example:**

```bash
canopenctl can0 1 start
canopenctl can0 1 write 0x1234 12 u32 
canopenctl can0 1 read 0x1234 12
    0       1   2  3     4      5     6     7
```

**Supported var types:**

- b
- i8
- i16
- i32
- u8
- u16
- u32
- r32
- vs (16 byte)
- d
