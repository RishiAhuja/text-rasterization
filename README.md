# Text Rasterization Explorer

A simple C++ project to understand how vector fonts (TTF files) get converted to pixels on screen.

## What This Does

This project explores the basics of text rendering by:
- Reading and parsing TTF font files
- Understanding font data structures (headers, tables, glyphs)
- Extracting vector glyph data (points, curves, contours)
- Learning the fundamentals of converting vector shapes to pixels

## Why This Exists

Text rendering is everywhere but feels like magic. This project breaks it down into simple, understandable pieces to learn:
- How TTF files store character shapes as vector data
- What happens when you display text on screen
- The structure of font tables and glyph data
- How coordinates and curves define character outlines

## Current Features

✅ **Implemented:**
- [x] Reading TTF file headers and basic info
- [x] Parsing font table directory
- [x] Finding and reading specific tables (like 'glyf')
- [x] Extracting simple glyph data (contours, points, coordinates)
- [x] Understanding on-curve vs off-curve points
- [x] Handling big-endian byte order conversion

🚧 **In Progress:**
- [ ] Composite glyph support
- [ ] Basic rasterization (vector to pixels)
- [ ] Character mapping (Unicode to glyph index)
- [ ] Rendering to pixel buffer

## Building

```bash
g++ main.cpp ttf_reader.cpp -o text_raster
./text_raster
```

## Sample Output

```
TTF Header:
  scalerType: 0x10000 (TrueType)
  numTables: 17

Tables:
  Table: GDEF, Offset: 242428, Length: 554
  Table: GPOS, Offset: 242984, Length: 9224
  ...

--- Reading 'glyf' table ---
Found 'glyf' table at offset: 565356, length: 286516

--- Parsing first glyph ---
Glyph Info:
  Contours: 3
  Bounding box: (0, 0) to (510, 730)
  End points: 3 6 9 
  Points:
    0: (90, 0) ON
    1: (420, 0) ON
    2: (420, 90) OFF
    ...
```

## Learning Goals

- Understand TTF file format structure
- Learn how fonts store vector outlines
- See how bezier curves define character shapes
- Build intuition for text rendering pipelines
- Keep it simple and educational

## Project Structure

```
text_raster/
├── main.cpp          # Main program
├── ttf_reader.h       # TTF parsing header
├── ttf_reader.cpp     # TTF parsing implementation
└── README.md          # This file
```

---

*This is a learning project focused on understanding fundamentals rather than performance or completeness.*