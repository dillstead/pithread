// halt
.globl  halt
        
halt:
        wfe
        b       halt
        
