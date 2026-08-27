fe_0               00000000  push     rbp
fe_0               00000001  mov      rbp, rsp
fe_0               00000004  sub      rsp, 0x20
fe_0               0000000b  mov      qword ptr [rbp + 0x10], rcx
fe_0               0000000f  movabs   rax, 0x28
fe_0               00000019  mov      r8, rax
fe_0               0000001c  mov      eax, 0
fe_0               00000021  mov      r11, rax
fe_0               00000024  mov      rax, qword ptr [rbp + 0x10]
fe_0               00000028  mov      r10, rax
fe_0               0000002b  mov      rcx, r10
fe_0               0000002e  mov      rdx, r11
fe_0               00000031  call     0x36
fe_0               00000036  leave    
fe_0               00000037  ret      
fe_0               00000038  add      dword ptr [rdx + rax], eax
fe_0               0000003b  add      eax, 0x50010304
fe_1               00000040  push     rbp
fe_1               00000041  mov      rbp, rsp
fe_1               00000044  sub      rsp, 0x20
fe_1               0000004b  mov      qword ptr [rbp + 0x10], rcx
fe_1               0000004f  movabs   rax, 0x28
fe_1               00000059  mov      r8, rax
fe_1               0000005c  mov      eax, 0
fe_1               00000061  mov      r11, rax
fe_1               00000064  mov      rax, qword ptr [rbp + 0x10]
fe_1               00000068  mov      r10, rax
fe_1               0000006b  mov      rcx, r10
fe_1               0000006e  mov      rdx, r11
fe_1               00000071  call     0x76
fe_1               00000076  mov      rax, qword ptr [rbp + 0x10]
fe_1               0000007a  mov      ecx, 1
fe_1               0000007f  mov      dword ptr [rax], ecx
fe_1               00000081  leave    
fe_1               00000082  ret      
fe_copy            00000083  push     rbp
fe_copy            00000084  mov      rbp, rsp
fe_copy            00000087  sub      rsp, 0x20
fe_copy            0000008e  mov      qword ptr [rbp + 0x10], rcx
fe_copy            00000092  mov      qword ptr [rbp + 0x18], rdx
fe_copy            00000096  movabs   rax, 0x28
fe_copy            000000a0  mov      r8, rax
fe_copy            000000a3  mov      rax, qword ptr [rbp + 0x18]
fe_copy            000000a7  mov      r11, rax
fe_copy            000000aa  mov      rax, qword ptr [rbp + 0x10]
fe_copy            000000ae  mov      r10, rax
fe_copy            000000b1  mov      rcx, r10
fe_copy            000000b4  mov      rdx, r11
fe_copy            000000b7  call     0xbc
fe_copy            000000bc  leave    
fe_copy            000000bd  ret      
fe_add             000000be  push     rbp
fe_add             000000bf  mov      rbp, rsp
fe_add             000000c2  sub      rsp, 0x10
fe_add             000000c9  mov      qword ptr [rbp + 0x10], rcx
fe_add             000000cd  mov      qword ptr [rbp + 0x18], rdx
fe_add             000000d1  mov      qword ptr [rbp + 0x20], r8
fe_add             000000d5  mov      eax, 0
fe_add             000000da  mov      dword ptr [rbp - 4], eax
fe_add             000000dd  mov      eax, dword ptr [rbp - 4]
fe_add             000000e0  cmp      eax, 0xa
fe_add             000000e3  jge      0x141
fe_add             000000e9  jmp      0xfc
fe_add             000000ee  mov      eax, dword ptr [rbp - 4]
fe_add             000000f1  mov      rcx, rax
fe_add             000000f4  add      eax, 1
fe_add             000000f7  mov      dword ptr [rbp - 4], eax
fe_add             000000fa  jmp      0xdd
fe_add             000000fc  mov      eax, dword ptr [rbp - 4]
fe_add             000000ff  movsxd   rax, eax
fe_add             00000102  shl      rax, 2
fe_add             00000106  mov      rcx, qword ptr [rbp + 0x10]
fe_add             0000010a  add      rcx, rax
fe_add             0000010d  mov      eax, dword ptr [rbp - 4]
fe_add             00000110  movsxd   rax, eax
fe_add             00000113  shl      rax, 2
fe_add             00000117  mov      rdx, qword ptr [rbp + 0x18]
fe_add             0000011b  add      rdx, rax
fe_add             0000011e  mov      eax, dword ptr [rbp - 4]
fe_add             00000121  movsxd   rax, eax
fe_add             00000124  shl      rax, 2
fe_add             00000128  mov      qword ptr [rbp - 0x10], rcx
fe_add             0000012c  mov      rcx, qword ptr [rbp + 0x20]
fe_add             00000130  add      rcx, rax
fe_add             00000133  mov      eax, dword ptr [rdx]
fe_add             00000135  mov      edx, dword ptr [rcx]
fe_add             00000137  add      eax, edx
fe_add             00000139  mov      rcx, qword ptr [rbp - 0x10]
fe_add             0000013d  mov      dword ptr [rcx], eax
fe_add             0000013f  jmp      0xee
fe_add             00000141  leave    
fe_add             00000142  ret      
fe_sub             00000143  push     rbp
fe_sub             00000144  mov      rbp, rsp
fe_sub             00000147  sub      rsp, 0x10
fe_sub             0000014e  mov      qword ptr [rbp + 0x10], rcx
fe_sub             00000152  mov      qword ptr [rbp + 0x18], rdx
fe_sub             00000156  mov      qword ptr [rbp + 0x20], r8
fe_sub             0000015a  mov      eax, 0
fe_sub             0000015f  mov      dword ptr [rbp - 4], eax
fe_sub             00000162  mov      eax, dword ptr [rbp - 4]
fe_sub             00000165  cmp      eax, 0xa
fe_sub             00000168  jge      0x1c6
fe_sub             0000016e  jmp      0x181
fe_sub             00000173  mov      eax, dword ptr [rbp - 4]
fe_sub             00000176  mov      rcx, rax
fe_sub             00000179  add      eax, 1
fe_sub             0000017c  mov      dword ptr [rbp - 4], eax
fe_sub             0000017f  jmp      0x162
fe_sub             00000181  mov      eax, dword ptr [rbp - 4]
fe_sub             00000184  movsxd   rax, eax
fe_sub             00000187  shl      rax, 2
fe_sub             0000018b  mov      rcx, qword ptr [rbp + 0x10]
fe_sub             0000018f  add      rcx, rax
fe_sub             00000192  mov      eax, dword ptr [rbp - 4]
fe_sub             00000195  movsxd   rax, eax
fe_sub             00000198  shl      rax, 2
fe_sub             0000019c  mov      rdx, qword ptr [rbp + 0x18]
fe_sub             000001a0  add      rdx, rax
fe_sub             000001a3  mov      eax, dword ptr [rbp - 4]
fe_sub             000001a6  movsxd   rax, eax
fe_sub             000001a9  shl      rax, 2
fe_sub             000001ad  mov      qword ptr [rbp - 0x10], rcx
fe_sub             000001b1  mov      rcx, qword ptr [rbp + 0x20]
fe_sub             000001b5  add      rcx, rax
fe_sub             000001b8  mov      eax, dword ptr [rdx]
fe_sub             000001ba  mov      edx, dword ptr [rcx]
fe_sub             000001bc  sub      eax, edx
fe_sub             000001be  mov      rcx, qword ptr [rbp - 0x10]
fe_sub             000001c2  mov      dword ptr [rcx], eax
fe_sub             000001c4  jmp      0x173
fe_sub             000001c6  leave    
fe_sub             000001c7  ret      
fe_cswap           000001c8  push     rbp
fe_cswap           000001c9  mov      rbp, rsp
fe_cswap           000001cc  sub      rsp, 0x10
fe_cswap           000001d3  mov      qword ptr [rbp + 0x10], rcx
fe_cswap           000001d7  mov      qword ptr [rbp + 0x18], rdx
fe_cswap           000001db  mov      qword ptr [rbp + 0x20], r8
fe_cswap           000001df  mov      eax, 0
fe_cswap           000001e4  mov      ecx, dword ptr [rbp + 0x20]
fe_cswap           000001e7  sub      eax, ecx
fe_cswap           000001e9  mov      dword ptr [rbp - 4], eax
fe_cswap           000001ec  mov      eax, 0
fe_cswap           000001f1  mov      dword ptr [rbp - 8], eax
fe_cswap           000001f4  mov      eax, dword ptr [rbp - 8]
fe_cswap           000001f7  cmp      eax, 0xa
fe_cswap           000001fa  jge      0x279
fe_cswap           00000200  jmp      0x213
fe_cswap           00000205  mov      eax, dword ptr [rbp - 8]
fe_cswap           00000208  mov      rcx, rax
fe_cswap           0000020b  add      eax, 1
fe_cswap           0000020e  mov      dword ptr [rbp - 8], eax
fe_cswap           00000211  jmp      0x1f4
fe_cswap           00000213  mov      eax, dword ptr [rbp - 8]
fe_cswap           00000216  movsxd   rax, eax
fe_cswap           00000219  shl      rax, 2
fe_cswap           0000021d  mov      rcx, qword ptr [rbp + 0x10]
fe_cswap           00000221  add      rcx, rax
fe_cswap           00000224  mov      eax, dword ptr [rbp - 8]
fe_cswap           00000227  movsxd   rax, eax
fe_cswap           0000022a  shl      rax, 2
fe_cswap           0000022e  mov      rdx, qword ptr [rbp + 0x18]
fe_cswap           00000232  add      rdx, rax
fe_cswap           00000235  mov      eax, dword ptr [rcx]
fe_cswap           00000237  mov      ecx, dword ptr [rdx]
fe_cswap           00000239  xor      eax, ecx
fe_cswap           0000023b  mov      ecx, dword ptr [rbp - 4]
fe_cswap           0000023e  and      eax, ecx
fe_cswap           00000240  mov      dword ptr [rbp - 0xc], eax
fe_cswap           00000243  mov      eax, dword ptr [rbp - 8]
fe_cswap           00000246  movsxd   rax, eax
fe_cswap           00000249  shl      rax, 2
fe_cswap           0000024d  mov      rcx, qword ptr [rbp + 0x10]
fe_cswap           00000251  add      rcx, rax
fe_cswap           00000254  mov      eax, dword ptr [rcx]
fe_cswap           00000256  mov      edx, dword ptr [rbp - 0xc]
fe_cswap           00000259  xor      eax, edx
fe_cswap           0000025b  mov      dword ptr [rcx], eax
fe_cswap           0000025d  mov      eax, dword ptr [rbp - 8]
fe_cswap           00000260  movsxd   rax, eax
fe_cswap           00000263  shl      rax, 2
fe_cswap           00000267  mov      rcx, qword ptr [rbp + 0x18]
fe_cswap           0000026b  add      rcx, rax
fe_cswap           0000026e  mov      eax, dword ptr [rcx]
fe_cswap           00000270  mov      edx, dword ptr [rbp - 0xc]
fe_cswap           00000273  xor      eax, edx
fe_cswap           00000275  mov      dword ptr [rcx], eax
fe_cswap           00000277  jmp      0x205
fe_cswap           00000279  leave    
fe_cswap           0000027a  ret      
fe_mul             0000027b  push     rbp
fe_mul             0000027c  mov      rbp, rsp
fe_mul             0000027f  sub      rsp, 0x1b0
fe_mul             00000286  mov      qword ptr [rbp + 0x10], rcx
fe_mul             0000028a  mov      qword ptr [rbp + 0x18], rdx
fe_mul             0000028e  mov      qword ptr [rbp + 0x20], r8
fe_mul             00000292  mov      rax, qword ptr [rbp + 0x18]
fe_mul             00000296  mov      ecx, dword ptr [rax]
fe_mul             00000298  movsxd   rcx, ecx
fe_mul             0000029b  mov      qword ptr [rbp - 8], rcx
fe_mul             0000029f  mov      rax, qword ptr [rbp + 0x18]
fe_mul             000002a3  add      rax, 4
fe_mul             000002a7  mov      ecx, dword ptr [rax]
fe_mul             000002a9  movsxd   rcx, ecx
fe_mul             000002ac  mov      qword ptr [rbp - 0x10], rcx
fe_mul             000002b0  mov      rax, qword ptr [rbp + 0x18]
fe_mul             000002b4  add      rax, 8
fe_mul             000002b8  mov      ecx, dword ptr [rax]
fe_mul             000002ba  movsxd   rcx, ecx
fe_mul             000002bd  mov      qword ptr [rbp - 0x18], rcx
fe_mul             000002c1  mov      rax, qword ptr [rbp + 0x18]
fe_mul             000002c5  add      rax, 0xc
fe_mul             000002c9  mov      ecx, dword ptr [rax]
fe_mul             000002cb  movsxd   rcx, ecx
fe_mul             000002ce  mov      qword ptr [rbp - 0x20], rcx
fe_mul             000002d2  mov      rax, qword ptr [rbp + 0x18]
fe_mul             000002d6  add      rax, 0x10
fe_mul             000002da  mov      ecx, dword ptr [rax]
fe_mul             000002dc  movsxd   rcx, ecx
fe_mul             000002df  mov      qword ptr [rbp - 0x28], rcx
fe_mul             000002e3  mov      rax, qword ptr [rbp + 0x18]
fe_mul             000002e7  add      rax, 0x14
fe_mul             000002eb  mov      ecx, dword ptr [rax]
fe_mul             000002ed  movsxd   rcx, ecx
fe_mul             000002f0  mov      qword ptr [rbp - 0x30], rcx
fe_mul             000002f4  mov      rax, qword ptr [rbp + 0x18]
fe_mul             000002f8  add      rax, 0x18
fe_mul             000002fc  mov      ecx, dword ptr [rax]
fe_mul             000002fe  movsxd   rcx, ecx
fe_mul             00000301  mov      qword ptr [rbp - 0x38], rcx
fe_mul             00000305  mov      rax, qword ptr [rbp + 0x18]
fe_mul             00000309  add      rax, 0x1c
fe_mul             0000030d  mov      ecx, dword ptr [rax]
fe_mul             0000030f  movsxd   rcx, ecx
fe_mul             00000312  mov      qword ptr [rbp - 0x40], rcx
fe_mul             00000316  mov      rax, qword ptr [rbp + 0x18]
fe_mul             0000031a  add      rax, 0x20
fe_mul             0000031e  mov      ecx, dword ptr [rax]
fe_mul             00000320  movsxd   rcx, ecx
fe_mul             00000323  mov      qword ptr [rbp - 0x48], rcx
fe_mul             00000327  mov      rax, qword ptr [rbp + 0x18]
fe_mul             0000032b  add      rax, 0x24
fe_mul             0000032f  mov      ecx, dword ptr [rax]
fe_mul             00000331  movsxd   rcx, ecx
fe_mul             00000334  mov      qword ptr [rbp - 0x50], rcx
fe_mul             00000338  mov      rax, qword ptr [rbp + 0x20]
fe_mul             0000033c  mov      ecx, dword ptr [rax]
fe_mul             0000033e  movsxd   rcx, ecx
fe_mul             00000341  mov      qword ptr [rbp - 0x58], rcx
fe_mul             00000345  mov      rax, qword ptr [rbp + 0x20]
fe_mul             00000349  add      rax, 4
fe_mul             0000034d  mov      ecx, dword ptr [rax]
fe_mul             0000034f  movsxd   rcx, ecx
fe_mul             00000352  mov      qword ptr [rbp - 0x60], rcx
fe_mul             00000356  mov      rax, qword ptr [rbp + 0x20]
fe_mul             0000035a  add      rax, 8
fe_mul             0000035e  mov      ecx, dword ptr [rax]
fe_mul             00000360  movsxd   rcx, ecx
fe_mul             00000363  mov      qword ptr [rbp - 0x68], rcx
fe_mul             00000367  mov      rax, qword ptr [rbp + 0x20]
fe_mul             0000036b  add      rax, 0xc
fe_mul             0000036f  mov      ecx, dword ptr [rax]
fe_mul             00000371  movsxd   rcx, ecx
fe_mul             00000374  mov      qword ptr [rbp - 0x70], rcx
fe_mul             00000378  mov      rax, qword ptr [rbp + 0x20]
fe_mul             0000037c  add      rax, 0x10
fe_mul             00000380  mov      ecx, dword ptr [rax]
fe_mul             00000382  movsxd   rcx, ecx
fe_mul             00000385  mov      qword ptr [rbp - 0x78], rcx
fe_mul             00000389  mov      rax, qword ptr [rbp + 0x20]
fe_mul             0000038d  add      rax, 0x14
fe_mul             00000391  mov      ecx, dword ptr [rax]
fe_mul             00000393  movsxd   rcx, ecx
fe_mul             00000396  mov      qword ptr [rbp - 0x80], rcx
fe_mul             0000039a  mov      rax, qword ptr [rbp + 0x20]
fe_mul             0000039e  add      rax, 0x18
fe_mul             000003a2  mov      ecx, dword ptr [rax]
fe_mul             000003a4  movsxd   rcx, ecx
fe_mul             000003a7  mov      qword ptr [rbp - 0x88], rcx
fe_mul             000003ae  mov      rax, qword ptr [rbp + 0x20]
fe_mul             000003b2  add      rax, 0x1c
fe_mul             000003b6  mov      ecx, dword ptr [rax]
fe_mul             000003b8  movsxd   rcx, ecx
fe_mul             000003bb  mov      qword ptr [rbp - 0x90], rcx
fe_mul             000003c2  mov      rax, qword ptr [rbp + 0x20]
fe_mul             000003c6  add      rax, 0x20
fe_mul             000003ca  mov      ecx, dword ptr [rax]
fe_mul             000003cc  movsxd   rcx, ecx
fe_mul             000003cf  mov      qword ptr [rbp - 0x98], rcx
fe_mul             000003d6  mov      rax, qword ptr [rbp + 0x20]
fe_mul             000003da  add      rax, 0x24
fe_mul             000003de  mov      ecx, dword ptr [rax]
fe_mul             000003e0  movsxd   rcx, ecx
fe_mul             000003e3  mov      qword ptr [rbp - 0xa0], rcx
fe_mul             000003ea  mov      rax, qword ptr [rbp - 0x60]
fe_mul             000003ee  movabs   rcx, 0x13
fe_mul             000003f8  imul     rax, rcx
fe_mul             000003fc  mov      qword ptr [rbp - 0xa8], rax
fe_mul             00000403  mov      rax, qword ptr [rbp - 0x68]
fe_mul             00000407  movabs   rcx, 0x13
fe_mul             00000411  imul     rax, rcx
fe_mul             00000415  mov      qword ptr [rbp - 0xb0], rax
fe_mul             0000041c  mov      rax, qword ptr [rbp - 0x70]
fe_mul             00000420  movabs   rcx, 0x13
fe_mul             0000042a  imul     rax, rcx
fe_mul             0000042e  mov      qword ptr [rbp - 0xb8], rax
fe_mul             00000435  mov      rax, qword ptr [rbp - 0x78]
fe_mul             00000439  movabs   rcx, 0x13
fe_mul             00000443  imul     rax, rcx
fe_mul             00000447  mov      qword ptr [rbp - 0xc0], rax
fe_mul             0000044e  mov      rax, qword ptr [rbp - 0x80]
fe_mul             00000452  movabs   rcx, 0x13
fe_mul             0000045c  imul     rax, rcx
fe_mul             00000460  mov      qword ptr [rbp - 0xc8], rax
fe_mul             00000467  mov      rax, qword ptr [rbp - 0x88]
fe_mul             0000046e  movabs   rcx, 0x13
fe_mul             00000478  imul     rax, rcx
fe_mul             0000047c  mov      qword ptr [rbp - 0xd0], rax
fe_mul             00000483  mov      rax, qword ptr [rbp - 0x90]
fe_mul             0000048a  movabs   rcx, 0x13
fe_mul             00000494  imul     rax, rcx
fe_mul             00000498  mov      qword ptr [rbp - 0xd8], rax
fe_mul             0000049f  mov      rax, qword ptr [rbp - 0x98]
fe_mul             000004a6  movabs   rcx, 0x13
fe_mul             000004b0  imul     rax, rcx
fe_mul             000004b4  mov      qword ptr [rbp - 0xe0], rax
fe_mul             000004bb  mov      rax, qword ptr [rbp - 0xa0]
fe_mul             000004c2  movabs   rcx, 0x13
fe_mul             000004cc  imul     rax, rcx
fe_mul             000004d0  mov      qword ptr [rbp - 0xe8], rax
fe_mul             000004d7  mov      rax, qword ptr [rbp - 0x10]
fe_mul             000004db  shl      rax, 1
fe_mul             000004df  mov      qword ptr [rbp - 0xf0], rax
fe_mul             000004e6  mov      rax, qword ptr [rbp - 0x20]
fe_mul             000004ea  shl      rax, 1
fe_mul             000004ee  mov      qword ptr [rbp - 0xf8], rax
fe_mul             000004f5  mov      rax, qword ptr [rbp - 0x30]
fe_mul             000004f9  shl      rax, 1
fe_mul             000004fd  mov      qword ptr [rbp - 0x100], rax
fe_mul             00000504  mov      rax, qword ptr [rbp - 0x40]
fe_mul             00000508  shl      rax, 1
fe_mul             0000050c  mov      qword ptr [rbp - 0x108], rax
fe_mul             00000513  mov      rax, qword ptr [rbp - 0x50]
fe_mul             00000517  shl      rax, 1
fe_mul             0000051b  mov      qword ptr [rbp - 0x110], rax
fe_mul             00000522  mov      rax, qword ptr [rbp - 8]
fe_mul             00000526  mov      rcx, qword ptr [rbp - 0x58]
fe_mul             0000052a  imul     rax, rcx
fe_mul             0000052e  mov      rcx, qword ptr [rbp - 0xf0]
fe_mul             00000535  mov      rdx, qword ptr [rbp - 0xe8]
fe_mul             0000053c  imul     rcx, rdx
fe_mul             00000540  add      rax, rcx
fe_mul             00000543  mov      rcx, qword ptr [rbp - 0x18]
fe_mul             00000547  mov      rdx, qword ptr [rbp - 0xe0]
fe_mul             0000054e  imul     rcx, rdx
fe_mul             00000552  add      rax, rcx
fe_mul             00000555  mov      rcx, qword ptr [rbp - 0xf8]
fe_mul             0000055c  mov      rdx, qword ptr [rbp - 0xd8]
fe_mul             00000563  imul     rcx, rdx
fe_mul             00000567  add      rax, rcx
fe_mul             0000056a  mov      rcx, qword ptr [rbp - 0x28]
fe_mul             0000056e  mov      rdx, qword ptr [rbp - 0xd0]
fe_mul             00000575  imul     rcx, rdx
fe_mul             00000579  add      rax, rcx
fe_mul             0000057c  mov      rcx, qword ptr [rbp - 0x100]
fe_mul             00000583  mov      rdx, qword ptr [rbp - 0xc8]
fe_mul             0000058a  imul     rcx, rdx
fe_mul             0000058e  add      rax, rcx
fe_mul             00000591  mov      rcx, qword ptr [rbp - 0x38]
fe_mul             00000595  mov      rdx, qword ptr [rbp - 0xc0]
fe_mul             0000059c  imul     rcx, rdx
fe_mul             000005a0  add      rax, rcx
fe_mul             000005a3  mov      rcx, qword ptr [rbp - 0x108]
fe_mul             000005aa  mov      rdx, qword ptr [rbp - 0xb8]
fe_mul             000005b1  imul     rcx, rdx
fe_mul             000005b5  add      rax, rcx
fe_mul             000005b8  mov      rcx, qword ptr [rbp - 0x48]
fe_mul             000005bc  mov      rdx, qword ptr [rbp - 0xb0]
fe_mul             000005c3  imul     rcx, rdx
fe_mul             000005c7  add      rax, rcx
fe_mul             000005ca  mov      rcx, qword ptr [rbp - 0x110]
fe_mul             000005d1  mov      rdx, qword ptr [rbp - 0xa8]
fe_mul             000005d8  imul     rcx, rdx
fe_mul             000005dc  add      rax, rcx
fe_mul             000005df  mov      qword ptr [rbp - 0x118], rax
fe_mul             000005e6  mov      rax, qword ptr [rbp - 8]
fe_mul             000005ea  mov      rcx, qword ptr [rbp - 0x60]
fe_mul             000005ee  imul     rax, rcx
fe_mul             000005f2  mov      rcx, qword ptr [rbp - 0x10]
fe_mul             000005f6  mov      rdx, qword ptr [rbp - 0x58]
fe_mul             000005fa  imul     rcx, rdx
fe_mul             000005fe  add      rax, rcx
fe_mul             00000601  mov      rcx, qword ptr [rbp - 0x18]
fe_mul             00000605  mov      rdx, qword ptr [rbp - 0xe8]
fe_mul             0000060c  imul     rcx, rdx
fe_mul             00000610  add      rax, rcx
fe_mul             00000613  mov      rcx, qword ptr [rbp - 0x20]
fe_mul             00000617  mov      rdx, qword ptr [rbp - 0xe0]
fe_mul             0000061e  imul     rcx, rdx
fe_mul             00000622  add      rax, rcx
fe_mul             00000625  mov      rcx, qword ptr [rbp - 0x28]
fe_mul             00000629  mov      rdx, qword ptr [rbp - 0xd8]
fe_mul             00000630  imul     rcx, rdx
fe_mul             00000634  add      rax, rcx
fe_mul             00000637  mov      rcx, qword ptr [rbp - 0x30]
fe_mul             0000063b  mov      rdx, qword ptr [rbp - 0xd0]
fe_mul             00000642  imul     rcx, rdx
fe_mul             00000646  add      rax, rcx
fe_mul             00000649  mov      rcx, qword ptr [rbp - 0x38]
fe_mul             0000064d  mov      rdx, qword ptr [rbp - 0xc8]
fe_mul             00000654  imul     rcx, rdx
fe_mul             00000658  add      rax, rcx
fe_mul             0000065b  mov      rcx, qword ptr [rbp - 0x40]
fe_mul             0000065f  mov      rdx, qword ptr [rbp - 0xc0]
fe_mul             00000666  imul     rcx, rdx
fe_mul             0000066a  add      rax, rcx
fe_mul             0000066d  mov      rcx, qword ptr [rbp - 0x48]
fe_mul             00000671  mov      rdx, qword ptr [rbp - 0xb8]
fe_mul             00000678  imul     rcx, rdx
fe_mul             0000067c  add      rax, rcx
fe_mul             0000067f  mov      rcx, qword ptr [rbp - 0x50]
fe_mul             00000683  mov      rdx, qword ptr [rbp - 0xb0]
fe_mul             0000068a  imul     rcx, rdx
fe_mul             0000068e  add      rax, rcx
fe_mul             00000691  mov      qword ptr [rbp - 0x120], rax
fe_mul             00000698  mov      rax, qword ptr [rbp - 8]
fe_mul             0000069c  mov      rcx, qword ptr [rbp - 0x68]
fe_mul             000006a0  imul     rax, rcx
fe_mul             000006a4  mov      rcx, qword ptr [rbp - 0xf0]
fe_mul             000006ab  mov      rdx, qword ptr [rbp - 0x60]
fe_mul             000006af  imul     rcx, rdx
fe_mul             000006b3  add      rax, rcx
fe_mul             000006b6  mov      rcx, qword ptr [rbp - 0x18]
fe_mul             000006ba  mov      rdx, qword ptr [rbp - 0x58]
fe_mul             000006be  imul     rcx, rdx
fe_mul             000006c2  add      rax, rcx
fe_mul             000006c5  mov      rcx, qword ptr [rbp - 0xf8]
fe_mul             000006cc  mov      rdx, qword ptr [rbp - 0xe8]
fe_mul             000006d3  imul     rcx, rdx
fe_mul             000006d7  add      rax, rcx
fe_mul             000006da  mov      rcx, qword ptr [rbp - 0x28]
fe_mul             000006de  mov      rdx, qword ptr [rbp - 0xe0]
fe_mul             000006e5  imul     rcx, rdx
fe_mul             000006e9  add      rax, rcx
fe_mul             000006ec  mov      rcx, qword ptr [rbp - 0x100]
fe_mul             000006f3  mov      rdx, qword ptr [rbp - 0xd8]
fe_mul             000006fa  imul     rcx, rdx
fe_mul             000006fe  add      rax, rcx
fe_mul             00000701  mov      rcx, qword ptr [rbp - 0x38]
fe_mul             00000705  mov      rdx, qword ptr [rbp - 0xd0]
fe_mul             0000070c  imul     rcx, rdx
fe_mul             00000710  add      rax, rcx
fe_mul             00000713  mov      rcx, qword ptr [rbp - 0x108]
fe_mul             0000071a  mov      rdx, qword ptr [rbp - 0xc8]
fe_mul             00000721  imul     rcx, rdx
fe_mul             00000725  add      rax, rcx
fe_mul             00000728  mov      rcx, qword ptr [rbp - 0x48]
fe_mul             0000072c  mov      rdx, qword ptr [rbp - 0xc0]
fe_mul             00000733  imul     rcx, rdx
fe_mul             00000737  add      rax, rcx
fe_mul             0000073a  mov      rcx, qword ptr [rbp - 0x110]
fe_mul             00000741  mov      rdx, qword ptr [rbp - 0xb8]
fe_mul             00000748  imul     rcx, rdx
fe_mul             0000074c  add      rax, rcx
fe_mul             0000074f  mov      qword ptr [rbp - 0x128], rax
fe_mul             00000756  mov      rax, qword ptr [rbp - 8]
fe_mul             0000075a  mov      rcx, qword ptr [rbp - 0x70]
fe_mul             0000075e  imul     rax, rcx
fe_mul             00000762  mov      rcx, qword ptr [rbp - 0x10]
fe_mul             00000766  mov      rdx, qword ptr [rbp - 0x68]
fe_mul             0000076a  imul     rcx, rdx
fe_mul             0000076e  add      rax, rcx
fe_mul             00000771  mov      rcx, qword ptr [rbp - 0x18]
fe_mul             00000775  mov      rdx, qword ptr [rbp - 0x60]
fe_mul             00000779  imul     rcx, rdx
fe_mul             0000077d  add      rax, rcx
fe_mul             00000780  mov      rcx, qword ptr [rbp - 0x20]
fe_mul             00000784  mov      rdx, qword ptr [rbp - 0x58]
fe_mul             00000788  imul     rcx, rdx
fe_mul             0000078c  add      rax, rcx
fe_mul             0000078f  mov      rcx, qword ptr [rbp - 0x28]
fe_mul             00000793  mov      rdx, qword ptr [rbp - 0xe8]
fe_mul             0000079a  imul     rcx, rdx
fe_mul             0000079e  add      rax, rcx
fe_mul             000007a1  mov      rcx, qword ptr [rbp - 0x30]
fe_mul             000007a5  mov      rdx, qword ptr [rbp - 0xe0]
fe_mul             000007ac  imul     rcx, rdx
fe_mul             000007b0  add      rax, rcx
fe_mul             000007b3  mov      rcx, qword ptr [rbp - 0x38]
fe_mul             000007b7  mov      rdx, qword ptr [rbp - 0xd8]
fe_mul             000007be  imul     rcx, rdx
fe_mul             000007c2  add      rax, rcx
fe_mul             000007c5  mov      rcx, qword ptr [rbp - 0x40]
fe_mul             000007c9  mov      rdx, qword ptr [rbp - 0xd0]
fe_mul             000007d0  imul     rcx, rdx
fe_mul             000007d4  add      rax, rcx
fe_mul             000007d7  mov      rcx, qword ptr [rbp - 0x48]
fe_mul             000007db  mov      rdx, qword ptr [rbp - 0xc8]
fe_mul             000007e2  imul     rcx, rdx
fe_mul             000007e6  add      rax, rcx
fe_mul             000007e9  mov      rcx, qword ptr [rbp - 0x50]
fe_mul             000007ed  mov      rdx, qword ptr [rbp - 0xc0]
fe_mul             000007f4  imul     rcx, rdx
fe_mul             000007f8  add      rax, rcx
fe_mul             000007fb  mov      qword ptr [rbp - 0x130], rax
fe_mul             00000802  mov      rax, qword ptr [rbp - 8]
fe_mul             00000806  mov      rcx, qword ptr [rbp - 0x78]
fe_mul             0000080a  imul     rax, rcx
fe_mul             0000080e  mov      rcx, qword ptr [rbp - 0xf0]
fe_mul             00000815  mov      rdx, qword ptr [rbp - 0x70]
fe_mul             00000819  imul     rcx, rdx
fe_mul             0000081d  add      rax, rcx
fe_mul             00000820  mov      rcx, qword ptr [rbp - 0x18]
fe_mul             00000824  mov      rdx, qword ptr [rbp - 0x68]
fe_mul             00000828  imul     rcx, rdx
fe_mul             0000082c  add      rax, rcx
fe_mul             0000082f  mov      rcx, qword ptr [rbp - 0xf8]
fe_mul             00000836  mov      rdx, qword ptr [rbp - 0x60]
fe_mul             0000083a  imul     rcx, rdx
fe_mul             0000083e  add      rax, rcx
fe_mul             00000841  mov      rcx, qword ptr [rbp - 0x28]
fe_mul             00000845  mov      rdx, qword ptr [rbp - 0x58]
fe_mul             00000849  imul     rcx, rdx
fe_mul             0000084d  add      rax, rcx
fe_mul             00000850  mov      rcx, qword ptr [rbp - 0x100]
fe_mul             00000857  mov      rdx, qword ptr [rbp - 0xe8]
fe_mul             0000085e  imul     rcx, rdx
fe_mul             00000862  add      rax, rcx
fe_mul             00000865  mov      rcx, qword ptr [rbp - 0x38]
fe_mul             00000869  mov      rdx, qword ptr [rbp - 0xe0]
fe_mul             00000870  imul     rcx, rdx
fe_mul             00000874  add      rax, rcx
fe_mul             00000877  mov      rcx, qword ptr [rbp - 0x108]
fe_mul             0000087e  mov      rdx, qword ptr [rbp - 0xd8]
fe_mul             00000885  imul     rcx, rdx
fe_mul             00000889  add      rax, rcx
fe_mul             0000088c  mov      rcx, qword ptr [rbp - 0x48]
fe_mul             00000890  mov      rdx, qword ptr [rbp - 0xd0]
fe_mul             00000897  imul     rcx, rdx
fe_mul             0000089b  add      rax, rcx
fe_mul             0000089e  mov      rcx, qword ptr [rbp - 0x110]
fe_mul             000008a5  mov      rdx, qword ptr [rbp - 0xc8]
fe_mul             000008ac  imul     rcx, rdx
fe_mul             000008b0  add      rax, rcx
fe_mul             000008b3  mov      qword ptr [rbp - 0x138], rax
fe_mul             000008ba  mov      rax, qword ptr [rbp - 8]
fe_mul             000008be  mov      rcx, qword ptr [rbp - 0x80]
fe_mul             000008c2  imul     rax, rcx
fe_mul             000008c6  mov      rcx, qword ptr [rbp - 0x10]
fe_mul             000008ca  mov      rdx, qword ptr [rbp - 0x78]
fe_mul             000008ce  imul     rcx, rdx
fe_mul             000008d2  add      rax, rcx
fe_mul             000008d5  mov      rcx, qword ptr [rbp - 0x18]
fe_mul             000008d9  mov      rdx, qword ptr [rbp - 0x70]
fe_mul             000008dd  imul     rcx, rdx
fe_mul             000008e1  add      rax, rcx
fe_mul             000008e4  mov      rcx, qword ptr [rbp - 0x20]
fe_mul             000008e8  mov      rdx, qword ptr [rbp - 0x68]
fe_mul             000008ec  imul     rcx, rdx
fe_mul             000008f0  add      rax, rcx
fe_mul             000008f3  mov      rcx, qword ptr [rbp - 0x28]
fe_mul             000008f7  mov      rdx, qword ptr [rbp - 0x60]
fe_mul             000008fb  imul     rcx, rdx
fe_mul             000008ff  add      rax, rcx
fe_mul             00000902  mov      rcx, qword ptr [rbp - 0x30]
fe_mul             00000906  mov      rdx, qword ptr [rbp - 0x58]
fe_mul             0000090a  imul     rcx, rdx
fe_mul             0000090e  add      rax, rcx
fe_mul             00000911  mov      rcx, qword ptr [rbp - 0x38]
fe_mul             00000915  mov      rdx, qword ptr [rbp - 0xe8]
fe_mul             0000091c  imul     rcx, rdx
fe_mul             00000920  add      rax, rcx
fe_mul             00000923  mov      rcx, qword ptr [rbp - 0x40]
fe_mul             00000927  mov      rdx, qword ptr [rbp - 0xe0]
fe_mul             0000092e  imul     rcx, rdx
fe_mul             00000932  add      rax, rcx
fe_mul             00000935  mov      rcx, qword ptr [rbp - 0x48]
fe_mul             00000939  mov      rdx, qword ptr [rbp - 0xd8]
fe_mul             00000940  imul     rcx, rdx
fe_mul             00000944  add      rax, rcx
fe_mul             00000947  mov      rcx, qword ptr [rbp - 0x50]
fe_mul             0000094b  mov      rdx, qword ptr [rbp - 0xd0]
fe_mul             00000952  imul     rcx, rdx
fe_mul             00000956  add      rax, rcx
fe_mul             00000959  mov      qword ptr [rbp - 0x140], rax
fe_mul             00000960  mov      rax, qword ptr [rbp - 8]
fe_mul             00000964  mov      rcx, qword ptr [rbp - 0x88]
fe_mul             0000096b  imul     rax, rcx
fe_mul             0000096f  mov      rcx, qword ptr [rbp - 0xf0]
fe_mul             00000976  mov      rdx, qword ptr [rbp - 0x80]
fe_mul             0000097a  imul     rcx, rdx
fe_mul             0000097e  add      rax, rcx
fe_mul             00000981  mov      rcx, qword ptr [rbp - 0x18]
fe_mul             00000985  mov      rdx, qword ptr [rbp - 0x78]
fe_mul             00000989  imul     rcx, rdx
fe_mul             0000098d  add      rax, rcx
fe_mul             00000990  mov      rcx, qword ptr [rbp - 0xf8]
fe_mul             00000997  mov      rdx, qword ptr [rbp - 0x70]
fe_mul             0000099b  imul     rcx, rdx
fe_mul             0000099f  add      rax, rcx
fe_mul             000009a2  mov      rcx, qword ptr [rbp - 0x28]
fe_mul             000009a6  mov      rdx, qword ptr [rbp - 0x68]
fe_mul             000009aa  imul     rcx, rdx
fe_mul             000009ae  add      rax, rcx
fe_mul             000009b1  mov      rcx, qword ptr [rbp - 0x100]
fe_mul             000009b8  mov      rdx, qword ptr [rbp - 0x60]
fe_mul             000009bc  imul     rcx, rdx
fe_mul             000009c0  add      rax, rcx
fe_mul             000009c3  mov      rcx, qword ptr [rbp - 0x38]
fe_mul             000009c7  mov      rdx, qword ptr [rbp - 0x58]
fe_mul             000009cb  imul     rcx, rdx
fe_mul             000009cf  add      rax, rcx
fe_mul             000009d2  mov      rcx, qword ptr [rbp - 0x108]
fe_mul             000009d9  mov      rdx, qword ptr [rbp - 0xe8]
fe_mul             000009e0  imul     rcx, rdx
fe_mul             000009e4  add      rax, rcx
fe_mul             000009e7  mov      rcx, qword ptr [rbp - 0x48]
fe_mul             000009eb  mov      rdx, qword ptr [rbp - 0xe0]
fe_mul             000009f2  imul     rcx, rdx
fe_mul             000009f6  add      rax, rcx
fe_mul             000009f9  mov      rcx, qword ptr [rbp - 0x110]
fe_mul             00000a00  mov      rdx, qword ptr [rbp - 0xd8]
fe_mul             00000a07  imul     rcx, rdx
fe_mul             00000a0b  add      rax, rcx
fe_mul             00000a0e  mov      qword ptr [rbp - 0x148], rax
fe_mul             00000a15  mov      rax, qword ptr [rbp - 8]
fe_mul             00000a19  mov      rcx, qword ptr [rbp - 0x90]
fe_mul             00000a20  imul     rax, rcx
fe_mul             00000a24  mov      rcx, qword ptr [rbp - 0x10]
fe_mul             00000a28  mov      rdx, qword ptr [rbp - 0x88]
fe_mul             00000a2f  imul     rcx, rdx
fe_mul             00000a33  add      rax, rcx
fe_mul             00000a36  mov      rcx, qword ptr [rbp - 0x18]
fe_mul             00000a3a  mov      rdx, qword ptr [rbp - 0x80]
fe_mul             00000a3e  imul     rcx, rdx
fe_mul             00000a42  add      rax, rcx
fe_mul             00000a45  mov      rcx, qword ptr [rbp - 0x20]
fe_mul             00000a49  mov      rdx, qword ptr [rbp - 0x78]
fe_mul             00000a4d  imul     rcx, rdx
fe_mul             00000a51  add      rax, rcx
fe_mul             00000a54  mov      rcx, qword ptr [rbp - 0x28]
fe_mul             00000a58  mov      rdx, qword ptr [rbp - 0x70]
fe_mul             00000a5c  imul     rcx, rdx
fe_mul             00000a60  add      rax, rcx
fe_mul             00000a63  mov      rcx, qword ptr [rbp - 0x30]
fe_mul             00000a67  mov      rdx, qword ptr [rbp - 0x68]
fe_mul             00000a6b  imul     rcx, rdx
fe_mul             00000a6f  add      rax, rcx
fe_mul             00000a72  mov      rcx, qword ptr [rbp - 0x38]
fe_mul             00000a76  mov      rdx, qword ptr [rbp - 0x60]
fe_mul             00000a7a  imul     rcx, rdx
fe_mul             00000a7e  add      rax, rcx
fe_mul             00000a81  mov      rcx, qword ptr [rbp - 0x40]
fe_mul             00000a85  mov      rdx, qword ptr [rbp - 0x58]
fe_mul             00000a89  imul     rcx, rdx
fe_mul             00000a8d  add      rax, rcx
fe_mul             00000a90  mov      rcx, qword ptr [rbp - 0x48]
fe_mul             00000a94  mov      rdx, qword ptr [rbp - 0xe8]
fe_mul             00000a9b  imul     rcx, rdx
fe_mul             00000a9f  add      rax, rcx
fe_mul             00000aa2  mov      rcx, qword ptr [rbp - 0x50]
fe_mul             00000aa6  mov      rdx, qword ptr [rbp - 0xe0]
fe_mul             00000aad  imul     rcx, rdx
fe_mul             00000ab1  add      rax, rcx
fe_mul             00000ab4  mov      qword ptr [rbp - 0x150], rax
fe_mul             00000abb  mov      rax, qword ptr [rbp - 8]
fe_mul             00000abf  mov      rcx, qword ptr [rbp - 0x98]
fe_mul             00000ac6  imul     rax, rcx
fe_mul             00000aca  mov      rcx, qword ptr [rbp - 0xf0]
fe_mul             00000ad1  mov      rdx, qword ptr [rbp - 0x90]
fe_mul             00000ad8  imul     rcx, rdx
fe_mul             00000adc  add      rax, rcx
fe_mul             00000adf  mov      rcx, qword ptr [rbp - 0x18]
fe_mul             00000ae3  mov      rdx, qword ptr [rbp - 0x88]
fe_mul             00000aea  imul     rcx, rdx
fe_mul             00000aee  add      rax, rcx
fe_mul             00000af1  mov      rcx, qword ptr [rbp - 0xf8]
fe_mul             00000af8  mov      rdx, qword ptr [rbp - 0x80]
fe_mul             00000afc  imul     rcx, rdx
fe_mul             00000b00  add      rax, rcx
fe_mul             00000b03  mov      rcx, qword ptr [rbp - 0x28]
fe_mul             00000b07  mov      rdx, qword ptr [rbp - 0x78]
fe_mul             00000b0b  imul     rcx, rdx
fe_mul             00000b0f  add      rax, rcx
fe_mul             00000b12  mov      rcx, qword ptr [rbp - 0x100]
fe_mul             00000b19  mov      rdx, qword ptr [rbp - 0x70]
fe_mul             00000b1d  imul     rcx, rdx
fe_mul             00000b21  add      rax, rcx
fe_mul             00000b24  mov      rcx, qword ptr [rbp - 0x38]
fe_mul             00000b28  mov      rdx, qword ptr [rbp - 0x68]
fe_mul             00000b2c  imul     rcx, rdx
fe_mul             00000b30  add      rax, rcx
fe_mul             00000b33  mov      rcx, qword ptr [rbp - 0x108]
fe_mul             00000b3a  mov      rdx, qword ptr [rbp - 0x60]
fe_mul             00000b3e  imul     rcx, rdx
fe_mul             00000b42  add      rax, rcx
fe_mul             00000b45  mov      rcx, qword ptr [rbp - 0x48]
fe_mul             00000b49  mov      rdx, qword ptr [rbp - 0x58]
fe_mul             00000b4d  imul     rcx, rdx
fe_mul             00000b51  add      rax, rcx
fe_mul             00000b54  mov      rcx, qword ptr [rbp - 0x110]
fe_mul             00000b5b  mov      rdx, qword ptr [rbp - 0xe8]
fe_mul             00000b62  imul     rcx, rdx
fe_mul             00000b66  add      rax, rcx
fe_mul             00000b69  mov      qword ptr [rbp - 0x158], rax
fe_mul             00000b70  mov      rax, qword ptr [rbp - 8]
fe_mul             00000b74  mov      rcx, qword ptr [rbp - 0xa0]
fe_mul             00000b7b  imul     rax, rcx
fe_mul             00000b7f  mov      rcx, qword ptr [rbp - 0x10]
fe_mul             00000b83  mov      rdx, qword ptr [rbp - 0x98]
fe_mul             00000b8a  imul     rcx, rdx
fe_mul             00000b8e  add      rax, rcx
fe_mul             00000b91  mov      rcx, qword ptr [rbp - 0x18]
fe_mul             00000b95  mov      rdx, qword ptr [rbp - 0x90]
fe_mul             00000b9c  imul     rcx, rdx
fe_mul             00000ba0  add      rax, rcx
fe_mul             00000ba3  mov      rcx, qword ptr [rbp - 0x20]
fe_mul             00000ba7  mov      rdx, qword ptr [rbp - 0x88]
fe_mul             00000bae  imul     rcx, rdx
fe_mul             00000bb2  add      rax, rcx
fe_mul             00000bb5  mov      rcx, qword ptr [rbp - 0x28]
fe_mul             00000bb9  mov      rdx, qword ptr [rbp - 0x80]
fe_mul             00000bbd  imul     rcx, rdx
fe_mul             00000bc1  add      rax, rcx
fe_mul             00000bc4  mov      rcx, qword ptr [rbp - 0x30]
fe_mul             00000bc8  mov      rdx, qword ptr [rbp - 0x78]
fe_mul             00000bcc  imul     rcx, rdx
fe_mul             00000bd0  add      rax, rcx
fe_mul             00000bd3  mov      rcx, qword ptr [rbp - 0x38]
fe_mul             00000bd7  mov      rdx, qword ptr [rbp - 0x70]
fe_mul             00000bdb  imul     rcx, rdx
fe_mul             00000bdf  add      rax, rcx
fe_mul             00000be2  mov      rcx, qword ptr [rbp - 0x40]
fe_mul             00000be6  mov      rdx, qword ptr [rbp - 0x68]
fe_mul             00000bea  imul     rcx, rdx
fe_mul             00000bee  add      rax, rcx
fe_mul             00000bf1  mov      rcx, qword ptr [rbp - 0x48]
fe_mul             00000bf5  mov      rdx, qword ptr [rbp - 0x60]
fe_mul             00000bf9  imul     rcx, rdx
fe_mul             00000bfd  add      rax, rcx
fe_mul             00000c00  mov      rcx, qword ptr [rbp - 0x50]
fe_mul             00000c04  mov      rdx, qword ptr [rbp - 0x58]
fe_mul             00000c08  imul     rcx, rdx
fe_mul             00000c0c  add      rax, rcx
fe_mul             00000c0f  mov      qword ptr [rbp - 0x160], rax
fe_mul             00000c16  mov      rax, qword ptr [rbp - 0x118]
fe_mul             00000c1d  add      rax, 0x2000000
fe_mul             00000c24  sar      rax, 0x1a
fe_mul             00000c28  mov      qword ptr [rbp - 0x168], rax
fe_mul             00000c2f  mov      rax, qword ptr [rbp - 0x120]
fe_mul             00000c36  mov      rcx, qword ptr [rbp - 0x168]
fe_mul             00000c3d  add      rax, rcx
fe_mul             00000c40  mov      qword ptr [rbp - 0x120], rax
fe_mul             00000c47  mov      rax, qword ptr [rbp - 0x168]
fe_mul             00000c4e  shl      rax, 0x1a
fe_mul             00000c52  mov      rcx, qword ptr [rbp - 0x118]
fe_mul             00000c59  sub      rcx, rax
fe_mul             00000c5c  mov      qword ptr [rbp - 0x118], rcx
fe_mul             00000c63  mov      rax, qword ptr [rbp - 0x138]
fe_mul             00000c6a  add      rax, 0x2000000
fe_mul             00000c71  sar      rax, 0x1a
fe_mul             00000c75  mov      qword ptr [rbp - 0x188], rax
fe_mul             00000c7c  mov      rax, qword ptr [rbp - 0x140]
fe_mul             00000c83  mov      rcx, qword ptr [rbp - 0x188]
fe_mul             00000c8a  add      rax, rcx
fe_mul             00000c8d  mov      qword ptr [rbp - 0x140], rax
fe_mul             00000c94  mov      rax, qword ptr [rbp - 0x188]
fe_mul             00000c9b  shl      rax, 0x1a
fe_mul             00000c9f  mov      rcx, qword ptr [rbp - 0x138]
fe_mul             00000ca6  sub      rcx, rax
fe_mul             00000ca9  mov      qword ptr [rbp - 0x138], rcx
fe_mul             00000cb0  mov      rax, qword ptr [rbp - 0x120]
fe_mul             00000cb7  add      rax, 0x1000000
fe_mul             00000cbe  sar      rax, 0x19
fe_mul             00000cc2  mov      qword ptr [rbp - 0x170], rax
fe_mul             00000cc9  mov      rax, qword ptr [rbp - 0x128]
fe_mul             00000cd0  mov      rcx, qword ptr [rbp - 0x170]
fe_mul             00000cd7  add      rax, rcx
fe_mul             00000cda  mov      qword ptr [rbp - 0x128], rax
fe_mul             00000ce1  mov      rax, qword ptr [rbp - 0x170]
fe_mul             00000ce8  shl      rax, 0x19
fe_mul             00000cec  mov      rcx, qword ptr [rbp - 0x120]
fe_mul             00000cf3  sub      rcx, rax
fe_mul             00000cf6  mov      qword ptr [rbp - 0x120], rcx
fe_mul             00000cfd  mov      rax, qword ptr [rbp - 0x140]
fe_mul             00000d04  add      rax, 0x1000000
fe_mul             00000d0b  sar      rax, 0x19
fe_mul             00000d0f  mov      qword ptr [rbp - 0x190], rax
fe_mul             00000d16  mov      rax, qword ptr [rbp - 0x148]
fe_mul             00000d1d  mov      rcx, qword ptr [rbp - 0x190]
fe_mul             00000d24  add      rax, rcx
fe_mul             00000d27  mov      qword ptr [rbp - 0x148], rax
fe_mul             00000d2e  mov      rax, qword ptr [rbp - 0x190]
fe_mul             00000d35  shl      rax, 0x19
fe_mul             00000d39  mov      rcx, qword ptr [rbp - 0x140]
fe_mul             00000d40  sub      rcx, rax
fe_mul             00000d43  mov      qword ptr [rbp - 0x140], rcx
fe_mul             00000d4a  mov      rax, qword ptr [rbp - 0x128]
fe_mul             00000d51  add      rax, 0x2000000
fe_mul             00000d58  sar      rax, 0x1a
fe_mul             00000d5c  mov      qword ptr [rbp - 0x178], rax
fe_mul             00000d63  mov      rax, qword ptr [rbp - 0x130]
fe_mul             00000d6a  mov      rcx, qword ptr [rbp - 0x178]
fe_mul             00000d71  add      rax, rcx
fe_mul             00000d74  mov      qword ptr [rbp - 0x130], rax
fe_mul             00000d7b  mov      rax, qword ptr [rbp - 0x178]
fe_mul             00000d82  shl      rax, 0x1a
fe_mul             00000d86  mov      rcx, qword ptr [rbp - 0x128]
fe_mul             00000d8d  sub      rcx, rax
fe_mul             00000d90  mov      qword ptr [rbp - 0x128], rcx
fe_mul             00000d97  mov      rax, qword ptr [rbp - 0x148]
fe_mul             00000d9e  add      rax, 0x2000000
fe_mul             00000da5  sar      rax, 0x1a
fe_mul             00000da9  mov      qword ptr [rbp - 0x198], rax
fe_mul             00000db0  mov      rax, qword ptr [rbp - 0x150]
fe_mul             00000db7  mov      rcx, qword ptr [rbp - 0x198]
fe_mul             00000dbe  add      rax, rcx
fe_mul             00000dc1  mov      qword ptr [rbp - 0x150], rax
fe_mul             00000dc8  mov      rax, qword ptr [rbp - 0x198]
fe_mul             00000dcf  shl      rax, 0x1a
fe_mul             00000dd3  mov      rcx, qword ptr [rbp - 0x148]
fe_mul             00000dda  sub      rcx, rax
fe_mul             00000ddd  mov      qword ptr [rbp - 0x148], rcx
fe_mul             00000de4  mov      rax, qword ptr [rbp - 0x130]
fe_mul             00000deb  add      rax, 0x1000000
fe_mul             00000df2  sar      rax, 0x19
fe_mul             00000df6  mov      qword ptr [rbp - 0x180], rax
fe_mul             00000dfd  mov      rax, qword ptr [rbp - 0x138]
fe_mul             00000e04  mov      rcx, qword ptr [rbp - 0x180]
fe_mul             00000e0b  add      rax, rcx
fe_mul             00000e0e  mov      qword ptr [rbp - 0x138], rax
fe_mul             00000e15  mov      rax, qword ptr [rbp - 0x180]
fe_mul             00000e1c  shl      rax, 0x19
fe_mul             00000e20  mov      rcx, qword ptr [rbp - 0x130]
fe_mul             00000e27  sub      rcx, rax
fe_mul             00000e2a  mov      qword ptr [rbp - 0x130], rcx
fe_mul             00000e31  mov      rax, qword ptr [rbp - 0x150]
fe_mul             00000e38  add      rax, 0x1000000
fe_mul             00000e3f  sar      rax, 0x19
fe_mul             00000e43  mov      qword ptr [rbp - 0x1a0], rax
fe_mul             00000e4a  mov      rax, qword ptr [rbp - 0x158]
fe_mul             00000e51  mov      rcx, qword ptr [rbp - 0x1a0]
fe_mul             00000e58  add      rax, rcx
fe_mul             00000e5b  mov      qword ptr [rbp - 0x158], rax
fe_mul             00000e62  mov      rax, qword ptr [rbp - 0x1a0]
fe_mul             00000e69  shl      rax, 0x19
fe_mul             00000e6d  mov      rcx, qword ptr [rbp - 0x150]
fe_mul             00000e74  sub      rcx, rax
fe_mul             00000e77  mov      qword ptr [rbp - 0x150], rcx
fe_mul             00000e7e  mov      rax, qword ptr [rbp - 0x138]
fe_mul             00000e85  add      rax, 0x2000000
fe_mul             00000e8c  sar      rax, 0x1a
fe_mul             00000e90  mov      qword ptr [rbp - 0x188], rax
fe_mul             00000e97  mov      rax, qword ptr [rbp - 0x140]
fe_mul             00000e9e  mov      rcx, qword ptr [rbp - 0x188]
fe_mul             00000ea5  add      rax, rcx
fe_mul             00000ea8  mov      qword ptr [rbp - 0x140], rax
fe_mul             00000eaf  mov      rax, qword ptr [rbp - 0x188]
fe_mul             00000eb6  shl      rax, 0x1a
fe_mul             00000eba  mov      rcx, qword ptr [rbp - 0x138]
fe_mul             00000ec1  sub      rcx, rax
fe_mul             00000ec4  mov      qword ptr [rbp - 0x138], rcx
fe_mul             00000ecb  mov      rax, qword ptr [rbp - 0x158]
fe_mul             00000ed2  add      rax, 0x2000000
fe_mul             00000ed9  sar      rax, 0x1a
fe_mul             00000edd  mov      qword ptr [rbp - 0x1a8], rax
fe_mul             00000ee4  mov      rax, qword ptr [rbp - 0x160]
fe_mul             00000eeb  mov      rcx, qword ptr [rbp - 0x1a8]
fe_mul             00000ef2  add      rax, rcx
fe_mul             00000ef5  mov      qword ptr [rbp - 0x160], rax
fe_mul             00000efc  mov      rax, qword ptr [rbp - 0x1a8]
fe_mul             00000f03  shl      rax, 0x1a
fe_mul             00000f07  mov      rcx, qword ptr [rbp - 0x158]
fe_mul             00000f0e  sub      rcx, rax
fe_mul             00000f11  mov      qword ptr [rbp - 0x158], rcx
fe_mul             00000f18  mov      rax, qword ptr [rbp - 0x160]
fe_mul             00000f1f  add      rax, 0x1000000
fe_mul             00000f26  sar      rax, 0x19
fe_mul             00000f2a  mov      qword ptr [rbp - 0x1b0], rax
fe_mul             00000f31  mov      rax, qword ptr [rbp - 0x1b0]
fe_mul             00000f38  movabs   rcx, 0x13
fe_mul             00000f42  imul     rax, rcx
fe_mul             00000f46  mov      rcx, qword ptr [rbp - 0x118]
fe_mul             00000f4d  add      rcx, rax
fe_mul             00000f50  mov      qword ptr [rbp - 0x118], rcx
fe_mul             00000f57  mov      rax, qword ptr [rbp - 0x1b0]
fe_mul             00000f5e  shl      rax, 0x19
fe_mul             00000f62  mov      rcx, qword ptr [rbp - 0x160]
fe_mul             00000f69  sub      rcx, rax
fe_mul             00000f6c  mov      qword ptr [rbp - 0x160], rcx
fe_mul             00000f73  mov      rax, qword ptr [rbp - 0x118]
fe_mul             00000f7a  add      rax, 0x2000000
fe_mul             00000f81  sar      rax, 0x1a
fe_mul             00000f85  mov      qword ptr [rbp - 0x168], rax
fe_mul             00000f8c  mov      rax, qword ptr [rbp - 0x120]
fe_mul             00000f93  mov      rcx, qword ptr [rbp - 0x168]
fe_mul             00000f9a  add      rax, rcx
fe_mul             00000f9d  mov      qword ptr [rbp - 0x120], rax
fe_mul             00000fa4  mov      rax, qword ptr [rbp - 0x168]
fe_mul             00000fab  shl      rax, 0x1a
fe_mul             00000faf  mov      rcx, qword ptr [rbp - 0x118]
fe_mul             00000fb6  sub      rcx, rax
fe_mul             00000fb9  mov      qword ptr [rbp - 0x118], rcx
fe_mul             00000fc0  mov      rax, qword ptr [rbp + 0x10]
fe_mul             00000fc4  mov      ecx, dword ptr [rbp - 0x118]
fe_mul             00000fca  mov      dword ptr [rax], ecx
fe_mul             00000fcc  mov      rax, qword ptr [rbp + 0x10]
fe_mul             00000fd0  add      rax, 4
fe_mul             00000fd4  mov      ecx, dword ptr [rbp - 0x120]
fe_mul             00000fda  mov      dword ptr [rax], ecx
fe_mul             00000fdc  mov      rax, qword ptr [rbp + 0x10]
fe_mul             00000fe0  add      rax, 8
fe_mul             00000fe4  mov      ecx, dword ptr [rbp - 0x128]
fe_mul             00000fea  mov      dword ptr [rax], ecx
fe_mul             00000fec  mov      rax, qword ptr [rbp + 0x10]
fe_mul             00000ff0  add      rax, 0xc
fe_mul             00000ff4  mov      ecx, dword ptr [rbp - 0x130]
fe_mul             00000ffa  mov      dword ptr [rax], ecx
fe_mul             00000ffc  mov      rax, qword ptr [rbp + 0x10]
fe_mul             00001000  add      rax, 0x10
fe_mul             00001004  mov      ecx, dword ptr [rbp - 0x138]
fe_mul             0000100a  mov      dword ptr [rax], ecx
fe_mul             0000100c  mov      rax, qword ptr [rbp + 0x10]
fe_mul             00001010  add      rax, 0x14
fe_mul             00001014  mov      ecx, dword ptr [rbp - 0x140]
fe_mul             0000101a  mov      dword ptr [rax], ecx
fe_mul             0000101c  mov      rax, qword ptr [rbp + 0x10]
fe_mul             00001020  add      rax, 0x18
fe_mul             00001024  mov      ecx, dword ptr [rbp - 0x148]
fe_mul             0000102a  mov      dword ptr [rax], ecx
fe_mul             0000102c  mov      rax, qword ptr [rbp + 0x10]
fe_mul             00001030  add      rax, 0x1c
fe_mul             00001034  mov      ecx, dword ptr [rbp - 0x150]
fe_mul             0000103a  mov      dword ptr [rax], ecx
fe_mul             0000103c  mov      rax, qword ptr [rbp + 0x10]
fe_mul             00001040  add      rax, 0x20
fe_mul             00001044  mov      ecx, dword ptr [rbp - 0x158]
fe_mul             0000104a  mov      dword ptr [rax], ecx
fe_mul             0000104c  mov      rax, qword ptr [rbp + 0x10]
fe_mul             00001050  add      rax, 0x24
fe_mul             00001054  mov      ecx, dword ptr [rbp - 0x160]
fe_mul             0000105a  mov      dword ptr [rax], ecx
fe_mul             0000105c  leave    
fe_mul             0000105d  ret      
fe_sq              0000105e  push     rbp
fe_sq              0000105f  mov      rbp, rsp
fe_sq              00001062  sub      rsp, 0x20
fe_sq              00001069  mov      qword ptr [rbp + 0x10], rcx
fe_sq              0000106d  mov      qword ptr [rbp + 0x18], rdx
fe_sq              00001071  mov      rax, qword ptr [rbp + 0x18]
fe_sq              00001075  mov      r8, rax
fe_sq              00001078  mov      rax, qword ptr [rbp + 0x18]
fe_sq              0000107c  mov      r11, rax
fe_sq              0000107f  mov      rax, qword ptr [rbp + 0x10]
fe_sq              00001083  mov      r10, rax
fe_sq              00001086  mov      rcx, r10
fe_sq              00001089  mov      rdx, r11
fe_sq              0000108c  call     0x1091
fe_sq              00001091  leave    
fe_sq              00001092  ret      
fe_mul121666       00001093  push     rbp
fe_mul121666       00001094  mov      rbp, rsp
fe_mul121666       00001097  sub      rsp, 0xa0
fe_mul121666       0000109e  mov      qword ptr [rbp + 0x10], rcx
fe_mul121666       000010a2  mov      qword ptr [rbp + 0x18], rdx
fe_mul121666       000010a6  mov      rax, qword ptr [rbp + 0x18]
fe_mul121666       000010aa  mov      ecx, dword ptr [rax]
fe_mul121666       000010ac  movsxd   rcx, ecx
fe_mul121666       000010af  movabs   rax, 0x1db42
fe_mul121666       000010b9  imul     rcx, rax
fe_mul121666       000010bd  mov      qword ptr [rbp - 8], rcx
fe_mul121666       000010c1  mov      rax, qword ptr [rbp + 0x18]
fe_mul121666       000010c5  add      rax, 4
fe_mul121666       000010c9  mov      ecx, dword ptr [rax]
fe_mul121666       000010cb  movsxd   rcx, ecx
fe_mul121666       000010ce  movabs   rax, 0x1db42
fe_mul121666       000010d8  imul     rcx, rax
fe_mul121666       000010dc  mov      qword ptr [rbp - 0x10], rcx
fe_mul121666       000010e0  mov      rax, qword ptr [rbp + 0x18]
fe_mul121666       000010e4  add      rax, 8
fe_mul121666       000010e8  mov      ecx, dword ptr [rax]
fe_mul121666       000010ea  movsxd   rcx, ecx
fe_mul121666       000010ed  movabs   rax, 0x1db42
fe_mul121666       000010f7  imul     rcx, rax
fe_mul121666       000010fb  mov      qword ptr [rbp - 0x18], rcx
fe_mul121666       000010ff  mov      rax, qword ptr [rbp + 0x18]
fe_mul121666       00001103  add      rax, 0xc
fe_mul121666       00001107  mov      ecx, dword ptr [rax]
fe_mul121666       00001109  movsxd   rcx, ecx
fe_mul121666       0000110c  movabs   rax, 0x1db42
fe_mul121666       00001116  imul     rcx, rax
fe_mul121666       0000111a  mov      qword ptr [rbp - 0x20], rcx
fe_mul121666       0000111e  mov      rax, qword ptr [rbp + 0x18]
fe_mul121666       00001122  add      rax, 0x10
fe_mul121666       00001126  mov      ecx, dword ptr [rax]
fe_mul121666       00001128  movsxd   rcx, ecx
fe_mul121666       0000112b  movabs   rax, 0x1db42
fe_mul121666       00001135  imul     rcx, rax
fe_mul121666       00001139  mov      qword ptr [rbp - 0x28], rcx
fe_mul121666       0000113d  mov      rax, qword ptr [rbp + 0x18]
fe_mul121666       00001141  add      rax, 0x14
fe_mul121666       00001145  mov      ecx, dword ptr [rax]
fe_mul121666       00001147  movsxd   rcx, ecx
fe_mul121666       0000114a  movabs   rax, 0x1db42
fe_mul121666       00001154  imul     rcx, rax
fe_mul121666       00001158  mov      qword ptr [rbp - 0x30], rcx
fe_mul121666       0000115c  mov      rax, qword ptr [rbp + 0x18]
fe_mul121666       00001160  add      rax, 0x18
fe_mul121666       00001164  mov      ecx, dword ptr [rax]
fe_mul121666       00001166  movsxd   rcx, ecx
fe_mul121666       00001169  movabs   rax, 0x1db42
fe_mul121666       00001173  imul     rcx, rax
fe_mul121666       00001177  mov      qword ptr [rbp - 0x38], rcx
fe_mul121666       0000117b  mov      rax, qword ptr [rbp + 0x18]
fe_mul121666       0000117f  add      rax, 0x1c
fe_mul121666       00001183  mov      ecx, dword ptr [rax]
fe_mul121666       00001185  movsxd   rcx, ecx
fe_mul121666       00001188  movabs   rax, 0x1db42
fe_mul121666       00001192  imul     rcx, rax
fe_mul121666       00001196  mov      qword ptr [rbp - 0x40], rcx
fe_mul121666       0000119a  mov      rax, qword ptr [rbp + 0x18]
fe_mul121666       0000119e  add      rax, 0x20
fe_mul121666       000011a2  mov      ecx, dword ptr [rax]
fe_mul121666       000011a4  movsxd   rcx, ecx
fe_mul121666       000011a7  movabs   rax, 0x1db42
fe_mul121666       000011b1  imul     rcx, rax
fe_mul121666       000011b5  mov      qword ptr [rbp - 0x48], rcx
fe_mul121666       000011b9  mov      rax, qword ptr [rbp + 0x18]
fe_mul121666       000011bd  add      rax, 0x24
fe_mul121666       000011c1  mov      ecx, dword ptr [rax]
fe_mul121666       000011c3  movsxd   rcx, ecx
fe_mul121666       000011c6  movabs   rax, 0x1db42
fe_mul121666       000011d0  imul     rcx, rax
fe_mul121666       000011d4  mov      qword ptr [rbp - 0x50], rcx
fe_mul121666       000011d8  mov      rax, qword ptr [rbp - 0x50]
fe_mul121666       000011dc  add      rax, 0x1000000
fe_mul121666       000011e3  sar      rax, 0x19
fe_mul121666       000011e7  mov      qword ptr [rbp - 0xa0], rax
fe_mul121666       000011ee  mov      rax, qword ptr [rbp - 0xa0]
fe_mul121666       000011f5  movabs   rcx, 0x13
fe_mul121666       000011ff  imul     rax, rcx
fe_mul121666       00001203  mov      rcx, qword ptr [rbp - 8]
fe_mul121666       00001207  add      rcx, rax
fe_mul121666       0000120a  mov      qword ptr [rbp - 8], rcx
fe_mul121666       0000120e  mov      rax, qword ptr [rbp - 0xa0]
fe_mul121666       00001215  shl      rax, 0x19
fe_mul121666       00001219  mov      rcx, qword ptr [rbp - 0x50]
fe_mul121666       0000121d  sub      rcx, rax
fe_mul121666       00001220  mov      qword ptr [rbp - 0x50], rcx
fe_mul121666       00001224  mov      rax, qword ptr [rbp - 0x10]
fe_mul121666       00001228  add      rax, 0x1000000
fe_mul121666       0000122f  sar      rax, 0x19
fe_mul121666       00001233  mov      qword ptr [rbp - 0x60], rax
fe_mul121666       00001237  mov      rax, qword ptr [rbp - 0x18]
fe_mul121666       0000123b  mov      rcx, qword ptr [rbp - 0x60]
fe_mul121666       0000123f  add      rax, rcx
fe_mul121666       00001242  mov      qword ptr [rbp - 0x18], rax
fe_mul121666       00001246  mov      rax, qword ptr [rbp - 0x60]
fe_mul121666       0000124a  shl      rax, 0x19
fe_mul121666       0000124e  mov      rcx, qword ptr [rbp - 0x10]
fe_mul121666       00001252  sub      rcx, rax
fe_mul121666       00001255  mov      qword ptr [rbp - 0x10], rcx
fe_mul121666       00001259  mov      rax, qword ptr [rbp - 0x20]
fe_mul121666       0000125d  add      rax, 0x1000000
fe_mul121666       00001264  sar      rax, 0x19
fe_mul121666       00001268  mov      qword ptr [rbp - 0x70], rax
fe_mul121666       0000126c  mov      rax, qword ptr [rbp - 0x28]
fe_mul121666       00001270  mov      rcx, qword ptr [rbp - 0x70]
fe_mul121666       00001274  add      rax, rcx
fe_mul121666       00001277  mov      qword ptr [rbp - 0x28], rax
fe_mul121666       0000127b  mov      rax, qword ptr [rbp - 0x70]
fe_mul121666       0000127f  shl      rax, 0x19
fe_mul121666       00001283  mov      rcx, qword ptr [rbp - 0x20]
fe_mul121666       00001287  sub      rcx, rax
fe_mul121666       0000128a  mov      qword ptr [rbp - 0x20], rcx
fe_mul121666       0000128e  mov      rax, qword ptr [rbp - 0x30]
fe_mul121666       00001292  add      rax, 0x1000000
fe_mul121666       00001299  sar      rax, 0x19
fe_mul121666       0000129d  mov      qword ptr [rbp - 0x80], rax
fe_mul121666       000012a1  mov      rax, qword ptr [rbp - 0x38]
fe_mul121666       000012a5  mov      rcx, qword ptr [rbp - 0x80]
fe_mul121666       000012a9  add      rax, rcx
fe_mul121666       000012ac  mov      qword ptr [rbp - 0x38], rax
fe_mul121666       000012b0  mov      rax, qword ptr [rbp - 0x80]
fe_mul121666       000012b4  shl      rax, 0x19
fe_mul121666       000012b8  mov      rcx, qword ptr [rbp - 0x30]
fe_mul121666       000012bc  sub      rcx, rax
fe_mul121666       000012bf  mov      qword ptr [rbp - 0x30], rcx
fe_mul121666       000012c3  mov      rax, qword ptr [rbp - 0x40]
fe_mul121666       000012c7  add      rax, 0x1000000
fe_mul121666       000012ce  sar      rax, 0x19
fe_mul121666       000012d2  mov      qword ptr [rbp - 0x90], rax
fe_mul121666       000012d9  mov      rax, qword ptr [rbp - 0x48]
fe_mul121666       000012dd  mov      rcx, qword ptr [rbp - 0x90]
fe_mul121666       000012e4  add      rax, rcx
fe_mul121666       000012e7  mov      qword ptr [rbp - 0x48], rax
fe_mul121666       000012eb  mov      rax, qword ptr [rbp - 0x90]
fe_mul121666       000012f2  shl      rax, 0x19
fe_mul121666       000012f6  mov      rcx, qword ptr [rbp - 0x40]
fe_mul121666       000012fa  sub      rcx, rax
fe_mul121666       000012fd  mov      qword ptr [rbp - 0x40], rcx
fe_mul121666       00001301  mov      rax, qword ptr [rbp - 8]
fe_mul121666       00001305  add      rax, 0x2000000
fe_mul121666       0000130c  sar      rax, 0x1a
fe_mul121666       00001310  mov      qword ptr [rbp - 0x58], rax
fe_mul121666       00001314  mov      rax, qword ptr [rbp - 0x10]
fe_mul121666       00001318  mov      rcx, qword ptr [rbp - 0x58]
fe_mul121666       0000131c  add      rax, rcx
fe_mul121666       0000131f  mov      qword ptr [rbp - 0x10], rax
fe_mul121666       00001323  mov      rax, qword ptr [rbp - 0x58]
fe_mul121666       00001327  shl      rax, 0x1a
fe_mul121666       0000132b  mov      rcx, qword ptr [rbp - 8]
fe_mul121666       0000132f  sub      rcx, rax
fe_mul121666       00001332  mov      qword ptr [rbp - 8], rcx
fe_mul121666       00001336  mov      rax, qword ptr [rbp - 0x18]
fe_mul121666       0000133a  add      rax, 0x2000000
fe_mul121666       00001341  sar      rax, 0x1a
fe_mul121666       00001345  mov      qword ptr [rbp - 0x68], rax
fe_mul121666       00001349  mov      rax, qword ptr [rbp - 0x20]
fe_mul121666       0000134d  mov      rcx, qword ptr [rbp - 0x68]
fe_mul121666       00001351  add      rax, rcx
fe_mul121666       00001354  mov      qword ptr [rbp - 0x20], rax
fe_mul121666       00001358  mov      rax, qword ptr [rbp - 0x68]
fe_mul121666       0000135c  shl      rax, 0x1a
fe_mul121666       00001360  mov      rcx, qword ptr [rbp - 0x18]
fe_mul121666       00001364  sub      rcx, rax
fe_mul121666       00001367  mov      qword ptr [rbp - 0x18], rcx
fe_mul121666       0000136b  mov      rax, qword ptr [rbp - 0x28]
fe_mul121666       0000136f  add      rax, 0x2000000
fe_mul121666       00001376  sar      rax, 0x1a
fe_mul121666       0000137a  mov      qword ptr [rbp - 0x78], rax
fe_mul121666       0000137e  mov      rax, qword ptr [rbp - 0x30]
fe_mul121666       00001382  mov      rcx, qword ptr [rbp - 0x78]
fe_mul121666       00001386  add      rax, rcx
fe_mul121666       00001389  mov      qword ptr [rbp - 0x30], rax
fe_mul121666       0000138d  mov      rax, qword ptr [rbp - 0x78]
fe_mul121666       00001391  shl      rax, 0x1a
fe_mul121666       00001395  mov      rcx, qword ptr [rbp - 0x28]
fe_mul121666       00001399  sub      rcx, rax
fe_mul121666       0000139c  mov      qword ptr [rbp - 0x28], rcx
fe_mul121666       000013a0  mov      rax, qword ptr [rbp - 0x38]
fe_mul121666       000013a4  add      rax, 0x2000000
fe_mul121666       000013ab  sar      rax, 0x1a
fe_mul121666       000013af  mov      qword ptr [rbp - 0x88], rax
fe_mul121666       000013b6  mov      rax, qword ptr [rbp - 0x40]
fe_mul121666       000013ba  mov      rcx, qword ptr [rbp - 0x88]
fe_mul121666       000013c1  add      rax, rcx
fe_mul121666       000013c4  mov      qword ptr [rbp - 0x40], rax
fe_mul121666       000013c8  mov      rax, qword ptr [rbp - 0x88]
fe_mul121666       000013cf  shl      rax, 0x1a
fe_mul121666       000013d3  mov      rcx, qword ptr [rbp - 0x38]
fe_mul121666       000013d7  sub      rcx, rax
fe_mul121666       000013da  mov      qword ptr [rbp - 0x38], rcx
fe_mul121666       000013de  mov      rax, qword ptr [rbp - 0x48]
fe_mul121666       000013e2  add      rax, 0x2000000
fe_mul121666       000013e9  sar      rax, 0x1a
fe_mul121666       000013ed  mov      qword ptr [rbp - 0x98], rax
fe_mul121666       000013f4  mov      rax, qword ptr [rbp - 0x50]
fe_mul121666       000013f8  mov      rcx, qword ptr [rbp - 0x98]
fe_mul121666       000013ff  add      rax, rcx
fe_mul121666       00001402  mov      qword ptr [rbp - 0x50], rax
fe_mul121666       00001406  mov      rax, qword ptr [rbp - 0x98]
fe_mul121666       0000140d  shl      rax, 0x1a
fe_mul121666       00001411  mov      rcx, qword ptr [rbp - 0x48]
fe_mul121666       00001415  sub      rcx, rax
fe_mul121666       00001418  mov      qword ptr [rbp - 0x48], rcx
fe_mul121666       0000141c  mov      rax, qword ptr [rbp + 0x10]
fe_mul121666       00001420  mov      ecx, dword ptr [rbp - 8]
fe_mul121666       00001423  mov      dword ptr [rax], ecx
fe_mul121666       00001425  mov      rax, qword ptr [rbp + 0x10]
fe_mul121666       00001429  add      rax, 4
fe_mul121666       0000142d  mov      ecx, dword ptr [rbp - 0x10]
fe_mul121666       00001430  mov      dword ptr [rax], ecx
fe_mul121666       00001432  mov      rax, qword ptr [rbp + 0x10]
fe_mul121666       00001436  add      rax, 8
fe_mul121666       0000143a  mov      ecx, dword ptr [rbp - 0x18]
fe_mul121666       0000143d  mov      dword ptr [rax], ecx
fe_mul121666       0000143f  mov      rax, qword ptr [rbp + 0x10]
fe_mul121666       00001443  add      rax, 0xc
fe_mul121666       00001447  mov      ecx, dword ptr [rbp - 0x20]
fe_mul121666       0000144a  mov      dword ptr [rax], ecx
fe_mul121666       0000144c  mov      rax, qword ptr [rbp + 0x10]
fe_mul121666       00001450  add      rax, 0x10
fe_mul121666       00001454  mov      ecx, dword ptr [rbp - 0x28]
fe_mul121666       00001457  mov      dword ptr [rax], ecx
fe_mul121666       00001459  mov      rax, qword ptr [rbp + 0x10]
fe_mul121666       0000145d  add      rax, 0x14
fe_mul121666       00001461  mov      ecx, dword ptr [rbp - 0x30]
fe_mul121666       00001464  mov      dword ptr [rax], ecx
fe_mul121666       00001466  mov      rax, qword ptr [rbp + 0x10]
fe_mul121666       0000146a  add      rax, 0x18
fe_mul121666       0000146e  mov      ecx, dword ptr [rbp - 0x38]
fe_mul121666       00001471  mov      dword ptr [rax], ecx
fe_mul121666       00001473  mov      rax, qword ptr [rbp + 0x10]
fe_mul121666       00001477  add      rax, 0x1c
fe_mul121666       0000147b  mov      ecx, dword ptr [rbp - 0x40]
fe_mul121666       0000147e  mov      dword ptr [rax], ecx
fe_mul121666       00001480  mov      rax, qword ptr [rbp + 0x10]
fe_mul121666       00001484  add      rax, 0x20
fe_mul121666       00001488  mov      ecx, dword ptr [rbp - 0x48]
fe_mul121666       0000148b  mov      dword ptr [rax], ecx
fe_mul121666       0000148d  mov      rax, qword ptr [rbp + 0x10]
fe_mul121666       00001491  add      rax, 0x24
fe_mul121666       00001495  mov      ecx, dword ptr [rbp - 0x50]
fe_mul121666       00001498  mov      dword ptr [rax], ecx
fe_mul121666       0000149a  leave    
fe_mul121666       0000149b  ret      
fe_invert          0000149c  push     rbp
fe_invert          0000149d  mov      rbp, rsp
fe_invert          000014a0  sub      rsp, 0xd0
fe_invert          000014a7  mov      qword ptr [rbp + 0x10], rcx
fe_invert          000014ab  mov      qword ptr [rbp + 0x18], rdx
fe_invert          000014af  mov      rax, qword ptr [rbp + 0x18]
fe_invert          000014b3  mov      r11, rax
fe_invert          000014b6  lea      rax, [rbp - 0x28]
fe_invert          000014ba  mov      r10, rax
fe_invert          000014bd  mov      rcx, r10
fe_invert          000014c0  mov      rdx, r11
fe_invert          000014c3  call     0x14c8
fe_invert          000014c8  lea      rax, [rbp - 0x28]
fe_invert          000014cc  mov      r11, rax
fe_invert          000014cf  lea      rax, [rbp - 0x50]
fe_invert          000014d3  mov      r10, rax
fe_invert          000014d6  mov      rcx, r10
fe_invert          000014d9  mov      rdx, r11
fe_invert          000014dc  call     0x14e1
fe_invert          000014e1  lea      rax, [rbp - 0x50]
fe_invert          000014e5  mov      r11, rax
fe_invert          000014e8  lea      rax, [rbp - 0x50]
fe_invert          000014ec  mov      r10, rax
fe_invert          000014ef  mov      rcx, r10
fe_invert          000014f2  mov      rdx, r11
fe_invert          000014f5  call     0x14fa
fe_invert          000014fa  lea      rax, [rbp - 0x50]
fe_invert          000014fe  mov      r8, rax
fe_invert          00001501  mov      rax, qword ptr [rbp + 0x18]
fe_invert          00001505  mov      r11, rax
fe_invert          00001508  lea      rax, [rbp - 0x50]
fe_invert          0000150c  mov      r10, rax
fe_invert          0000150f  mov      rcx, r10
fe_invert          00001512  mov      rdx, r11
fe_invert          00001515  call     0x151a
fe_invert          0000151a  lea      rax, [rbp - 0x50]
fe_invert          0000151e  mov      r8, rax
fe_invert          00001521  lea      rax, [rbp - 0x28]
fe_invert          00001525  mov      r11, rax
fe_invert          00001528  lea      rax, [rbp - 0x28]
fe_invert          0000152c  mov      r10, rax
fe_invert          0000152f  mov      rcx, r10
fe_invert          00001532  mov      rdx, r11
fe_invert          00001535  call     0x153a
fe_invert          0000153a  lea      rax, [rbp - 0x28]
fe_invert          0000153e  mov      r11, rax
fe_invert          00001541  lea      rax, [rbp - 0x78]
fe_invert          00001545  mov      r10, rax
fe_invert          00001548  mov      rcx, r10
fe_invert          0000154b  mov      rdx, r11
fe_invert          0000154e  call     0x1553
fe_invert          00001553  lea      rax, [rbp - 0x78]
fe_invert          00001557  mov      r8, rax
fe_invert          0000155a  lea      rax, [rbp - 0x50]
fe_invert          0000155e  mov      r11, rax
fe_invert          00001561  lea      rax, [rbp - 0x50]
fe_invert          00001565  mov      r10, rax
fe_invert          00001568  mov      rcx, r10
fe_invert          0000156b  mov      rdx, r11
fe_invert          0000156e  call     0x1573
fe_invert          00001573  lea      rax, [rbp - 0x50]
fe_invert          00001577  mov      r11, rax
fe_invert          0000157a  lea      rax, [rbp - 0x78]
fe_invert          0000157e  mov      r10, rax
fe_invert          00001581  mov      rcx, r10
fe_invert          00001584  mov      rdx, r11
fe_invert          00001587  call     0x158c
fe_invert          0000158c  mov      eax, 1
fe_invert          00001591  mov      dword ptr [rbp - 0xa4], eax
fe_invert          00001597  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          0000159d  cmp      eax, 5
fe_invert          000015a0  jge      0x15da
fe_invert          000015a6  jmp      0x15bf
fe_invert          000015ab  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          000015b1  mov      rcx, rax
fe_invert          000015b4  add      eax, 1
fe_invert          000015b7  mov      dword ptr [rbp - 0xa4], eax
fe_invert          000015bd  jmp      0x1597
fe_invert          000015bf  lea      rax, [rbp - 0x78]
fe_invert          000015c3  mov      r11, rax
fe_invert          000015c6  lea      rax, [rbp - 0x78]
fe_invert          000015ca  mov      r10, rax
fe_invert          000015cd  mov      rcx, r10
fe_invert          000015d0  mov      rdx, r11
fe_invert          000015d3  call     0x15d8
fe_invert          000015d8  jmp      0x15ab
fe_invert          000015da  lea      rax, [rbp - 0x50]
fe_invert          000015de  mov      r8, rax
fe_invert          000015e1  lea      rax, [rbp - 0x78]
fe_invert          000015e5  mov      r11, rax
fe_invert          000015e8  lea      rax, [rbp - 0x50]
fe_invert          000015ec  mov      r10, rax
fe_invert          000015ef  mov      rcx, r10
fe_invert          000015f2  mov      rdx, r11
fe_invert          000015f5  call     0x15fa
fe_invert          000015fa  lea      rax, [rbp - 0x50]
fe_invert          000015fe  mov      r11, rax
fe_invert          00001601  lea      rax, [rbp - 0x78]
fe_invert          00001605  mov      r10, rax
fe_invert          00001608  mov      rcx, r10
fe_invert          0000160b  mov      rdx, r11
fe_invert          0000160e  call     0x1613
fe_invert          00001613  mov      eax, 1
fe_invert          00001618  mov      dword ptr [rbp - 0xa4], eax
fe_invert          0000161e  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          00001624  cmp      eax, 0xa
fe_invert          00001627  jge      0x1661
fe_invert          0000162d  jmp      0x1646
fe_invert          00001632  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          00001638  mov      rcx, rax
fe_invert          0000163b  add      eax, 1
fe_invert          0000163e  mov      dword ptr [rbp - 0xa4], eax
fe_invert          00001644  jmp      0x161e
fe_invert          00001646  lea      rax, [rbp - 0x78]
fe_invert          0000164a  mov      r11, rax
fe_invert          0000164d  lea      rax, [rbp - 0x78]
fe_invert          00001651  mov      r10, rax
fe_invert          00001654  mov      rcx, r10
fe_invert          00001657  mov      rdx, r11
fe_invert          0000165a  call     0x165f
fe_invert          0000165f  jmp      0x1632
fe_invert          00001661  lea      rax, [rbp - 0x50]
fe_invert          00001665  mov      r8, rax
fe_invert          00001668  lea      rax, [rbp - 0x78]
fe_invert          0000166c  mov      r11, rax
fe_invert          0000166f  lea      rax, [rbp - 0x78]
fe_invert          00001673  mov      r10, rax
fe_invert          00001676  mov      rcx, r10
fe_invert          00001679  mov      rdx, r11
fe_invert          0000167c  call     0x1681
fe_invert          00001681  lea      rax, [rbp - 0x78]
fe_invert          00001685  mov      r11, rax
fe_invert          00001688  lea      rax, [rbp - 0xa0]
fe_invert          0000168f  mov      r10, rax
fe_invert          00001692  mov      rcx, r10
fe_invert          00001695  mov      rdx, r11
fe_invert          00001698  call     0x169d
fe_invert          0000169d  mov      eax, 1
fe_invert          000016a2  mov      dword ptr [rbp - 0xa4], eax
fe_invert          000016a8  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          000016ae  cmp      eax, 0x14
fe_invert          000016b1  jge      0x16f1
fe_invert          000016b7  jmp      0x16d0
fe_invert          000016bc  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          000016c2  mov      rcx, rax
fe_invert          000016c5  add      eax, 1
fe_invert          000016c8  mov      dword ptr [rbp - 0xa4], eax
fe_invert          000016ce  jmp      0x16a8
fe_invert          000016d0  lea      rax, [rbp - 0xa0]
fe_invert          000016d7  mov      r11, rax
fe_invert          000016da  lea      rax, [rbp - 0xa0]
fe_invert          000016e1  mov      r10, rax
fe_invert          000016e4  mov      rcx, r10
fe_invert          000016e7  mov      rdx, r11
fe_invert          000016ea  call     0x16ef
fe_invert          000016ef  jmp      0x16bc
fe_invert          000016f1  lea      rax, [rbp - 0x78]
fe_invert          000016f5  mov      r8, rax
fe_invert          000016f8  lea      rax, [rbp - 0xa0]
fe_invert          000016ff  mov      r11, rax
fe_invert          00001702  lea      rax, [rbp - 0x78]
fe_invert          00001706  mov      r10, rax
fe_invert          00001709  mov      rcx, r10
fe_invert          0000170c  mov      rdx, r11
fe_invert          0000170f  call     0x1714
fe_invert          00001714  lea      rax, [rbp - 0x78]
fe_invert          00001718  mov      r11, rax
fe_invert          0000171b  lea      rax, [rbp - 0x78]
fe_invert          0000171f  mov      r10, rax
fe_invert          00001722  mov      rcx, r10
fe_invert          00001725  mov      rdx, r11
fe_invert          00001728  call     0x172d
fe_invert          0000172d  mov      eax, 1
fe_invert          00001732  mov      dword ptr [rbp - 0xa4], eax
fe_invert          00001738  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          0000173e  cmp      eax, 0xa
fe_invert          00001741  jge      0x177b
fe_invert          00001747  jmp      0x1760
fe_invert          0000174c  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          00001752  mov      rcx, rax
fe_invert          00001755  add      eax, 1
fe_invert          00001758  mov      dword ptr [rbp - 0xa4], eax
fe_invert          0000175e  jmp      0x1738
fe_invert          00001760  lea      rax, [rbp - 0x78]
fe_invert          00001764  mov      r11, rax
fe_invert          00001767  lea      rax, [rbp - 0x78]
fe_invert          0000176b  mov      r10, rax
fe_invert          0000176e  mov      rcx, r10
fe_invert          00001771  mov      rdx, r11
fe_invert          00001774  call     0x1779
fe_invert          00001779  jmp      0x174c
fe_invert          0000177b  lea      rax, [rbp - 0x50]
fe_invert          0000177f  mov      r8, rax
fe_invert          00001782  lea      rax, [rbp - 0x78]
fe_invert          00001786  mov      r11, rax
fe_invert          00001789  lea      rax, [rbp - 0x50]
fe_invert          0000178d  mov      r10, rax
fe_invert          00001790  mov      rcx, r10
fe_invert          00001793  mov      rdx, r11
fe_invert          00001796  call     0x179b
fe_invert          0000179b  lea      rax, [rbp - 0x50]
fe_invert          0000179f  mov      r11, rax
fe_invert          000017a2  lea      rax, [rbp - 0x78]
fe_invert          000017a6  mov      r10, rax
fe_invert          000017a9  mov      rcx, r10
fe_invert          000017ac  mov      rdx, r11
fe_invert          000017af  call     0x17b4
fe_invert          000017b4  mov      eax, 1
fe_invert          000017b9  mov      dword ptr [rbp - 0xa4], eax
fe_invert          000017bf  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          000017c5  cmp      eax, 0x32
fe_invert          000017c8  jge      0x1802
fe_invert          000017ce  jmp      0x17e7
fe_invert          000017d3  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          000017d9  mov      rcx, rax
fe_invert          000017dc  add      eax, 1
fe_invert          000017df  mov      dword ptr [rbp - 0xa4], eax
fe_invert          000017e5  jmp      0x17bf
fe_invert          000017e7  lea      rax, [rbp - 0x78]
fe_invert          000017eb  mov      r11, rax
fe_invert          000017ee  lea      rax, [rbp - 0x78]
fe_invert          000017f2  mov      r10, rax
fe_invert          000017f5  mov      rcx, r10
fe_invert          000017f8  mov      rdx, r11
fe_invert          000017fb  call     0x1800
fe_invert          00001800  jmp      0x17d3
fe_invert          00001802  lea      rax, [rbp - 0x50]
fe_invert          00001806  mov      r8, rax
fe_invert          00001809  lea      rax, [rbp - 0x78]
fe_invert          0000180d  mov      r11, rax
fe_invert          00001810  lea      rax, [rbp - 0x78]
fe_invert          00001814  mov      r10, rax
fe_invert          00001817  mov      rcx, r10
fe_invert          0000181a  mov      rdx, r11
fe_invert          0000181d  call     0x1822
fe_invert          00001822  lea      rax, [rbp - 0x78]
fe_invert          00001826  mov      r11, rax
fe_invert          00001829  lea      rax, [rbp - 0xa0]
fe_invert          00001830  mov      r10, rax
fe_invert          00001833  mov      rcx, r10
fe_invert          00001836  mov      rdx, r11
fe_invert          00001839  call     0x183e
fe_invert          0000183e  mov      eax, 1
fe_invert          00001843  mov      dword ptr [rbp - 0xa4], eax
fe_invert          00001849  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          0000184f  cmp      eax, 0x64
fe_invert          00001852  jge      0x1892
fe_invert          00001858  jmp      0x1871
fe_invert          0000185d  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          00001863  mov      rcx, rax
fe_invert          00001866  add      eax, 1
fe_invert          00001869  mov      dword ptr [rbp - 0xa4], eax
fe_invert          0000186f  jmp      0x1849
fe_invert          00001871  lea      rax, [rbp - 0xa0]
fe_invert          00001878  mov      r11, rax
fe_invert          0000187b  lea      rax, [rbp - 0xa0]
fe_invert          00001882  mov      r10, rax
fe_invert          00001885  mov      rcx, r10
fe_invert          00001888  mov      rdx, r11
fe_invert          0000188b  call     0x1890
fe_invert          00001890  jmp      0x185d
fe_invert          00001892  lea      rax, [rbp - 0x78]
fe_invert          00001896  mov      r8, rax
fe_invert          00001899  lea      rax, [rbp - 0xa0]
fe_invert          000018a0  mov      r11, rax
fe_invert          000018a3  lea      rax, [rbp - 0x78]
fe_invert          000018a7  mov      r10, rax
fe_invert          000018aa  mov      rcx, r10
fe_invert          000018ad  mov      rdx, r11
fe_invert          000018b0  call     0x18b5
fe_invert          000018b5  lea      rax, [rbp - 0x78]
fe_invert          000018b9  mov      r11, rax
fe_invert          000018bc  lea      rax, [rbp - 0x78]
fe_invert          000018c0  mov      r10, rax
fe_invert          000018c3  mov      rcx, r10
fe_invert          000018c6  mov      rdx, r11
fe_invert          000018c9  call     0x18ce
fe_invert          000018ce  mov      eax, 1
fe_invert          000018d3  mov      dword ptr [rbp - 0xa4], eax
fe_invert          000018d9  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          000018df  cmp      eax, 0x32
fe_invert          000018e2  jge      0x191c
fe_invert          000018e8  jmp      0x1901
fe_invert          000018ed  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          000018f3  mov      rcx, rax
fe_invert          000018f6  add      eax, 1
fe_invert          000018f9  mov      dword ptr [rbp - 0xa4], eax
fe_invert          000018ff  jmp      0x18d9
fe_invert          00001901  lea      rax, [rbp - 0x78]
fe_invert          00001905  mov      r11, rax
fe_invert          00001908  lea      rax, [rbp - 0x78]
fe_invert          0000190c  mov      r10, rax
fe_invert          0000190f  mov      rcx, r10
fe_invert          00001912  mov      rdx, r11
fe_invert          00001915  call     0x191a
fe_invert          0000191a  jmp      0x18ed
fe_invert          0000191c  lea      rax, [rbp - 0x50]
fe_invert          00001920  mov      r8, rax
fe_invert          00001923  lea      rax, [rbp - 0x78]
fe_invert          00001927  mov      r11, rax
fe_invert          0000192a  lea      rax, [rbp - 0x50]
fe_invert          0000192e  mov      r10, rax
fe_invert          00001931  mov      rcx, r10
fe_invert          00001934  mov      rdx, r11
fe_invert          00001937  call     0x193c
fe_invert          0000193c  lea      rax, [rbp - 0x50]
fe_invert          00001940  mov      r11, rax
fe_invert          00001943  lea      rax, [rbp - 0x50]
fe_invert          00001947  mov      r10, rax
fe_invert          0000194a  mov      rcx, r10
fe_invert          0000194d  mov      rdx, r11
fe_invert          00001950  call     0x1955
fe_invert          00001955  mov      eax, 1
fe_invert          0000195a  mov      dword ptr [rbp - 0xa4], eax
fe_invert          00001960  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          00001966  cmp      eax, 5
fe_invert          00001969  jge      0x19a3
fe_invert          0000196f  jmp      0x1988
fe_invert          00001974  mov      eax, dword ptr [rbp - 0xa4]
fe_invert          0000197a  mov      rcx, rax
fe_invert          0000197d  add      eax, 1
fe_invert          00001980  mov      dword ptr [rbp - 0xa4], eax
fe_invert          00001986  jmp      0x1960
fe_invert          00001988  lea      rax, [rbp - 0x50]
fe_invert          0000198c  mov      r11, rax
fe_invert          0000198f  lea      rax, [rbp - 0x50]
fe_invert          00001993  mov      r10, rax
fe_invert          00001996  mov      rcx, r10
fe_invert          00001999  mov      rdx, r11
fe_invert          0000199c  call     0x19a1
fe_invert          000019a1  jmp      0x1974
fe_invert          000019a3  lea      rax, [rbp - 0x28]
fe_invert          000019a7  mov      r8, rax
fe_invert          000019aa  lea      rax, [rbp - 0x50]
fe_invert          000019ae  mov      r11, rax
fe_invert          000019b1  mov      rax, qword ptr [rbp + 0x10]
fe_invert          000019b5  mov      r10, rax
fe_invert          000019b8  mov      rcx, r10
fe_invert          000019bb  mov      rdx, r11
fe_invert          000019be  call     0x19c3
fe_invert          000019c3  leave    
fe_invert          000019c4  ret      
load_3             000019c5  push     rbp
load_3             000019c6  mov      rbp, rsp
load_3             000019c9  sub      rsp, 0
load_3             000019d0  mov      qword ptr [rbp + 0x10], rcx
load_3             000019d4  mov      rax, qword ptr [rbp + 0x10]
load_3             000019d8  movzx    ecx, byte ptr [rax]
load_3             000019db  movsxd   rcx, ecx
load_3             000019de  mov      rax, qword ptr [rbp + 0x10]
load_3             000019e2  add      rax, 1
load_3             000019e6  movzx    edx, byte ptr [rax]
load_3             000019e9  movsxd   rdx, edx
load_3             000019ec  shl      rdx, 8
load_3             000019f0  or       rcx, rdx
load_3             000019f3  mov      rax, qword ptr [rbp + 0x10]
load_3             000019f7  add      rax, 2
load_3             000019fb  movzx    edx, byte ptr [rax]
load_3             000019fe  movsxd   rdx, edx
load_3             00001a01  shl      rdx, 0x10
load_3             00001a05  or       rcx, rdx
load_3             00001a08  mov      rax, rcx
load_3             00001a0b  leave    
load_3             00001a0c  ret      
load_4             00001a0d  push     rbp
load_4             00001a0e  mov      rbp, rsp
load_4             00001a11  sub      rsp, 0
load_4             00001a18  mov      qword ptr [rbp + 0x10], rcx
load_4             00001a1c  mov      rax, qword ptr [rbp + 0x10]
load_4             00001a20  movzx    ecx, byte ptr [rax]
load_4             00001a23  movsxd   rcx, ecx
load_4             00001a26  mov      rax, qword ptr [rbp + 0x10]
load_4             00001a2a  add      rax, 1
load_4             00001a2e  movzx    edx, byte ptr [rax]
load_4             00001a31  movsxd   rdx, edx
load_4             00001a34  shl      rdx, 8
load_4             00001a38  or       rcx, rdx
load_4             00001a3b  mov      rax, qword ptr [rbp + 0x10]
load_4             00001a3f  add      rax, 2
load_4             00001a43  movzx    edx, byte ptr [rax]
load_4             00001a46  movsxd   rdx, edx
load_4             00001a49  shl      rdx, 0x10
load_4             00001a4d  or       rcx, rdx
load_4             00001a50  mov      rax, qword ptr [rbp + 0x10]
load_4             00001a54  add      rax, 3
load_4             00001a58  movzx    edx, byte ptr [rax]
load_4             00001a5b  movsxd   rdx, edx
load_4             00001a5e  shl      rdx, 0x18
load_4             00001a62  or       rcx, rdx
load_4             00001a65  mov      rax, rcx
load_4             00001a68  leave    
load_4             00001a69  ret      
fe_frombytes       00001a6a  push     rbp
fe_frombytes       00001a6b  mov      rbp, rsp
fe_frombytes       00001a6e  sub      rsp, 0xc0
fe_frombytes       00001a75  mov      qword ptr [rbp + 0x10], rcx
fe_frombytes       00001a79  mov      qword ptr [rbp + 0x18], rdx
fe_frombytes       00001a7d  mov      rax, qword ptr [rbp + 0x18]
fe_frombytes       00001a81  mov      r10, rax
fe_frombytes       00001a84  mov      rcx, r10
fe_frombytes       00001a87  call     0x1a8c
fe_frombytes       00001a8c  mov      qword ptr [rbp - 8], rax
fe_frombytes       00001a90  mov      rax, qword ptr [rbp + 0x18]
fe_frombytes       00001a94  add      rax, 4
fe_frombytes       00001a98  mov      r10, rax
fe_frombytes       00001a9b  mov      rcx, r10
fe_frombytes       00001a9e  call     0x1aa3
fe_frombytes       00001aa3  shl      rax, 6
fe_frombytes       00001aa7  mov      qword ptr [rbp - 0x10], rax
fe_frombytes       00001aab  mov      rax, qword ptr [rbp + 0x18]
fe_frombytes       00001aaf  add      rax, 7
fe_frombytes       00001ab3  mov      r10, rax
fe_frombytes       00001ab6  mov      rcx, r10
fe_frombytes       00001ab9  call     0x1abe
fe_frombytes       00001abe  shl      rax, 5
fe_frombytes       00001ac2  mov      qword ptr [rbp - 0x18], rax
fe_frombytes       00001ac6  mov      rax, qword ptr [rbp + 0x18]
fe_frombytes       00001aca  add      rax, 0xa
fe_frombytes       00001ace  mov      r10, rax
fe_frombytes       00001ad1  mov      rcx, r10
fe_frombytes       00001ad4  call     0x1ad9
fe_frombytes       00001ad9  shl      rax, 3
fe_frombytes       00001add  mov      qword ptr [rbp - 0x20], rax
fe_frombytes       00001ae1  mov      rax, qword ptr [rbp + 0x18]
fe_frombytes       00001ae5  add      rax, 0xd
fe_frombytes       00001ae9  mov      r10, rax
fe_frombytes       00001aec  mov      rcx, r10
fe_frombytes       00001aef  call     0x1af4
fe_frombytes       00001af4  shl      rax, 2
fe_frombytes       00001af8  mov      qword ptr [rbp - 0x28], rax
fe_frombytes       00001afc  mov      rax, qword ptr [rbp + 0x18]
fe_frombytes       00001b00  add      rax, 0x10
fe_frombytes       00001b04  mov      r10, rax
fe_frombytes       00001b07  mov      rcx, r10
fe_frombytes       00001b0a  call     0x1b0f
fe_frombytes       00001b0f  mov      qword ptr [rbp - 0x30], rax
fe_frombytes       00001b13  mov      rax, qword ptr [rbp + 0x18]
fe_frombytes       00001b17  add      rax, 0x14
fe_frombytes       00001b1b  mov      r10, rax
fe_frombytes       00001b1e  mov      rcx, r10
fe_frombytes       00001b21  call     0x1b26
fe_frombytes       00001b26  shl      rax, 7
fe_frombytes       00001b2a  mov      qword ptr [rbp - 0x38], rax
fe_frombytes       00001b2e  mov      rax, qword ptr [rbp + 0x18]
fe_frombytes       00001b32  add      rax, 0x17
fe_frombytes       00001b36  mov      r10, rax
fe_frombytes       00001b39  mov      rcx, r10
fe_frombytes       00001b3c  call     0x1b41
fe_frombytes       00001b41  shl      rax, 5
fe_frombytes       00001b45  mov      qword ptr [rbp - 0x40], rax
fe_frombytes       00001b49  mov      rax, qword ptr [rbp + 0x18]
fe_frombytes       00001b4d  add      rax, 0x1a
fe_frombytes       00001b51  mov      r10, rax
fe_frombytes       00001b54  mov      rcx, r10
fe_frombytes       00001b57  call     0x1b5c
fe_frombytes       00001b5c  shl      rax, 4
fe_frombytes       00001b60  mov      qword ptr [rbp - 0x48], rax
fe_frombytes       00001b64  mov      rax, qword ptr [rbp + 0x18]
fe_frombytes       00001b68  add      rax, 0x1d
fe_frombytes       00001b6c  mov      r10, rax
fe_frombytes       00001b6f  mov      rcx, r10
fe_frombytes       00001b72  call     0x1b77
fe_frombytes       00001b77  and      rax, 0x7fffff
fe_frombytes       00001b7e  shl      rax, 2
fe_frombytes       00001b82  mov      qword ptr [rbp - 0x50], rax
fe_frombytes       00001b86  mov      rax, qword ptr [rbp - 0x50]
fe_frombytes       00001b8a  add      rax, 0x1000000
fe_frombytes       00001b91  sar      rax, 0x19
fe_frombytes       00001b95  mov      qword ptr [rbp - 0xa0], rax
fe_frombytes       00001b9c  mov      rax, qword ptr [rbp - 0xa0]
fe_frombytes       00001ba3  movabs   rcx, 0x13
fe_frombytes       00001bad  imul     rax, rcx
fe_frombytes       00001bb1  mov      rcx, qword ptr [rbp - 8]
fe_frombytes       00001bb5  add      rcx, rax
fe_frombytes       00001bb8  mov      qword ptr [rbp - 8], rcx
fe_frombytes       00001bbc  mov      rax, qword ptr [rbp - 0xa0]
fe_frombytes       00001bc3  shl      rax, 0x19
fe_frombytes       00001bc7  mov      rcx, qword ptr [rbp - 0x50]
fe_frombytes       00001bcb  sub      rcx, rax
fe_frombytes       00001bce  mov      qword ptr [rbp - 0x50], rcx
fe_frombytes       00001bd2  mov      rax, qword ptr [rbp - 0x10]
fe_frombytes       00001bd6  add      rax, 0x1000000
fe_frombytes       00001bdd  sar      rax, 0x19
fe_frombytes       00001be1  mov      qword ptr [rbp - 0x60], rax
fe_frombytes       00001be5  mov      rax, qword ptr [rbp - 0x18]
fe_frombytes       00001be9  mov      rcx, qword ptr [rbp - 0x60]
fe_frombytes       00001bed  add      rax, rcx
fe_frombytes       00001bf0  mov      qword ptr [rbp - 0x18], rax
fe_frombytes       00001bf4  mov      rax, qword ptr [rbp - 0x60]
fe_frombytes       00001bf8  shl      rax, 0x19
fe_frombytes       00001bfc  mov      rcx, qword ptr [rbp - 0x10]
fe_frombytes       00001c00  sub      rcx, rax
fe_frombytes       00001c03  mov      qword ptr [rbp - 0x10], rcx
fe_frombytes       00001c07  mov      rax, qword ptr [rbp - 0x20]
fe_frombytes       00001c0b  add      rax, 0x1000000
fe_frombytes       00001c12  sar      rax, 0x19
fe_frombytes       00001c16  mov      qword ptr [rbp - 0x70], rax
fe_frombytes       00001c1a  mov      rax, qword ptr [rbp - 0x28]
fe_frombytes       00001c1e  mov      rcx, qword ptr [rbp - 0x70]
fe_frombytes       00001c22  add      rax, rcx
fe_frombytes       00001c25  mov      qword ptr [rbp - 0x28], rax
fe_frombytes       00001c29  mov      rax, qword ptr [rbp - 0x70]
fe_frombytes       00001c2d  shl      rax, 0x19
fe_frombytes       00001c31  mov      rcx, qword ptr [rbp - 0x20]
fe_frombytes       00001c35  sub      rcx, rax
fe_frombytes       00001c38  mov      qword ptr [rbp - 0x20], rcx
fe_frombytes       00001c3c  mov      rax, qword ptr [rbp - 0x30]
fe_frombytes       00001c40  add      rax, 0x1000000
fe_frombytes       00001c47  sar      rax, 0x19
fe_frombytes       00001c4b  mov      qword ptr [rbp - 0x80], rax
fe_frombytes       00001c4f  mov      rax, qword ptr [rbp - 0x38]
fe_frombytes       00001c53  mov      rcx, qword ptr [rbp - 0x80]
fe_frombytes       00001c57  add      rax, rcx
fe_frombytes       00001c5a  mov      qword ptr [rbp - 0x38], rax
fe_frombytes       00001c5e  mov      rax, qword ptr [rbp - 0x80]
fe_frombytes       00001c62  shl      rax, 0x19
fe_frombytes       00001c66  mov      rcx, qword ptr [rbp - 0x30]
fe_frombytes       00001c6a  sub      rcx, rax
fe_frombytes       00001c6d  mov      qword ptr [rbp - 0x30], rcx
fe_frombytes       00001c71  mov      rax, qword ptr [rbp - 0x40]
fe_frombytes       00001c75  add      rax, 0x1000000
fe_frombytes       00001c7c  sar      rax, 0x19
fe_frombytes       00001c80  mov      qword ptr [rbp - 0x90], rax
fe_frombytes       00001c87  mov      rax, qword ptr [rbp - 0x48]
fe_frombytes       00001c8b  mov      rcx, qword ptr [rbp - 0x90]
fe_frombytes       00001c92  add      rax, rcx
fe_frombytes       00001c95  mov      qword ptr [rbp - 0x48], rax
fe_frombytes       00001c99  mov      rax, qword ptr [rbp - 0x90]
fe_frombytes       00001ca0  shl      rax, 0x19
fe_frombytes       00001ca4  mov      rcx, qword ptr [rbp - 0x40]
fe_frombytes       00001ca8  sub      rcx, rax
fe_frombytes       00001cab  mov      qword ptr [rbp - 0x40], rcx
fe_frombytes       00001caf  mov      rax, qword ptr [rbp - 8]
fe_frombytes       00001cb3  add      rax, 0x2000000
fe_frombytes       00001cba  sar      rax, 0x1a
fe_frombytes       00001cbe  mov      qword ptr [rbp - 0x58], rax
fe_frombytes       00001cc2  mov      rax, qword ptr [rbp - 0x10]
fe_frombytes       00001cc6  mov      rcx, qword ptr [rbp - 0x58]
fe_frombytes       00001cca  add      rax, rcx
fe_frombytes       00001ccd  mov      qword ptr [rbp - 0x10], rax
fe_frombytes       00001cd1  mov      rax, qword ptr [rbp - 0x58]
fe_frombytes       00001cd5  shl      rax, 0x1a
fe_frombytes       00001cd9  mov      rcx, qword ptr [rbp - 8]
fe_frombytes       00001cdd  sub      rcx, rax
fe_frombytes       00001ce0  mov      qword ptr [rbp - 8], rcx
fe_frombytes       00001ce4  mov      rax, qword ptr [rbp - 0x18]
fe_frombytes       00001ce8  add      rax, 0x2000000
fe_frombytes       00001cef  sar      rax, 0x1a
fe_frombytes       00001cf3  mov      qword ptr [rbp - 0x68], rax
fe_frombytes       00001cf7  mov      rax, qword ptr [rbp - 0x20]
fe_frombytes       00001cfb  mov      rcx, qword ptr [rbp - 0x68]
fe_frombytes       00001cff  add      rax, rcx
fe_frombytes       00001d02  mov      qword ptr [rbp - 0x20], rax
fe_frombytes       00001d06  mov      rax, qword ptr [rbp - 0x68]
fe_frombytes       00001d0a  shl      rax, 0x1a
fe_frombytes       00001d0e  mov      rcx, qword ptr [rbp - 0x18]
fe_frombytes       00001d12  sub      rcx, rax
fe_frombytes       00001d15  mov      qword ptr [rbp - 0x18], rcx
fe_frombytes       00001d19  mov      rax, qword ptr [rbp - 0x28]
fe_frombytes       00001d1d  add      rax, 0x2000000
fe_frombytes       00001d24  sar      rax, 0x1a
fe_frombytes       00001d28  mov      qword ptr [rbp - 0x78], rax
fe_frombytes       00001d2c  mov      rax, qword ptr [rbp - 0x30]
fe_frombytes       00001d30  mov      rcx, qword ptr [rbp - 0x78]
fe_frombytes       00001d34  add      rax, rcx
fe_frombytes       00001d37  mov      qword ptr [rbp - 0x30], rax
fe_frombytes       00001d3b  mov      rax, qword ptr [rbp - 0x78]
fe_frombytes       00001d3f  shl      rax, 0x1a
fe_frombytes       00001d43  mov      rcx, qword ptr [rbp - 0x28]
fe_frombytes       00001d47  sub      rcx, rax
fe_frombytes       00001d4a  mov      qword ptr [rbp - 0x28], rcx
fe_frombytes       00001d4e  mov      rax, qword ptr [rbp - 0x38]
fe_frombytes       00001d52  add      rax, 0x2000000
fe_frombytes       00001d59  sar      rax, 0x1a
fe_frombytes       00001d5d  mov      qword ptr [rbp - 0x88], rax
fe_frombytes       00001d64  mov      rax, qword ptr [rbp - 0x40]
fe_frombytes       00001d68  mov      rcx, qword ptr [rbp - 0x88]
fe_frombytes       00001d6f  add      rax, rcx
fe_frombytes       00001d72  mov      qword ptr [rbp - 0x40], rax
fe_frombytes       00001d76  mov      rax, qword ptr [rbp - 0x88]
fe_frombytes       00001d7d  shl      rax, 0x1a
fe_frombytes       00001d81  mov      rcx, qword ptr [rbp - 0x38]
fe_frombytes       00001d85  sub      rcx, rax
fe_frombytes       00001d88  mov      qword ptr [rbp - 0x38], rcx
fe_frombytes       00001d8c  mov      rax, qword ptr [rbp - 0x48]
fe_frombytes       00001d90  add      rax, 0x2000000
fe_frombytes       00001d97  sar      rax, 0x1a
fe_frombytes       00001d9b  mov      qword ptr [rbp - 0x98], rax
fe_frombytes       00001da2  mov      rax, qword ptr [rbp - 0x50]
fe_frombytes       00001da6  mov      rcx, qword ptr [rbp - 0x98]
fe_frombytes       00001dad  add      rax, rcx
fe_frombytes       00001db0  mov      qword ptr [rbp - 0x50], rax
fe_frombytes       00001db4  mov      rax, qword ptr [rbp - 0x98]
fe_frombytes       00001dbb  shl      rax, 0x1a
fe_frombytes       00001dbf  mov      rcx, qword ptr [rbp - 0x48]
fe_frombytes       00001dc3  sub      rcx, rax
fe_frombytes       00001dc6  mov      qword ptr [rbp - 0x48], rcx
fe_frombytes       00001dca  mov      rax, qword ptr [rbp + 0x10]
fe_frombytes       00001dce  mov      ecx, dword ptr [rbp - 8]
fe_frombytes       00001dd1  mov      dword ptr [rax], ecx
fe_frombytes       00001dd3  mov      rax, qword ptr [rbp + 0x10]
fe_frombytes       00001dd7  add      rax, 4
fe_frombytes       00001ddb  mov      ecx, dword ptr [rbp - 0x10]
fe_frombytes       00001dde  mov      dword ptr [rax], ecx
fe_frombytes       00001de0  mov      rax, qword ptr [rbp + 0x10]
fe_frombytes       00001de4  add      rax, 8
fe_frombytes       00001de8  mov      ecx, dword ptr [rbp - 0x18]
fe_frombytes       00001deb  mov      dword ptr [rax], ecx
fe_frombytes       00001ded  mov      rax, qword ptr [rbp + 0x10]
fe_frombytes       00001df1  add      rax, 0xc
fe_frombytes       00001df5  mov      ecx, dword ptr [rbp - 0x20]
fe_frombytes       00001df8  mov      dword ptr [rax], ecx
fe_frombytes       00001dfa  mov      rax, qword ptr [rbp + 0x10]
fe_frombytes       00001dfe  add      rax, 0x10
fe_frombytes       00001e02  mov      ecx, dword ptr [rbp - 0x28]
fe_frombytes       00001e05  mov      dword ptr [rax], ecx
fe_frombytes       00001e07  mov      rax, qword ptr [rbp + 0x10]
fe_frombytes       00001e0b  add      rax, 0x14
fe_frombytes       00001e0f  mov      ecx, dword ptr [rbp - 0x30]
fe_frombytes       00001e12  mov      dword ptr [rax], ecx
fe_frombytes       00001e14  mov      rax, qword ptr [rbp + 0x10]
fe_frombytes       00001e18  add      rax, 0x18
fe_frombytes       00001e1c  mov      ecx, dword ptr [rbp - 0x38]
fe_frombytes       00001e1f  mov      dword ptr [rax], ecx
fe_frombytes       00001e21  mov      rax, qword ptr [rbp + 0x10]
fe_frombytes       00001e25  add      rax, 0x1c
fe_frombytes       00001e29  mov      ecx, dword ptr [rbp - 0x40]
fe_frombytes       00001e2c  mov      dword ptr [rax], ecx
fe_frombytes       00001e2e  mov      rax, qword ptr [rbp + 0x10]
fe_frombytes       00001e32  add      rax, 0x20
fe_frombytes       00001e36  mov      ecx, dword ptr [rbp - 0x48]
fe_frombytes       00001e39  mov      dword ptr [rax], ecx
fe_frombytes       00001e3b  mov      rax, qword ptr [rbp + 0x10]
fe_frombytes       00001e3f  add      rax, 0x24
fe_frombytes       00001e43  mov      ecx, dword ptr [rbp - 0x50]
fe_frombytes       00001e46  mov      dword ptr [rax], ecx
fe_frombytes       00001e48  leave    
fe_frombytes       00001e49  ret      
fe_tobytes         00001e4a  push     rbp
fe_tobytes         00001e4b  mov      rbp, rsp
fe_tobytes         00001e4e  sub      rsp, 0x60
fe_tobytes         00001e55  mov      qword ptr [rbp + 0x10], rcx
fe_tobytes         00001e59  mov      qword ptr [rbp + 0x18], rdx
fe_tobytes         00001e5d  movabs   rax, 0x28
fe_tobytes         00001e67  mov      r8, rax
fe_tobytes         00001e6a  mov      rax, qword ptr [rbp + 0x18]
fe_tobytes         00001e6e  mov      r11, rax
fe_tobytes         00001e71  lea      rax, [rbp - 0x28]
fe_tobytes         00001e75  mov      r10, rax
fe_tobytes         00001e78  mov      rcx, r10
fe_tobytes         00001e7b  mov      rdx, r11
fe_tobytes         00001e7e  call     0x1e83
fe_tobytes         00001e83  mov      eax, dword ptr [rbp - 4]
fe_tobytes         00001e86  mov      ecx, 0x13
fe_tobytes         00001e8b  imul     eax, ecx
fe_tobytes         00001e8e  add      eax, 0x1000000
fe_tobytes         00001e94  sar      eax, 0x19
fe_tobytes         00001e97  mov      dword ptr [rbp - 0x2c], eax
fe_tobytes         00001e9a  mov      eax, dword ptr [rbp - 0x28]
fe_tobytes         00001e9d  mov      ecx, dword ptr [rbp - 0x2c]
fe_tobytes         00001ea0  add      eax, ecx
fe_tobytes         00001ea2  sar      eax, 0x1a
fe_tobytes         00001ea5  mov      dword ptr [rbp - 0x2c], eax
fe_tobytes         00001ea8  mov      eax, dword ptr [rbp - 0x24]
fe_tobytes         00001eab  mov      ecx, dword ptr [rbp - 0x2c]
fe_tobytes         00001eae  add      eax, ecx
fe_tobytes         00001eb0  sar      eax, 0x19
fe_tobytes         00001eb3  mov      dword ptr [rbp - 0x2c], eax
fe_tobytes         00001eb6  mov      eax, dword ptr [rbp - 0x20]
fe_tobytes         00001eb9  mov      ecx, dword ptr [rbp - 0x2c]
fe_tobytes         00001ebc  add      eax, ecx
fe_tobytes         00001ebe  sar      eax, 0x1a
fe_tobytes         00001ec1  mov      dword ptr [rbp - 0x2c], eax
fe_tobytes         00001ec4  mov      eax, dword ptr [rbp - 0x1c]
fe_tobytes         00001ec7  mov      ecx, dword ptr [rbp - 0x2c]
fe_tobytes         00001eca  add      eax, ecx
fe_tobytes         00001ecc  sar      eax, 0x19
fe_tobytes         00001ecf  mov      dword ptr [rbp - 0x2c], eax
fe_tobytes         00001ed2  mov      eax, dword ptr [rbp - 0x18]
fe_tobytes         00001ed5  mov      ecx, dword ptr [rbp - 0x2c]
fe_tobytes         00001ed8  add      eax, ecx
fe_tobytes         00001eda  sar      eax, 0x1a
fe_tobytes         00001edd  mov      dword ptr [rbp - 0x2c], eax
fe_tobytes         00001ee0  mov      eax, dword ptr [rbp - 0x14]
fe_tobytes         00001ee3  mov      ecx, dword ptr [rbp - 0x2c]
fe_tobytes         00001ee6  add      eax, ecx
fe_tobytes         00001ee8  sar      eax, 0x19
fe_tobytes         00001eeb  mov      dword ptr [rbp - 0x2c], eax
fe_tobytes         00001eee  mov      eax, dword ptr [rbp - 0x10]
fe_tobytes         00001ef1  mov      ecx, dword ptr [rbp - 0x2c]
fe_tobytes         00001ef4  add      eax, ecx
fe_tobytes         00001ef6  sar      eax, 0x1a
fe_tobytes         00001ef9  mov      dword ptr [rbp - 0x2c], eax
fe_tobytes         00001efc  mov      eax, dword ptr [rbp - 0xc]
fe_tobytes         00001eff  mov      ecx, dword ptr [rbp - 0x2c]
fe_tobytes         00001f02  add      eax, ecx
fe_tobytes         00001f04  sar      eax, 0x19
fe_tobytes         00001f07  mov      dword ptr [rbp - 0x2c], eax
fe_tobytes         00001f0a  mov      eax, dword ptr [rbp - 8]
fe_tobytes         00001f0d  mov      ecx, dword ptr [rbp - 0x2c]
fe_tobytes         00001f10  add      eax, ecx
fe_tobytes         00001f12  sar      eax, 0x1a
fe_tobytes         00001f15  mov      dword ptr [rbp - 0x2c], eax
fe_tobytes         00001f18  mov      eax, dword ptr [rbp - 4]
fe_tobytes         00001f1b  mov      ecx, dword ptr [rbp - 0x2c]
fe_tobytes         00001f1e  add      eax, ecx
fe_tobytes         00001f20  sar      eax, 0x19
fe_tobytes         00001f23  mov      dword ptr [rbp - 0x2c], eax
fe_tobytes         00001f26  mov      eax, dword ptr [rbp - 0x2c]
fe_tobytes         00001f29  mov      ecx, 0x13
fe_tobytes         00001f2e  imul     eax, ecx
fe_tobytes         00001f31  mov      ecx, dword ptr [rbp - 0x28]
fe_tobytes         00001f34  add      ecx, eax
fe_tobytes         00001f36  mov      dword ptr [rbp - 0x28], ecx
fe_tobytes         00001f39  mov      eax, dword ptr [rbp - 0x28]
fe_tobytes         00001f3c  sar      eax, 0x1a
fe_tobytes         00001f3f  mov      dword ptr [rbp - 0x30], eax
fe_tobytes         00001f42  mov      eax, dword ptr [rbp - 0x24]
fe_tobytes         00001f45  mov      ecx, dword ptr [rbp - 0x30]
fe_tobytes         00001f48  add      eax, ecx
fe_tobytes         00001f4a  mov      dword ptr [rbp - 0x24], eax
fe_tobytes         00001f4d  mov      eax, dword ptr [rbp - 0x30]
fe_tobytes         00001f50  shl      eax, 0x1a
fe_tobytes         00001f53  mov      ecx, dword ptr [rbp - 0x28]
fe_tobytes         00001f56  sub      ecx, eax
fe_tobytes         00001f58  mov      dword ptr [rbp - 0x28], ecx
fe_tobytes         00001f5b  mov      eax, dword ptr [rbp - 0x24]
fe_tobytes         00001f5e  sar      eax, 0x19
fe_tobytes         00001f61  mov      dword ptr [rbp - 0x30], eax
fe_tobytes         00001f64  mov      eax, dword ptr [rbp - 0x20]
fe_tobytes         00001f67  mov      ecx, dword ptr [rbp - 0x30]
fe_tobytes         00001f6a  add      eax, ecx
fe_tobytes         00001f6c  mov      dword ptr [rbp - 0x20], eax
fe_tobytes         00001f6f  mov      eax, dword ptr [rbp - 0x30]
fe_tobytes         00001f72  shl      eax, 0x19
fe_tobytes         00001f75  mov      ecx, dword ptr [rbp - 0x24]
fe_tobytes         00001f78  sub      ecx, eax
fe_tobytes         00001f7a  mov      dword ptr [rbp - 0x24], ecx
fe_tobytes         00001f7d  mov      eax, dword ptr [rbp - 0x20]
fe_tobytes         00001f80  sar      eax, 0x1a
fe_tobytes         00001f83  mov      dword ptr [rbp - 0x30], eax
fe_tobytes         00001f86  mov      eax, dword ptr [rbp - 0x1c]
fe_tobytes         00001f89  mov      ecx, dword ptr [rbp - 0x30]
fe_tobytes         00001f8c  add      eax, ecx
fe_tobytes         00001f8e  mov      dword ptr [rbp - 0x1c], eax
fe_tobytes         00001f91  mov      eax, dword ptr [rbp - 0x30]
fe_tobytes         00001f94  shl      eax, 0x1a
fe_tobytes         00001f97  mov      ecx, dword ptr [rbp - 0x20]
fe_tobytes         00001f9a  sub      ecx, eax
fe_tobytes         00001f9c  mov      dword ptr [rbp - 0x20], ecx
fe_tobytes         00001f9f  mov      eax, dword ptr [rbp - 0x1c]
fe_tobytes         00001fa2  sar      eax, 0x19
fe_tobytes         00001fa5  mov      dword ptr [rbp - 0x30], eax
fe_tobytes         00001fa8  mov      eax, dword ptr [rbp - 0x18]
fe_tobytes         00001fab  mov      ecx, dword ptr [rbp - 0x30]
fe_tobytes         00001fae  add      eax, ecx
fe_tobytes         00001fb0  mov      dword ptr [rbp - 0x18], eax
fe_tobytes         00001fb3  mov      eax, dword ptr [rbp - 0x30]
fe_tobytes         00001fb6  shl      eax, 0x19
fe_tobytes         00001fb9  mov      ecx, dword ptr [rbp - 0x1c]
fe_tobytes         00001fbc  sub      ecx, eax
fe_tobytes         00001fbe  mov      dword ptr [rbp - 0x1c], ecx
fe_tobytes         00001fc1  mov      eax, dword ptr [rbp - 0x18]
fe_tobytes         00001fc4  sar      eax, 0x1a
fe_tobytes         00001fc7  mov      dword ptr [rbp - 0x30], eax
fe_tobytes         00001fca  mov      eax, dword ptr [rbp - 0x14]
fe_tobytes         00001fcd  mov      ecx, dword ptr [rbp - 0x30]
fe_tobytes         00001fd0  add      eax, ecx
fe_tobytes         00001fd2  mov      dword ptr [rbp - 0x14], eax
fe_tobytes         00001fd5  mov      eax, dword ptr [rbp - 0x30]
fe_tobytes         00001fd8  shl      eax, 0x1a
fe_tobytes         00001fdb  mov      ecx, dword ptr [rbp - 0x18]
fe_tobytes         00001fde  sub      ecx, eax
fe_tobytes         00001fe0  mov      dword ptr [rbp - 0x18], ecx
fe_tobytes         00001fe3  mov      eax, dword ptr [rbp - 0x14]
fe_tobytes         00001fe6  sar      eax, 0x19
fe_tobytes         00001fe9  mov      dword ptr [rbp - 0x30], eax
fe_tobytes         00001fec  mov      eax, dword ptr [rbp - 0x10]
fe_tobytes         00001fef  mov      ecx, dword ptr [rbp - 0x30]
fe_tobytes         00001ff2  add      eax, ecx
fe_tobytes         00001ff4  mov      dword ptr [rbp - 0x10], eax
fe_tobytes         00001ff7  mov      eax, dword ptr [rbp - 0x30]
fe_tobytes         00001ffa  shl      eax, 0x19
fe_tobytes         00001ffd  mov      ecx, dword ptr [rbp - 0x14]
fe_tobytes         00002000  sub      ecx, eax
fe_tobytes         00002002  mov      dword ptr [rbp - 0x14], ecx
fe_tobytes         00002005  mov      eax, dword ptr [rbp - 0x10]
fe_tobytes         00002008  sar      eax, 0x1a
fe_tobytes         0000200b  mov      dword ptr [rbp - 0x30], eax
fe_tobytes         0000200e  mov      eax, dword ptr [rbp - 0xc]
fe_tobytes         00002011  mov      ecx, dword ptr [rbp - 0x30]
fe_tobytes         00002014  add      eax, ecx
fe_tobytes         00002016  mov      dword ptr [rbp - 0xc], eax
fe_tobytes         00002019  mov      eax, dword ptr [rbp - 0x30]
fe_tobytes         0000201c  shl      eax, 0x1a
fe_tobytes         0000201f  mov      ecx, dword ptr [rbp - 0x10]
fe_tobytes         00002022  sub      ecx, eax
fe_tobytes         00002024  mov      dword ptr [rbp - 0x10], ecx
fe_tobytes         00002027  mov      eax, dword ptr [rbp - 0xc]
fe_tobytes         0000202a  sar      eax, 0x19
fe_tobytes         0000202d  mov      dword ptr [rbp - 0x30], eax
fe_tobytes         00002030  mov      eax, dword ptr [rbp - 8]
fe_tobytes         00002033  mov      ecx, dword ptr [rbp - 0x30]
fe_tobytes         00002036  add      eax, ecx
fe_tobytes         00002038  mov      dword ptr [rbp - 8], eax
fe_tobytes         0000203b  mov      eax, dword ptr [rbp - 0x30]
fe_tobytes         0000203e  shl      eax, 0x19
fe_tobytes         00002041  mov      ecx, dword ptr [rbp - 0xc]
fe_tobytes         00002044  sub      ecx, eax
fe_tobytes         00002046  mov      dword ptr [rbp - 0xc], ecx
fe_tobytes         00002049  mov      eax, dword ptr [rbp - 8]
fe_tobytes         0000204c  sar      eax, 0x1a
fe_tobytes         0000204f  mov      dword ptr [rbp - 0x30], eax
fe_tobytes         00002052  mov      eax, dword ptr [rbp - 4]
fe_tobytes         00002055  mov      ecx, dword ptr [rbp - 0x30]
fe_tobytes         00002058  add      eax, ecx
fe_tobytes         0000205a  mov      dword ptr [rbp - 4], eax
fe_tobytes         0000205d  mov      eax, dword ptr [rbp - 0x30]
fe_tobytes         00002060  shl      eax, 0x1a
fe_tobytes         00002063  mov      ecx, dword ptr [rbp - 8]
fe_tobytes         00002066  sub      ecx, eax
fe_tobytes         00002068  mov      dword ptr [rbp - 8], ecx
fe_tobytes         0000206b  mov      eax, dword ptr [rbp - 4]
fe_tobytes         0000206e  sar      eax, 0x19
fe_tobytes         00002071  mov      dword ptr [rbp - 0x30], eax
fe_tobytes         00002074  mov      eax, dword ptr [rbp - 0x30]
fe_tobytes         00002077  shl      eax, 0x19
fe_tobytes         0000207a  mov      ecx, dword ptr [rbp - 4]
fe_tobytes         0000207d  sub      ecx, eax
fe_tobytes         0000207f  mov      dword ptr [rbp - 4], ecx
fe_tobytes         00002082  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         00002086  mov      ecx, dword ptr [rbp - 0x28]
fe_tobytes         00002089  and      ecx, 0xff
fe_tobytes         0000208f  mov      byte ptr [rax], cl
fe_tobytes         00002091  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         00002095  add      rax, 1
fe_tobytes         00002099  mov      ecx, dword ptr [rbp - 0x28]
fe_tobytes         0000209c  sar      ecx, 8
fe_tobytes         0000209f  and      ecx, 0xff
fe_tobytes         000020a5  mov      byte ptr [rax], cl
fe_tobytes         000020a7  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         000020ab  add      rax, 2
fe_tobytes         000020af  mov      ecx, dword ptr [rbp - 0x28]
fe_tobytes         000020b2  sar      ecx, 0x10
fe_tobytes         000020b5  and      ecx, 0xff
fe_tobytes         000020bb  mov      byte ptr [rax], cl
fe_tobytes         000020bd  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         000020c1  add      rax, 3
fe_tobytes         000020c5  mov      ecx, dword ptr [rbp - 0x28]
fe_tobytes         000020c8  sar      ecx, 0x18
fe_tobytes         000020cb  mov      edx, dword ptr [rbp - 0x24]
fe_tobytes         000020ce  shl      edx, 2
fe_tobytes         000020d1  or       ecx, edx
fe_tobytes         000020d3  and      ecx, 0xff
fe_tobytes         000020d9  mov      byte ptr [rax], cl
fe_tobytes         000020db  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         000020df  add      rax, 4
fe_tobytes         000020e3  mov      ecx, dword ptr [rbp - 0x24]
fe_tobytes         000020e6  sar      ecx, 6
fe_tobytes         000020e9  and      ecx, 0xff
fe_tobytes         000020ef  mov      byte ptr [rax], cl
fe_tobytes         000020f1  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         000020f5  add      rax, 5
fe_tobytes         000020f9  mov      ecx, dword ptr [rbp - 0x24]
fe_tobytes         000020fc  sar      ecx, 0xe
fe_tobytes         000020ff  and      ecx, 0xff
fe_tobytes         00002105  mov      byte ptr [rax], cl
fe_tobytes         00002107  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         0000210b  add      rax, 6
fe_tobytes         0000210f  mov      ecx, dword ptr [rbp - 0x24]
fe_tobytes         00002112  sar      ecx, 0x16
fe_tobytes         00002115  mov      edx, dword ptr [rbp - 0x20]
fe_tobytes         00002118  shl      edx, 3
fe_tobytes         0000211b  or       ecx, edx
fe_tobytes         0000211d  and      ecx, 0xff
fe_tobytes         00002123  mov      byte ptr [rax], cl
fe_tobytes         00002125  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         00002129  add      rax, 7
fe_tobytes         0000212d  mov      ecx, dword ptr [rbp - 0x20]
fe_tobytes         00002130  sar      ecx, 5
fe_tobytes         00002133  and      ecx, 0xff
fe_tobytes         00002139  mov      byte ptr [rax], cl
fe_tobytes         0000213b  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         0000213f  add      rax, 8
fe_tobytes         00002143  mov      ecx, dword ptr [rbp - 0x20]
fe_tobytes         00002146  sar      ecx, 0xd
fe_tobytes         00002149  and      ecx, 0xff
fe_tobytes         0000214f  mov      byte ptr [rax], cl
fe_tobytes         00002151  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         00002155  add      rax, 9
fe_tobytes         00002159  mov      ecx, dword ptr [rbp - 0x20]
fe_tobytes         0000215c  sar      ecx, 0x15
fe_tobytes         0000215f  mov      edx, dword ptr [rbp - 0x1c]
fe_tobytes         00002162  shl      edx, 5
fe_tobytes         00002165  or       ecx, edx
fe_tobytes         00002167  and      ecx, 0xff
fe_tobytes         0000216d  mov      byte ptr [rax], cl
fe_tobytes         0000216f  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         00002173  add      rax, 0xa
fe_tobytes         00002177  mov      ecx, dword ptr [rbp - 0x1c]
fe_tobytes         0000217a  sar      ecx, 3
fe_tobytes         0000217d  and      ecx, 0xff
fe_tobytes         00002183  mov      byte ptr [rax], cl
fe_tobytes         00002185  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         00002189  add      rax, 0xb
fe_tobytes         0000218d  mov      ecx, dword ptr [rbp - 0x1c]
fe_tobytes         00002190  sar      ecx, 0xb
fe_tobytes         00002193  and      ecx, 0xff
fe_tobytes         00002199  mov      byte ptr [rax], cl
fe_tobytes         0000219b  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         0000219f  add      rax, 0xc
fe_tobytes         000021a3  mov      ecx, dword ptr [rbp - 0x1c]
fe_tobytes         000021a6  sar      ecx, 0x13
fe_tobytes         000021a9  mov      edx, dword ptr [rbp - 0x18]
fe_tobytes         000021ac  shl      edx, 6
fe_tobytes         000021af  or       ecx, edx
fe_tobytes         000021b1  and      ecx, 0xff
fe_tobytes         000021b7  mov      byte ptr [rax], cl
fe_tobytes         000021b9  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         000021bd  add      rax, 0xd
fe_tobytes         000021c1  mov      ecx, dword ptr [rbp - 0x18]
fe_tobytes         000021c4  sar      ecx, 2
fe_tobytes         000021c7  and      ecx, 0xff
fe_tobytes         000021cd  mov      byte ptr [rax], cl
fe_tobytes         000021cf  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         000021d3  add      rax, 0xe
fe_tobytes         000021d7  mov      ecx, dword ptr [rbp - 0x18]
fe_tobytes         000021da  sar      ecx, 0xa
fe_tobytes         000021dd  and      ecx, 0xff
fe_tobytes         000021e3  mov      byte ptr [rax], cl
fe_tobytes         000021e5  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         000021e9  add      rax, 0xf
fe_tobytes         000021ed  mov      ecx, dword ptr [rbp - 0x18]
fe_tobytes         000021f0  sar      ecx, 0x12
fe_tobytes         000021f3  and      ecx, 0xff
fe_tobytes         000021f9  mov      byte ptr [rax], cl
fe_tobytes         000021fb  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         000021ff  add      rax, 0x10
fe_tobytes         00002203  mov      ecx, dword ptr [rbp - 0x14]
fe_tobytes         00002206  and      ecx, 0xff
fe_tobytes         0000220c  mov      byte ptr [rax], cl
fe_tobytes         0000220e  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         00002212  add      rax, 0x11
fe_tobytes         00002216  mov      ecx, dword ptr [rbp - 0x14]
fe_tobytes         00002219  sar      ecx, 8
fe_tobytes         0000221c  and      ecx, 0xff
fe_tobytes         00002222  mov      byte ptr [rax], cl
fe_tobytes         00002224  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         00002228  add      rax, 0x12
fe_tobytes         0000222c  mov      ecx, dword ptr [rbp - 0x14]
fe_tobytes         0000222f  sar      ecx, 0x10
fe_tobytes         00002232  and      ecx, 0xff
fe_tobytes         00002238  mov      byte ptr [rax], cl
fe_tobytes         0000223a  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         0000223e  add      rax, 0x13
fe_tobytes         00002242  mov      ecx, dword ptr [rbp - 0x14]
fe_tobytes         00002245  sar      ecx, 0x18
fe_tobytes         00002248  mov      edx, dword ptr [rbp - 0x10]
fe_tobytes         0000224b  shl      edx, 1
fe_tobytes         0000224e  or       ecx, edx
fe_tobytes         00002250  and      ecx, 0xff
fe_tobytes         00002256  mov      byte ptr [rax], cl
fe_tobytes         00002258  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         0000225c  add      rax, 0x14
fe_tobytes         00002260  mov      ecx, dword ptr [rbp - 0x10]
fe_tobytes         00002263  sar      ecx, 7
fe_tobytes         00002266  and      ecx, 0xff
fe_tobytes         0000226c  mov      byte ptr [rax], cl
fe_tobytes         0000226e  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         00002272  add      rax, 0x15
fe_tobytes         00002276  mov      ecx, dword ptr [rbp - 0x10]
fe_tobytes         00002279  sar      ecx, 0xf
fe_tobytes         0000227c  and      ecx, 0xff
fe_tobytes         00002282  mov      byte ptr [rax], cl
fe_tobytes         00002284  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         00002288  add      rax, 0x16
fe_tobytes         0000228c  mov      ecx, dword ptr [rbp - 0x10]
fe_tobytes         0000228f  sar      ecx, 0x17
fe_tobytes         00002292  mov      edx, dword ptr [rbp - 0xc]
fe_tobytes         00002295  shl      edx, 3
fe_tobytes         00002298  or       ecx, edx
fe_tobytes         0000229a  and      ecx, 0xff
fe_tobytes         000022a0  mov      byte ptr [rax], cl
fe_tobytes         000022a2  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         000022a6  add      rax, 0x17
fe_tobytes         000022aa  mov      ecx, dword ptr [rbp - 0xc]
fe_tobytes         000022ad  sar      ecx, 5
fe_tobytes         000022b0  and      ecx, 0xff
fe_tobytes         000022b6  mov      byte ptr [rax], cl
fe_tobytes         000022b8  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         000022bc  add      rax, 0x18
fe_tobytes         000022c0  mov      ecx, dword ptr [rbp - 0xc]
fe_tobytes         000022c3  sar      ecx, 0xd
fe_tobytes         000022c6  and      ecx, 0xff
fe_tobytes         000022cc  mov      byte ptr [rax], cl
fe_tobytes         000022ce  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         000022d2  add      rax, 0x19
fe_tobytes         000022d6  mov      ecx, dword ptr [rbp - 0xc]
fe_tobytes         000022d9  sar      ecx, 0x15
fe_tobytes         000022dc  mov      edx, dword ptr [rbp - 8]
fe_tobytes         000022df  shl      edx, 4
fe_tobytes         000022e2  or       ecx, edx
fe_tobytes         000022e4  and      ecx, 0xff
fe_tobytes         000022ea  mov      byte ptr [rax], cl
fe_tobytes         000022ec  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         000022f0  add      rax, 0x1a
fe_tobytes         000022f4  mov      ecx, dword ptr [rbp - 8]
fe_tobytes         000022f7  sar      ecx, 4
fe_tobytes         000022fa  and      ecx, 0xff
fe_tobytes         00002300  mov      byte ptr [rax], cl
fe_tobytes         00002302  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         00002306  add      rax, 0x1b
fe_tobytes         0000230a  mov      ecx, dword ptr [rbp - 8]
fe_tobytes         0000230d  sar      ecx, 0xc
fe_tobytes         00002310  and      ecx, 0xff
fe_tobytes         00002316  mov      byte ptr [rax], cl
fe_tobytes         00002318  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         0000231c  add      rax, 0x1c
fe_tobytes         00002320  mov      ecx, dword ptr [rbp - 8]
fe_tobytes         00002323  sar      ecx, 0x14
fe_tobytes         00002326  mov      edx, dword ptr [rbp - 4]
fe_tobytes         00002329  shl      edx, 6
fe_tobytes         0000232c  or       ecx, edx
fe_tobytes         0000232e  and      ecx, 0xff
fe_tobytes         00002334  mov      byte ptr [rax], cl
fe_tobytes         00002336  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         0000233a  add      rax, 0x1d
fe_tobytes         0000233e  mov      ecx, dword ptr [rbp - 4]
fe_tobytes         00002341  sar      ecx, 2
fe_tobytes         00002344  and      ecx, 0xff
fe_tobytes         0000234a  mov      byte ptr [rax], cl
fe_tobytes         0000234c  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         00002350  add      rax, 0x1e
fe_tobytes         00002354  mov      ecx, dword ptr [rbp - 4]
fe_tobytes         00002357  sar      ecx, 0xa
fe_tobytes         0000235a  and      ecx, 0xff
fe_tobytes         00002360  mov      byte ptr [rax], cl
fe_tobytes         00002362  mov      rax, qword ptr [rbp + 0x10]
fe_tobytes         00002366  add      rax, 0x1f
fe_tobytes         0000236a  mov      ecx, dword ptr [rbp - 4]
fe_tobytes         0000236d  sar      ecx, 0x12
fe_tobytes         00002370  and      ecx, 0xff
fe_tobytes         00002376  mov      byte ptr [rax], cl
fe_tobytes         00002378  leave    
fe_tobytes         00002379  ret      
x25519_scalarmult  0000237a  push     rbp
x25519_scalarmult  0000237b  mov      rbp, rsp
x25519_scalarmult  0000237e  sub      rsp, 0x170
x25519_scalarmult  00002385  mov      qword ptr [rbp + 0x10], rcx
x25519_scalarmult  00002389  mov      qword ptr [rbp + 0x18], rdx
x25519_scalarmult  0000238d  mov      qword ptr [rbp + 0x20], r8
x25519_scalarmult  00002391  mov      eax, 0
x25519_scalarmult  00002396  mov      dword ptr [rbp - 0x140], eax
x25519_scalarmult  0000239c  movabs   rax, 0x20
x25519_scalarmult  000023a6  mov      r8, rax
x25519_scalarmult  000023a9  mov      rax, qword ptr [rbp + 0x18]
x25519_scalarmult  000023ad  mov      r11, rax
x25519_scalarmult  000023b0  lea      rax, [rbp - 0x20]
x25519_scalarmult  000023b4  mov      r10, rax
x25519_scalarmult  000023b7  mov      rcx, r10
x25519_scalarmult  000023ba  mov      rdx, r11
x25519_scalarmult  000023bd  call     0x23c2
x25519_scalarmult  000023c2  movzx    eax, byte ptr [rbp - 0x20]
x25519_scalarmult  000023c6  and      eax, 0xf8
x25519_scalarmult  000023cc  mov      byte ptr [rbp - 0x20], al
x25519_scalarmult  000023cf  movzx    eax, byte ptr [rbp - 1]
x25519_scalarmult  000023d3  and      eax, 0x7f
x25519_scalarmult  000023d6  mov      byte ptr [rbp - 1], al
x25519_scalarmult  000023d9  movzx    eax, byte ptr [rbp - 1]
x25519_scalarmult  000023dd  or       eax, 0x40
x25519_scalarmult  000023e0  mov      byte ptr [rbp - 1], al
x25519_scalarmult  000023e3  mov      rax, qword ptr [rbp + 0x20]
x25519_scalarmult  000023e7  mov      r11, rax
x25519_scalarmult  000023ea  lea      rax, [rbp - 0x48]
x25519_scalarmult  000023ee  mov      r10, rax
x25519_scalarmult  000023f1  mov      rcx, r10
x25519_scalarmult  000023f4  mov      rdx, r11
x25519_scalarmult  000023f7  call     0x23fc
x25519_scalarmult  000023fc  lea      rax, [rbp - 0x70]
x25519_scalarmult  00002400  mov      r10, rax
x25519_scalarmult  00002403  mov      rcx, r10
x25519_scalarmult  00002406  call     0x240b
x25519_scalarmult  0000240b  lea      rax, [rbp - 0x98]
x25519_scalarmult  00002412  mov      r10, rax
x25519_scalarmult  00002415  mov      rcx, r10
x25519_scalarmult  00002418  call     0x241d
x25519_scalarmult  0000241d  lea      rax, [rbp - 0x48]
x25519_scalarmult  00002421  mov      r11, rax
x25519_scalarmult  00002424  lea      rax, [rbp - 0xc0]
x25519_scalarmult  0000242b  mov      r10, rax
x25519_scalarmult  0000242e  mov      rcx, r10
x25519_scalarmult  00002431  mov      rdx, r11
x25519_scalarmult  00002434  call     0x2439
x25519_scalarmult  00002439  lea      rax, [rbp - 0xe8]
x25519_scalarmult  00002440  mov      r10, rax
x25519_scalarmult  00002443  mov      rcx, r10
x25519_scalarmult  00002446  call     0x244b
x25519_scalarmult  0000244b  mov      eax, 0xfe
x25519_scalarmult  00002450  mov      dword ptr [rbp - 0x13c], eax
x25519_scalarmult  00002456  mov      eax, dword ptr [rbp - 0x13c]
x25519_scalarmult  0000245c  cmp      eax, 0
x25519_scalarmult  0000245f  jl       0x27b8
x25519_scalarmult  00002465  jmp      0x247e
x25519_scalarmult  0000246a  mov      eax, dword ptr [rbp - 0x13c]
x25519_scalarmult  00002470  mov      rcx, rax
x25519_scalarmult  00002473  add      eax, -1
x25519_scalarmult  00002476  mov      dword ptr [rbp - 0x13c], eax
x25519_scalarmult  0000247c  jmp      0x2456
x25519_scalarmult  0000247e  mov      eax, dword ptr [rbp - 0x13c]
x25519_scalarmult  00002484  sar      eax, 3
x25519_scalarmult  00002487  movsxd   rax, eax
x25519_scalarmult  0000248a  lea      rcx, [rbp - 0x20]
x25519_scalarmult  0000248e  add      rcx, rax
x25519_scalarmult  00002491  mov      eax, dword ptr [rbp - 0x13c]
x25519_scalarmult  00002497  and      eax, 7
x25519_scalarmult  0000249a  movzx    edx, byte ptr [rcx]
x25519_scalarmult  0000249d  mov      rcx, rax
x25519_scalarmult  000024a0  sar      edx, cl
x25519_scalarmult  000024a2  and      edx, 1
x25519_scalarmult  000024a5  mov      dword ptr [rbp - 0x144], edx
x25519_scalarmult  000024ab  mov      eax, dword ptr [rbp - 0x140]
x25519_scalarmult  000024b1  mov      ecx, dword ptr [rbp - 0x144]
x25519_scalarmult  000024b7  xor      eax, ecx
x25519_scalarmult  000024b9  mov      dword ptr [rbp - 0x140], eax
x25519_scalarmult  000024bf  mov      eax, dword ptr [rbp - 0x140]
x25519_scalarmult  000024c5  mov      r8, rax
x25519_scalarmult  000024c8  lea      rax, [rbp - 0xc0]
x25519_scalarmult  000024cf  mov      r11, rax
x25519_scalarmult  000024d2  lea      rax, [rbp - 0x70]
x25519_scalarmult  000024d6  mov      r10, rax
x25519_scalarmult  000024d9  mov      rcx, r10
x25519_scalarmult  000024dc  mov      rdx, r11
x25519_scalarmult  000024df  call     0x24e4
x25519_scalarmult  000024e4  mov      eax, dword ptr [rbp - 0x140]
x25519_scalarmult  000024ea  mov      r8, rax
x25519_scalarmult  000024ed  lea      rax, [rbp - 0xe8]
x25519_scalarmult  000024f4  mov      r11, rax
x25519_scalarmult  000024f7  lea      rax, [rbp - 0x98]
x25519_scalarmult  000024fe  mov      r10, rax
x25519_scalarmult  00002501  mov      rcx, r10
x25519_scalarmult  00002504  mov      rdx, r11
x25519_scalarmult  00002507  call     0x250c
x25519_scalarmult  0000250c  mov      eax, dword ptr [rbp - 0x144]
x25519_scalarmult  00002512  mov      dword ptr [rbp - 0x140], eax
x25519_scalarmult  00002518  lea      rax, [rbp - 0xe8]
x25519_scalarmult  0000251f  mov      r8, rax
x25519_scalarmult  00002522  lea      rax, [rbp - 0xc0]
x25519_scalarmult  00002529  mov      r11, rax
x25519_scalarmult  0000252c  lea      rax, [rbp - 0x110]
x25519_scalarmult  00002533  mov      r10, rax
x25519_scalarmult  00002536  mov      rcx, r10
x25519_scalarmult  00002539  mov      rdx, r11
x25519_scalarmult  0000253c  call     0x2541
x25519_scalarmult  00002541  lea      rax, [rbp - 0x98]
x25519_scalarmult  00002548  mov      r8, rax
x25519_scalarmult  0000254b  lea      rax, [rbp - 0x70]
x25519_scalarmult  0000254f  mov      r11, rax
x25519_scalarmult  00002552  lea      rax, [rbp - 0x138]
x25519_scalarmult  00002559  mov      r10, rax
x25519_scalarmult  0000255c  mov      rcx, r10
x25519_scalarmult  0000255f  mov      rdx, r11
x25519_scalarmult  00002562  call     0x2567
x25519_scalarmult  00002567  lea      rax, [rbp - 0x98]
x25519_scalarmult  0000256e  mov      r8, rax
x25519_scalarmult  00002571  lea      rax, [rbp - 0x70]
x25519_scalarmult  00002575  mov      r11, rax
x25519_scalarmult  00002578  lea      rax, [rbp - 0x70]
x25519_scalarmult  0000257c  mov      r10, rax
x25519_scalarmult  0000257f  mov      rcx, r10
x25519_scalarmult  00002582  mov      rdx, r11
x25519_scalarmult  00002585  call     0x258a
x25519_scalarmult  0000258a  lea      rax, [rbp - 0xe8]
x25519_scalarmult  00002591  mov      r8, rax
x25519_scalarmult  00002594  lea      rax, [rbp - 0xc0]
x25519_scalarmult  0000259b  mov      r11, rax
x25519_scalarmult  0000259e  lea      rax, [rbp - 0x98]
x25519_scalarmult  000025a5  mov      r10, rax
x25519_scalarmult  000025a8  mov      rcx, r10
x25519_scalarmult  000025ab  mov      rdx, r11
x25519_scalarmult  000025ae  call     0x25b3
x25519_scalarmult  000025b3  lea      rax, [rbp - 0x70]
x25519_scalarmult  000025b7  mov      r8, rax
x25519_scalarmult  000025ba  lea      rax, [rbp - 0x110]
x25519_scalarmult  000025c1  mov      r11, rax
x25519_scalarmult  000025c4  lea      rax, [rbp - 0xe8]
x25519_scalarmult  000025cb  mov      r10, rax
x25519_scalarmult  000025ce  mov      rcx, r10
x25519_scalarmult  000025d1  mov      rdx, r11
x25519_scalarmult  000025d4  call     0x25d9
x25519_scalarmult  000025d9  lea      rax, [rbp - 0x138]
x25519_scalarmult  000025e0  mov      r8, rax
x25519_scalarmult  000025e3  lea      rax, [rbp - 0x98]
x25519_scalarmult  000025ea  mov      r11, rax
x25519_scalarmult  000025ed  lea      rax, [rbp - 0x98]
x25519_scalarmult  000025f4  mov      r10, rax
x25519_scalarmult  000025f7  mov      rcx, r10
x25519_scalarmult  000025fa  mov      rdx, r11
x25519_scalarmult  000025fd  call     0x2602
x25519_scalarmult  00002602  lea      rax, [rbp - 0x138]
x25519_scalarmult  00002609  mov      r11, rax
x25519_scalarmult  0000260c  lea      rax, [rbp - 0x110]
x25519_scalarmult  00002613  mov      r10, rax
x25519_scalarmult  00002616  mov      rcx, r10
x25519_scalarmult  00002619  mov      rdx, r11
x25519_scalarmult  0000261c  call     0x2621
x25519_scalarmult  00002621  lea      rax, [rbp - 0x70]
x25519_scalarmult  00002625  mov      r11, rax
x25519_scalarmult  00002628  lea      rax, [rbp - 0x138]
x25519_scalarmult  0000262f  mov      r10, rax
x25519_scalarmult  00002632  mov      rcx, r10
x25519_scalarmult  00002635  mov      rdx, r11
x25519_scalarmult  00002638  call     0x263d
x25519_scalarmult  0000263d  lea      rax, [rbp - 0x98]
x25519_scalarmult  00002644  mov      r8, rax
x25519_scalarmult  00002647  lea      rax, [rbp - 0xe8]
x25519_scalarmult  0000264e  mov      r11, rax
x25519_scalarmult  00002651  lea      rax, [rbp - 0xc0]
x25519_scalarmult  00002658  mov      r10, rax
x25519_scalarmult  0000265b  mov      rcx, r10
x25519_scalarmult  0000265e  mov      rdx, r11
x25519_scalarmult  00002661  call     0x2666
x25519_scalarmult  00002666  lea      rax, [rbp - 0x98]
x25519_scalarmult  0000266d  mov      r8, rax
x25519_scalarmult  00002670  lea      rax, [rbp - 0xe8]
x25519_scalarmult  00002677  mov      r11, rax
x25519_scalarmult  0000267a  lea      rax, [rbp - 0x98]
x25519_scalarmult  00002681  mov      r10, rax
x25519_scalarmult  00002684  mov      rcx, r10
x25519_scalarmult  00002687  mov      rdx, r11
x25519_scalarmult  0000268a  call     0x268f
x25519_scalarmult  0000268f  lea      rax, [rbp - 0x110]
x25519_scalarmult  00002696  mov      r8, rax
x25519_scalarmult  00002699  lea      rax, [rbp - 0x138]
x25519_scalarmult  000026a0  mov      r11, rax
x25519_scalarmult  000026a3  lea      rax, [rbp - 0x70]
x25519_scalarmult  000026a7  mov      r10, rax
x25519_scalarmult  000026aa  mov      rcx, r10
x25519_scalarmult  000026ad  mov      rdx, r11
x25519_scalarmult  000026b0  call     0x26b5
x25519_scalarmult  000026b5  lea      rax, [rbp - 0x110]
x25519_scalarmult  000026bc  mov      r8, rax
x25519_scalarmult  000026bf  lea      rax, [rbp - 0x138]
x25519_scalarmult  000026c6  mov      r11, rax
x25519_scalarmult  000026c9  lea      rax, [rbp - 0x138]
x25519_scalarmult  000026d0  mov      r10, rax
x25519_scalarmult  000026d3  mov      rcx, r10
x25519_scalarmult  000026d6  mov      rdx, r11
x25519_scalarmult  000026d9  call     0x26de
x25519_scalarmult  000026de  lea      rax, [rbp - 0x98]
x25519_scalarmult  000026e5  mov      r11, rax
x25519_scalarmult  000026e8  lea      rax, [rbp - 0x98]
x25519_scalarmult  000026ef  mov      r10, rax
x25519_scalarmult  000026f2  mov      rcx, r10
x25519_scalarmult  000026f5  mov      rdx, r11
x25519_scalarmult  000026f8  call     0x26fd
x25519_scalarmult  000026fd  lea      rax, [rbp - 0x138]
x25519_scalarmult  00002704  mov      r11, rax
x25519_scalarmult  00002707  lea      rax, [rbp - 0xe8]
x25519_scalarmult  0000270e  mov      r10, rax
x25519_scalarmult  00002711  mov      rcx, r10
x25519_scalarmult  00002714  mov      rdx, r11
x25519_scalarmult  00002717  call     0x271c
x25519_scalarmult  0000271c  lea      rax, [rbp - 0xc0]
x25519_scalarmult  00002723  mov      r11, rax
x25519_scalarmult  00002726  lea      rax, [rbp - 0xc0]
x25519_scalarmult  0000272d  mov      r10, rax
x25519_scalarmult  00002730  mov      rcx, r10
x25519_scalarmult  00002733  mov      rdx, r11
x25519_scalarmult  00002736  call     0x273b
x25519_scalarmult  0000273b  lea      rax, [rbp - 0xe8]
x25519_scalarmult  00002742  mov      r8, rax
x25519_scalarmult  00002745  lea      rax, [rbp - 0x110]
x25519_scalarmult  0000274c  mov      r11, rax
x25519_scalarmult  0000274f  lea      rax, [rbp - 0x110]
x25519_scalarmult  00002756  mov      r10, rax
x25519_scalarmult  00002759  mov      rcx, r10
x25519_scalarmult  0000275c  mov      rdx, r11
x25519_scalarmult  0000275f  call     0x2764
x25519_scalarmult  00002764  lea      rax, [rbp - 0x98]
x25519_scalarmult  0000276b  mov      r8, rax
x25519_scalarmult  0000276e  lea      rax, [rbp - 0x48]
x25519_scalarmult  00002772  mov      r11, rax
x25519_scalarmult  00002775  lea      rax, [rbp - 0xe8]
x25519_scalarmult  0000277c  mov      r10, rax
x25519_scalarmult  0000277f  mov      rcx, r10
x25519_scalarmult  00002782  mov      rdx, r11
x25519_scalarmult  00002785  call     0x278a
x25519_scalarmult  0000278a  lea      rax, [rbp - 0x110]
x25519_scalarmult  00002791  mov      r8, rax
x25519_scalarmult  00002794  lea      rax, [rbp - 0x138]
x25519_scalarmult  0000279b  mov      r11, rax
x25519_scalarmult  0000279e  lea      rax, [rbp - 0x98]
x25519_scalarmult  000027a5  mov      r10, rax
x25519_scalarmult  000027a8  mov      rcx, r10
x25519_scalarmult  000027ab  mov      rdx, r11
x25519_scalarmult  000027ae  call     0x27b3
x25519_scalarmult  000027b3  jmp      0x246a
x25519_scalarmult  000027b8  mov      eax, dword ptr [rbp - 0x140]
x25519_scalarmult  000027be  mov      r8, rax
x25519_scalarmult  000027c1  lea      rax, [rbp - 0xc0]
x25519_scalarmult  000027c8  mov      r11, rax
x25519_scalarmult  000027cb  lea      rax, [rbp - 0x70]
x25519_scalarmult  000027cf  mov      r10, rax
x25519_scalarmult  000027d2  mov      rcx, r10
x25519_scalarmult  000027d5  mov      rdx, r11
x25519_scalarmult  000027d8  call     0x27dd
x25519_scalarmult  000027dd  mov      eax, dword ptr [rbp - 0x140]
x25519_scalarmult  000027e3  mov      r8, rax
x25519_scalarmult  000027e6  lea      rax, [rbp - 0xe8]
x25519_scalarmult  000027ed  mov      r11, rax
x25519_scalarmult  000027f0  lea      rax, [rbp - 0x98]
x25519_scalarmult  000027f7  mov      r10, rax
x25519_scalarmult  000027fa  mov      rcx, r10
x25519_scalarmult  000027fd  mov      rdx, r11
x25519_scalarmult  00002800  call     0x2805
x25519_scalarmult  00002805  lea      rax, [rbp - 0x98]
x25519_scalarmult  0000280c  mov      r11, rax
x25519_scalarmult  0000280f  lea      rax, [rbp - 0x98]
x25519_scalarmult  00002816  mov      r10, rax
x25519_scalarmult  00002819  mov      rcx, r10
x25519_scalarmult  0000281c  mov      rdx, r11
x25519_scalarmult  0000281f  call     0x2824
x25519_scalarmult  00002824  lea      rax, [rbp - 0x98]
x25519_scalarmult  0000282b  mov      r8, rax
x25519_scalarmult  0000282e  lea      rax, [rbp - 0x70]
x25519_scalarmult  00002832  mov      r11, rax
x25519_scalarmult  00002835  lea      rax, [rbp - 0x70]
x25519_scalarmult  00002839  mov      r10, rax
x25519_scalarmult  0000283c  mov      rcx, r10
x25519_scalarmult  0000283f  mov      rdx, r11
x25519_scalarmult  00002842  call     0x2847
x25519_scalarmult  00002847  lea      rax, [rbp - 0x70]
x25519_scalarmult  0000284b  mov      r11, rax
x25519_scalarmult  0000284e  mov      rax, qword ptr [rbp + 0x10]
x25519_scalarmult  00002852  mov      r10, rax
x25519_scalarmult  00002855  mov      rcx, r10
x25519_scalarmult  00002858  mov      rdx, r11
x25519_scalarmult  0000285b  call     0x2860
x25519_scalarmult  00002860  movabs   rax, 0x20
x25519_scalarmult  0000286a  mov      r8, rax
x25519_scalarmult  0000286d  mov      eax, 0
x25519_scalarmult  00002872  mov      r11, rax
x25519_scalarmult  00002875  lea      rax, [rbp - 0x20]
x25519_scalarmult  00002879  mov      r10, rax
x25519_scalarmult  0000287c  mov      rcx, r10
x25519_scalarmult  0000287f  mov      rdx, r11
x25519_scalarmult  00002882  call     0x2887
x25519_scalarmult  00002887  leave    
x25519_scalarmult  00002888  ret      
x25519_base        00002889  push     rbp
x25519_base        0000288a  mov      rbp, rsp
x25519_base        0000288d  sub      rsp, 0x20
x25519_base        00002894  mov      qword ptr [rbp + 0x10], rcx
x25519_base        00002898  mov      qword ptr [rbp + 0x18], rdx
x25519_base        0000289c  lea      rax, [rip]
x25519_base        000028a3  mov      r8, rax
x25519_base        000028a6  mov      rax, qword ptr [rbp + 0x18]
x25519_base        000028aa  mov      r11, rax
x25519_base        000028ad  mov      rax, qword ptr [rbp + 0x10]
x25519_base        000028b1  mov      r10, rax
x25519_base        000028b4  mov      rcx, r10
x25519_base        000028b7  mov      rdx, r11
x25519_base        000028ba  call     0x28bf
x25519_base        000028bf  leave    
x25519_base        000028c0  ret      
x25519_is_zero     000028c1  push     rbp
x25519_is_zero     000028c2  mov      rbp, rsp
x25519_is_zero     000028c5  sub      rsp, 0x10
x25519_is_zero     000028cc  mov      qword ptr [rbp + 0x10], rcx
x25519_is_zero     000028d0  mov      eax, 0
x25519_is_zero     000028d5  mov      byte ptr [rbp - 1], al
x25519_is_zero     000028d8  mov      eax, 0
x25519_is_zero     000028dd  mov      dword ptr [rbp - 8], eax
x25519_is_zero     000028e0  mov      eax, dword ptr [rbp - 8]
x25519_is_zero     000028e3  cmp      eax, 0x20
x25519_is_zero     000028e6  jge      0x291a
x25519_is_zero     000028ec  jmp      0x28ff
x25519_is_zero     000028f1  mov      eax, dword ptr [rbp - 8]
x25519_is_zero     000028f4  mov      rcx, rax
x25519_is_zero     000028f7  add      eax, 1
x25519_is_zero     000028fa  mov      dword ptr [rbp - 8], eax
x25519_is_zero     000028fd  jmp      0x28e0
x25519_is_zero     000028ff  mov      eax, dword ptr [rbp - 8]
x25519_is_zero     00002902  movsxd   rax, eax
x25519_is_zero     00002905  mov      rcx, qword ptr [rbp + 0x10]
x25519_is_zero     00002909  add      rcx, rax
x25519_is_zero     0000290c  movzx    eax, byte ptr [rbp - 1]
x25519_is_zero     00002910  movzx    edx, byte ptr [rcx]
x25519_is_zero     00002913  or       eax, edx
x25519_is_zero     00002915  mov      byte ptr [rbp - 1], al
x25519_is_zero     00002918  jmp      0x28f1
x25519_is_zero     0000291a  movzx    eax, byte ptr [rbp - 1]
x25519_is_zero     0000291e  cmp      eax, 0
x25519_is_zero     00002921  mov      eax, 0
x25519_is_zero     00002926  sete     al
x25519_is_zero     00002929  leave    
x25519_is_zero     0000292a  ret      