            ┌──────────────┐
            │  Source Code  │
            └──────┬───────┘
                   │
          ┌────────▼────────┐
          │    Compiler     │
          └───┬────────┬────┘
              │        │
     VM Bytecode   UI Bytecode
        (logic)      (UI tree)
              │        │
              ▼        ▼
            VM     ArdanUI Runtime
                        │
                        ▼
                 SDL / Cocoa / Win32