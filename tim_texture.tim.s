.section .data

.global font_texture_tim_data
.type tim_texture_64, @object
font_texture_tim_data:
    .balign 4 # needed because it's an array of uint32_ts
    .incbin "testfont.tim"
