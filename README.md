BS2PC PS2 fix by Supadupaplex
=============================

This version of BS2PC contains various fixes/hacks for bsp to bs2 map conversion
that I made throughout 2017-2019 with some additional tweaks and new stuff
(I hope I didn't break anything when I was putting this thing together).

Fixes list:
1. increased subdivisions and vertices count caps
2. qsort WAD lumps: fix crashing on some WADs
3. slow texture lump iteration: fix animated textures
4. prefer WAD texture sizes over sizes stored in bsp: scaling artifacts can
   be avoided if WAD textures are manually resized to match power of 2
5. calculate bs2 texture lump size based on WAD texture sizes
6. added options: -nomerge, -szreport, -wadonly
7. give warning if bs2 map file is too big
8. added options: -noresize, -dryrun
9. fixed '{' textures brightness
10. ability to tweak subdivision with -divsz or to disable it with -nodiv

[Latest builds](https://github.com/supadupaplex/BS2PC/releases)


    Usage: bs2pc [-game \"path to base WAD directory for .bsp to .bs2\"]
        [-game \"path to mod WAD directory\"] \"source file\" [\"target file\"]
        [-szreport] [-wadonly] [-dryrun] [-nomerge] [-nodiv] [-divsz X]
        [-noresize]
    -nodiv    - [bsp to bs2] omit subdivision step. This option can reduce map
        RAM overhead and also improve performance a bit, especially on maps with
        giant water surfaces. Waves can look weird though, you may need to set
        "WaveHeight" to 0 in the entity properties
    -divsz    - [bsp to bs2] set subdivision size (default is 8). By increasing
        this parameter you can reduce number of subdivisions and thus to get
        benefits of -nodiv option up to a certain degree
    -nomerge  - [bsp to bs2] skip BS2PC_MergeStrips func, suggested by Triang3l:
        \"this function is unfinished. In this state it can bring
        performance boost but it also can cause glitches on water
        and transparent surfaces\"
    -szreport - [bsp to bs2] report map file lump sizes
    -wadonly  - [all] load textures from WAD files only, ignore textures stored
        directly in bsp/bs2 files
    -noresize - [bs2 to bsp] skip resizing textures - (!) breaks maps, useful
        for PS2 texture harvesting only
    -dryrun   - [all] skip write to output file (might be useful for
        troubleshooting or lump sizes inspection)


Original readme:

BS2PC
=====

Converts Half-Life PlayStation 2 (`.bs2`) maps to PC (`.bsp`).

Usage: `bs2pc path_to_map.bs2` or drag and drop. The `.bsp` file will be placed in the `.bs2` directory.

See the [Releases section](https://github.com/Triang3l/BS2PC/releases) for the Windows binary.

Features:
* Converts all sections of PlayStation 2 maps.
* Automatically decompresses the map.
* Changes `.dol` and `.spz` file extensions to `.mdl` and `.spr` in the entities.
* Removes `nodraw`-textured surfaces such as the edges of the handrails near the Sector C entrance.

Half-Life on PS2 uses DEFLATE compression for its maps and stores its internal structures (like the `m`-prefixed structures in GLQuake as opposed to the `d`-prefixed ones in the software-rendered engines and that are used in Quake and Half-Life `.bsp`) in the map files. This is part of the reason why the PlayStation 2 version loads levels much faster than the Dreamcast one. You can see some differences in the `bs2pc_lumps.h`, the PC structures have `_id_t` in the name, and the PlayStation 2 ones have `_gbx_t`. Many fields were omitted though because they are not needed in the PC format. Two sections, very similar to clipnodes, are also skipped.

On the PC maps after playing the PS2 ones or vice versa, some textures may look different (from minor power of 2 scaling artifacts to completely different images) as the textures have common names. If you want to switch between the two versions, restart the game so everything looks correct.

Licensed under GPLv2 (except for zlib) as it's derived from the Quake engine source, but not enforced - the format research results can be considered being in the public domain.
