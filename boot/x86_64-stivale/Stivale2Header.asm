.code64
.section .stivale2hdr

stivale2_header_tag_base:
    .quad 0
    .quad 0
    .quad 0
    .quad stivale2_header_tag_any_video

.section .rodata

stivale2_header_tag_any_video:
    .quad 0xc75c9fa92a44c4db
    .quad stivale2_header_tag_framebuffer
    #preference = 0, meaning pixel-based buffer (rather than character-based)
    .quad 0

stivale2_header_tag_framebuffer:
    .quad 0x3ecc1bc43d0f7971
    .quad stivale2_header_tag_smp
    #requested w/h/bpp, or 0 if we dont care
    .word 0
    .word 0
    .word 0
    #reserved bit, required for padding
    .word 0

stivale2_header_tag_smp:
    .quad 0x1ab015085f3273df
    .quad 0
    #set bit 1 to request x2apic over xapic (if avail)
    .quad 0
