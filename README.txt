chunkinfo - show information of PNG chunks

installation and example
------------------------
$ make
$ ./chunkinfo test.apng
[IHDR] length 13 at offset 0x0000000c (f1fde38f)
	Width = 51
	Height = 50
	Bit depth = 8 per channel
	Color type = RGB with Alpha channel
	Channels = 4 per pixel (32 bits)
	Compression = 0 (zlib deflate/inflate)
	Filter = 0 (adaptive filtering)
	Interlace = 0 (no interlace)

[acTL] length 8 at offset 0x00000025 (f38d9370)
	Number of frames = 2
	Number of plays = 0 (infinite)

[fcTL] length 26 at offset 0x00000039 (aa53fc21)
	Width = 51
	Height = 50
	X offset = 0
	Y offset = 0
	Delays = 1000 (denominator 1000)
	Disposal = 0 (None)
	Blend = 1 (Over)

[IDAT] length 1085 at offset 0x0000005f (80ad91a6)
	Image data

[fcTL] length 26 at offset 0x000004a8 (6257f46c)
	Width = 23
	Height = 41
	X offset = 9
	Y offset = 9
	Delays = 500 (denominator 1000)
	Disposal = 0 (None)
	Blend = 1 (Over)

[fdAT] length 180 at offset 0x000004ce (e64f2ddc)
	.....

[IEND] length 0 at offset 0x0000058e (ae426082)
	No data

All OK.
Found 7 chunks from file test.apng

supported chunks
----------------
- IHDR
- PLTE
- IDAT [*]
- tIME
- pHYs
- sRGB
- gAMA
- cHRM
- iCCP (partial, color profile is still compressed...)
- tEXt
- iTXt (partial, UTF-8 text is not printed...)
- zTXt (partial, text is still compressed...)
- bKGD
- sBIT
- tRNS
- sPLT
- hIST
- IEND
- oFFs (PNG official extension)
- sCAL (PNG official extension)
- pCAL (PNG official extension)
- gIFx (PNG official extension)
- gIFg (PNG official extension)
- sTER (PNG official extension)
- acTL (from APNG)
- fcTL (from APNG)

[*] = compile it with "make idat", it will create a file with filename
      "filename-IDAT.zlib", but if you want to run chunkinfo with same PNG
      file, you need to remove the previous "filename-IDAT.zlib", if not,
      in the next run it will append new IDAT to the same zlib file.

references
----------
- http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html
- https://www.w3.org/TR/2003/REC-PNG-20031110
- http://www.libpng.org/pub/png/book/toc.html
- http://www.libpng.org/pub/png/spec/register
- http://www.schaik.com/pngsuite
- https://wiki.mozilla.org/APNG_Specification
- https://philip.html5.org/tests/apng/tests.html
- https://github.com/skeeto/scratch/tree/master/pngattach
- http://www.libpng.org/pub/png/apps/pngcheck.html
- https://www.nayuki.io/page/png-file-chunk-inspector
- https://exiftool.org/TagNames/PNG.html
- https://w3c.github.io/PNG-spec

notes
-----
This software is far from perfect, no chunk checking like chunk ordering
of IDAT and PLTE, because there are chunks that need to appear before
IDAT and PLTE like cHRM, gAMA, and iCPP.
The error message is not really useful too.

And last, chunkinfo is public domain.
