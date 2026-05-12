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


| VM Type            | Engine-style Name |
| ------------------ | ----------------- |
| Stack + Upvalue    | CascadeVM       |
| Register + Upvalue | NovaVM          |
| Register + Context | AtlasVM         |
| Stack + Context    | TitanVM         |


class VM {};
class StackVM : public VM {};
class RegisterVM : public VM {};

class StackUpvalueVM : public StackVM {};
class RegisterUpvalueVM : public RegisterVM {};

class StackExecutionContextVM : public StackVM {};
class RegisterExecutionContextVM : public RegisterVM {};


Parser
 ↓
AST
 ↓
Semantic Analyzer
 ↓
HIR
 ↓
Type Inference
 ↓
MIR
 ↓
Optimization Passes
 ↓
Register Allocation
 ↓
Bytecode IR
 ↓
TurboBytecode
 ↓
VM


%1 = load b
%2 = load c
%3 = mul %1, %2
%4 = load a
%5 = add %4, %3
