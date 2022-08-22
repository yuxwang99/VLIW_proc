# VLIW_proc

This work is a scheduler for a simple VLIW processor. The processor will once again run a minimal subset of the RISC-V instruction set, this time with support for loads and stores, enriched with additional hypothetical support for a subset of the Itanium architectural capabilities. 

# Example
Input.json
[
    "mov LC, 100",
    "mov x2, 0x1000",
    "mov x3, 1",
    "mov x4, 25",
    "ld x5, 0(x2)",
    "mulu x6, x5, x4",
    "mulu x3, x3, x5",
    "st x6, 0(x2)",
    "mulu x2, x2, 1",
    "loop 4",
    "st x3, 0(x3)"
]

# Output

mov LC 100     mov x0 0x1000  nop            nop            nop            
mov x1 1       mov x2 25      nop            nop            nop            
nop            nop            nop            ld x3 x0       nop            
nop            nop            mulu x4 x3 x2  nop            nop            
nop            nop            mulu x5 x1 x3  nop            nop            
nop            nop            nop            nop            nop            
nop            nop            nop            st x6 x0       nop            
nop            nop            mulu x7 x0 1   nop            nop            
nop            nop            nop            nop            nop            
mov x0 x7      mov x1 x5      nop            nop            loop 2         
nop            nop            nop            nop            nop  
