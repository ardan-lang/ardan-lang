# arm64

🧩 2. ARM64 CPU Registers

| Register      | Purpose                                                 |
| ------------- | ------------------------------------------------------- |
| `x0`–`x30`    | General-purpose 64-bit registers                        |
| `w0`–`w30`    | Lower 32 bits of each `x` register                      |
| `sp`          | Stack pointer                                           |
| `lr`          | Link register (stores return address on function calls) |
| `pc`          | Program counter (next instruction address)              |
| `xzr` / `wzr` | Always zero (reads as 0, writes discarded)              |


```cpp
#include <cstring>
#include <iostream>
#include <sys/mman.h>

// On Apple Silicon (ARM64), returning 3 can be done like this:
// mov w0, #3     -> sets return value to 3 (in w0, the return register)
// ret             -> returns to caller
//
// Encodings:
// mov w0, #3  =  0x52800060
// ret         =  0xD65F03C0

int main() {
    // ARM64 machine code for:
    // mov w0, #3
    // ret
    uint32_t code[] = {
        0x52800060, // mov w0, #3
        0xD65F03C0  // ret
    };

    // Allocate executable memory
    void* exec = mmap(nullptr, 4096,
                      PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (exec == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Copy machine code into memory
    memcpy(exec, code, sizeof(code));

    // Create function pointer and call it
    int (*func)() = (int (*)())exec;
    int result = func();

    std::cout << "Result: " << result << std::endl;  // should print 3

    munmap(exec, 4096);
    return 0;
}

```
