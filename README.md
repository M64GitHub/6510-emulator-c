# 6510-emulator
A simple MOS6510 (Commodore 64) CPU emulator in C. Easily extendable. Perfect for embedded systems!

The test program contains a small routine executing a loop:
```
0800: A9 0A                       LDA #$0A        ; 2 
0802: AA                          TAX             ; 2 
0803: E8                          INX             ; 2 loop
0804: E0 14                       CPX #$14        ; 2 
0806: D0 FB                       BNE $0803       ; 2/3 
0808: 60                          RTS             ; 6 
```

Output of main_cpu_test:
```
Initializing CPU:
[CPU] [0800] [OP:00] [CYC:0] A: 00, X: 00, Y: 00, CYC-TT: 0
Writing Program ...
[CPU] [0800] [OP:00] [CYC:0] A: 00, X: 00, Y: 00, CYC-TT: 9
Running CPU:
[CPU] [0802] [OP:A9] [CYC:2] A: 0A, X: 00, Y: 00, CYC-TT: 11
[CPU] [0803] [OP:AA] [CYC:2] A: 0A, X: 0A, Y: 00, CYC-TT: 13
[CPU] [0804] [OP:E8] [CYC:2] A: 0A, X: 0B, Y: 00, CYC-TT: 15
[CPU] [0806] [OP:E0] [CYC:2] A: 0A, X: 0B, Y: 00, CYC-TT: 17
[CPU] [0803] [OP:D0] [CYC:3] A: 0A, X: 0B, Y: 00, CYC-TT: 20
[CPU] [0804] [OP:E8] [CYC:2] A: 0A, X: 0C, Y: 00, CYC-TT: 22
[CPU] [0806] [OP:E0] [CYC:2] A: 0A, X: 0C, Y: 00, CYC-TT: 24
[CPU] [0803] [OP:D0] [CYC:3] A: 0A, X: 0C, Y: 00, CYC-TT: 27
[CPU] [0804] [OP:E8] [CYC:2] A: 0A, X: 0D, Y: 00, CYC-TT: 29
[CPU] [0806] [OP:E0] [CYC:2] A: 0A, X: 0D, Y: 00, CYC-TT: 31
[CPU] [0803] [OP:D0] [CYC:3] A: 0A, X: 0D, Y: 00, CYC-TT: 34
[CPU] [0804] [OP:E8] [CYC:2] A: 0A, X: 0E, Y: 00, CYC-TT: 36
[CPU] [0806] [OP:E0] [CYC:2] A: 0A, X: 0E, Y: 00, CYC-TT: 38
[CPU] [0803] [OP:D0] [CYC:3] A: 0A, X: 0E, Y: 00, CYC-TT: 41
[CPU] [0804] [OP:E8] [CYC:2] A: 0A, X: 0F, Y: 00, CYC-TT: 43
[CPU] [0806] [OP:E0] [CYC:2] A: 0A, X: 0F, Y: 00, CYC-TT: 45
[CPU] [0803] [OP:D0] [CYC:3] A: 0A, X: 0F, Y: 00, CYC-TT: 48
[CPU] [0804] [OP:E8] [CYC:2] A: 0A, X: 10, Y: 00, CYC-TT: 50
[CPU] [0806] [OP:E0] [CYC:2] A: 0A, X: 10, Y: 00, CYC-TT: 52
[CPU] [0803] [OP:D0] [CYC:3] A: 0A, X: 10, Y: 00, CYC-TT: 55
[CPU] [0804] [OP:E8] [CYC:2] A: 0A, X: 11, Y: 00, CYC-TT: 57
[CPU] [0806] [OP:E0] [CYC:2] A: 0A, X: 11, Y: 00, CYC-TT: 59
[CPU] [0803] [OP:D0] [CYC:3] A: 0A, X: 11, Y: 00, CYC-TT: 62
[CPU] [0804] [OP:E8] [CYC:2] A: 0A, X: 12, Y: 00, CYC-TT: 64
[CPU] [0806] [OP:E0] [CYC:2] A: 0A, X: 12, Y: 00, CYC-TT: 66
[CPU] [0803] [OP:D0] [CYC:3] A: 0A, X: 12, Y: 00, CYC-TT: 69
[CPU] [0804] [OP:E8] [CYC:2] A: 0A, X: 13, Y: 00, CYC-TT: 71
[CPU] [0806] [OP:E0] [CYC:2] A: 0A, X: 13, Y: 00, CYC-TT: 73
[CPU] [0803] [OP:D0] [CYC:3] A: 0A, X: 13, Y: 00, CYC-TT: 76
[CPU] [0804] [OP:E8] [CYC:2] A: 0A, X: 14, Y: 00, CYC-TT: 78
[CPU] [0806] [OP:E0] [CYC:2] A: 0A, X: 14, Y: 00, CYC-TT: 80
[CPU] [0808] [OP:D0] [CYC:2] A: 0A, X: 14, Y: 00, CYC-TT: 82
[CPU] [0001] [OP:60] [CYC:6] A: 0A, X: 14, Y: 00, CYC-TT: 88
```
