# Text Rasterization Explorer

A C++ project exploring how vector fonts (TTF files) get converted to pixels on screen.

## What This Does

This project breaks down text rendering fundamentals:
- **TTF file parsing** - reading font headers, tables, and metadata
- **Glyph data extraction** - understanding how character shapes are stored
- **Vector point analysis** - examining curves, contours, and control points
- **Visual debugging** - ASCII plots and SVG exports of glyph outlines

## Current Features

✅ **Font Structure Analysis:**
- [x] TTF header parsing (scaler type, table count)
- [x] Table directory reading (finding 'glyf', 'loca', 'head' tables)
- [x] Big-endian byte order handling

✅ **Glyph Location System:**
- [x] 'loca' table parsing (both short/long formats)
- [x] Proper glyph indexing and offset calculation
- [x] Simple vs composite glyph detection

✅ **Simple Glyph Parsing:**
- [x] Glyph header extraction (contours, bounding box)
- [x] Point coordinate decoding (on-curve vs off-curve)
- [x] Flag-based compression handling
- [x] Delta coordinate reconstruction

✅ **Visualization Tools:**
- [x] ASCII art plotting in terminal
- [x] SVG export with point numbering
- [x] Contour outline rendering

🚧 **Known Limitations:**
- [ ] Composite glyph support (many fonts use these)
- [ ] Character-to-glyph mapping ('cmap' table)
- [ ] Hinting instruction processing
- [ ] Actual rasterization to pixels

## Sample Output

```
TTF Header:
  scalerType: 0x10000 (TrueType)
  numTables: 17

=== Understanding 'loca' table ===
Found 'head' table at offset: 316
Format: Long (4 bytes)
Found 'loca' table at offset: 123456, length: 2000
Number of glyphs: 499

=== Reading Glyph 42 ===
Reading glyph 42 at offset 27890 (size: 156 bytes)
Glyph Info:
  Contours: 2
  Bounding box: (45, 0) to (420, 730)
  End points: 15 31
  Points:
    0: (90, 0) ON
    1: (420, 0) ON
    2: (420, 90) OFF
    ...

Glyph Plot (80x40):
* = on-curve points, o = off-curve (control) points
    *
   o o
  *   *
 o     o
*       *

SVG exported to: glyph_42.svg
```

## Building & Running

```bash
# Compile
g++ main.cpp ttf_reader.cpp -o text_raster

# Run with your TTF font
./text_raster

# View generated SVG files in browser
firefox glyph_42.svg
```

## Project Structure

```
text_raster/
├── main.cpp              # Main program entry
├── ttf_reader.h           # TTF parsing declarations
├── ttf_reader.cpp         # TTF parsing implementation
├── README.md              # This file
├── *.svg                  # Generated glyph visualizations
└── *.ttf                  # Font files (not in repo)
```

## Understanding the Output

**Glyph Types:**
- **Simple glyphs** - contain actual outline data (what we can read)
- **Composite glyphs** - reference other glyphs (common in modern fonts)
- **Empty glyphs** - characters like space with no visual data

**Point Types:**
- **ON-curve points** (red/`*`) - actual outline vertices
- **OFF-curve points** (blue/`o`) - Bézier curve control points

**Coordinates:**
- Font units (typically 0-1000 or 0-2048)
- Y-axis points up (opposite of screen coordinates)
- Relative positioning using delta compression

## Learning Goals

- Understand TTF file format structure
- See how fonts store vector character shapes  
- Learn about Bézier curves and font geometry
- Build foundation for text rendering systems

---

*Educational project - focused on understanding rather than performance*