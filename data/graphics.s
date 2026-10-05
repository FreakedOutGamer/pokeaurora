@ The screens' graphics (graphics/), at the end of the read-only data.

	.section .rodata

	.global sAdapterErrorGfx
sAdapterErrorGfx:
	.incbin "graphics/adapter_error/tiles.4bpp"

	.global sPleaseWaitGfx
sPleaseWaitGfx:
	.incbin "graphics/please_wait/tiles.4bpp"

	.global sAdapterErrorTilemap
sAdapterErrorTilemap:
	.incbin "graphics/adapter_error/tilemap.bin"

	.global sPleaseWaitTilemap
sPleaseWaitTilemap:
	.incbin "graphics/please_wait/tilemap.bin"

	.global sAdapterErrorPalette
sAdapterErrorPalette:
	.incbin "graphics/adapter_error/palette.gbapal"

	.global sPleaseWaitPalette
sPleaseWaitPalette:
	.incbin "graphics/please_wait/palette.gbapal"

	.global sSendingGfx
sSendingGfx:
	.incbin "graphics/sending/tiles.4bpp"

	.global sIdChangeGfx
sIdChangeGfx:
	.incbin "graphics/id_change/tiles.4bpp"

	.global sExpiredGfx
sExpiredGfx:
	.incbin "graphics/expired/tiles.8bpp"

	.global sDigitsGfx
sDigitsGfx:
	.incbin "graphics/digits/tiles.4bpp"

	.global sSendingPalette
sSendingPalette:
	.incbin "graphics/sending/palette.gbapal"

	.global sIdChangePalette
sIdChangePalette:
	.incbin "graphics/id_change/palette.gbapal"

	@ Some of its colors have the unused top bit set, so it is kept as it is.
	.global sExpiredPalette
sExpiredPalette:
	.incbin "graphics/expired/palette.gbapal"

	.global sDigitsPalette
sDigitsPalette:
	.incbin "graphics/digits/palette.gbapal"

	.global sSendingBgTilemap
sSendingBgTilemap:
	.incbin "graphics/sending/background.bin"

	.global sSendingTextTilemap
sSendingTextTilemap:
	.incbin "graphics/sending/text.bin"

	.global sIdSelectTilemap
sIdSelectTilemap:
	.incbin "graphics/id_change/select.bin"

	.global sIdChangedTilemap
sIdChangedTilemap:
	.incbin "graphics/id_change/changed.bin"

	@ The second half isn't used.
	.global sExpiredTilemap
sExpiredTilemap:
	.incbin "graphics/expired/tilemap.bin"
