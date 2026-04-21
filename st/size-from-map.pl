#!/usr/bin/perl

# .text           0x0000000042880000      0x4b8
#  *(.theader)
#  .theader       0x0000000042880000        0xc build/headers.o
#                 0x0000000042880000                header
#  *(.text)
#  .text          0x000000004288000c      0x4aa build/merged.o
#                                         0x4ca (size before relaxing)
#                 0x0000000042880280                PROGRAM0_init__
#                 0x0000000042880358                PROGRAM0_body__
#                 0x0000000042880478                RES0_init__
#                 0x0000000042880490                RES0_run__
#                 0x00000000428804a4                config_init__
#                 0x00000000428804ac                config_run__
#  .text          0x00000000428804b6        0x0 build/headers.o
#                 0x00000000428804b8                . = ALIGN (0x4)
#  *fill*         0x00000000428804b6        0x2
#
# .literal        0x00000000428804b8        0x0
#  .literal       0x00000000428804b8        0x0 build/merged.o
#
# .data           0x000000003c7fffb8       0x38
#  *(.dheader)
#  .dheader       0x000000003c7fffb8       0x14 build/headers.o
#                 0x000000003c7fffb8                dheader
#  *(.data)
#  .data          0x000000003c7fffcc        0x0 build/headers.o
#  *fill*         0x000000003c7fffcc        0x4
#  .data          0x000000003c7fffd0        0x8 build/merged.o
#                 0x000000003c7fffd0                common_ticktime__
#  *(.rodata*)
#  .rodata.str1.1
#                 0x000000003c7fffd8       0x18 build/merged.o
#                 0x000000003c7ffff0                . = ALIGN (0x8)
#
# .bss            0x000000003c7ffe50       0xa0
#  *(.bss)
#  .bss           0x000000003c7ffe50        0x0 build/headers.o
#  .bss           0x000000003c7ffe50       0x9d build/merged.o
#                 0x000000003c7ffe50                greatest_tick_count__
#                 0x000000003c7ffe54                RES0__INSTANCE0

while (<>) {
    if (/^\.(text|data|bss)\s+\S+\s+(0x\w+)$/) {
        print oct($2), " ";
    }
}
print "\n";
