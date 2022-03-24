chunkinfo - show information of PNG chunks

supported chunks
----------------
- IHDR
- PLTE
- IDAT (with -D_DECODE_IDAT, but only created a zlib file, so you need to manually decompress the zlib file)
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
- acTL (from APNG)
- fcTL (from APNG)

references
----------
- http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html
- https://www.w3.org/TR/2003/REC-PNG-20031110
- http://www.libpng.org/pub/png/book/toc.html
- http://www.schaik.com/pngsuite
- https://wiki.mozilla.org/APNG_Specification
- https://philip.html5.org/tests/apng/tests.html
- https://github.com/skeeto/scratch/tree/master/pngattach

notes
-----
this software is far from perfect, no chunk checking like chunk ordering
of IDAT and PLTE, because there are chunks that need to appear before
IDAT and PLTE like cHRM, gAMA, and iCPP.

and last, chunkinfo is public domain.
