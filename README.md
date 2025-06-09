# Text Rasterization Explorer

A C++ project exploring how vector fonts (TTF files) get converted to pixels on screen.

## What This Does

This project breaks down text rendering fundamentals:
- **TTF file parsing** - reading font headers, tables, and metadata
- **Glyph data extraction** - understanding how character shapes are stored
- **Vector point analysis** - examining curves, contours, and control points
- **Bézier curve rendering** - converting TrueType quadratic curves to smooth outlines
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
- [x] Safe glyph-by-index reading with bounds checking

✅ **Simple Glyph Parsing:**
- [x] Glyph header extraction (contours, bounding box)
- [x] Point coordinate decoding (on-curve vs off-curve)
- [x] Flag-based compression handling
- [x] Delta coordinate reconstruction

✅ **Bézier Curve Rendering:**
- [x] TrueType quadratic Bézier curve support
- [x] Implied point calculation between consecutive off-curves
- [x] SVG path-based curve rendering (Q commands)
- [x] Proper contour closing with Z commands
- [x] Native browser curve rendering (no approximation)

✅ **Visualization Tools:**
- [x] ASCII art plotting in terminal
- [x] Basic SVG export with point numbering
- [x] Smooth curve SVG export with mathematical accuracy
- [x] Dual output (straight lines vs curves) for comparison

🚧 **Known Limitations:**
- [ ] Composite glyph support (many fonts use these extensively)
- [ ] Character-to-glyph mapping ('cmap' table)
- [ ] Hinting instruction processing
- [ ] Actual rasterization to pixels
- [ ] Cubic Bézier curves (PostScript fonts)

## Sample Output

```
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
    3: (400, 150) ON
    ...

Glyph Plot (80x40):
* = on-curve points, o = off-curve (control) points
    *
   o o
  *   *
 o     o
*       *

SVG exported to: glyph_42_basic.svg
Curve SVG exported to: glyph_42_curves.svg
```

## Bézier Curve Implementation

The project implements TrueType's quadratic Bézier curve system:

**SVG Path Commands Used:**
- `M x y` - Move to starting point
- `L x y` - Straight line to point (for on-curve → on-curve)
- `Q cx cy x y` - Quadratic curve with control point (for off-curve points)
- `Z` - Close path back to start

**TrueType Curve Rules:**
1. **ON → OFF → ON**: Creates single curve with middle point as control
2. **ON → OFF → OFF → ON**: Creates two curves with implied point between off-curve points
3. **Implied points**: Located at midpoint between consecutive off-curve points

## Building & Running

```bash
# Compile
g++ main.cpp ttf_reader.cpp -o text_raster

# Run with your TTF font
./text_raster

# View generated SVG files
firefox glyph_42_basic.svg    # Straight line version
firefox glyph_42_curves.svg  # Smooth curve version
```

## Project Structure

```
text_raster/
├── main.cpp              # Main program entry
├── ttf_reader.h           # TTF parsing declarations
├── ttf_reader.cpp         # TTF parsing implementation
├── README.md              # This file
├── *_basic.svg            # Basic glyph outlines (straight lines)
├── *_curves.svg           # Smooth curve outlines (Bézier curves)
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

**SVG Outputs:**
- **Basic version** - connects points with straight lines (shows font structure)
- **Curve version** - renders smooth Bézier curves (shows actual appearance)

**Coordinates:**
- Font units (typically 0-1000 or 0-2048)
- Y-axis points up (opposite of screen coordinates)
- Relative positioning using delta compression

## Technical Highlights

- **Proper 'loca' table parsing** for safe glyph access
- **Native SVG curve rendering** using browser's mathematical precision
- **TrueType-compliant** curve interpretation with implied points
- **Memory-safe** file parsing with bounds checking
- **Educational output** showing both raw structure and final appearance

## Learning Goals

- Understand TTF file format structure and tables
- See how fonts store vector character shapes using points and curves
- Learn about quadratic Bézier curves and font geometry
- Experience the complexity gap between font data and rendered text
- Build foundation for understanding text rendering pipelines

---

*Educational project - focused on understanding font internals rather than production text rendering*