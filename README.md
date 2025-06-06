# Text Rasterization Explorer

A simple C++ project to understand how vector fonts (TTF files) get converted to pixels on screen.

## What This Does

This project explores the basics of text rendering:
- Reading TTF font files
- Understanding font data structures
- Converting vector shapes to pixel grids (rasterization)
- Learning how text gets from font file to screen

## Why This Exists

Text rendering is everywhere but feels like magic. This project breaks it down into simple, understandable pieces to learn:
- How fonts store character shapes
- What happens when you display text
- The math behind converting curves to pixels

## Current Status

**Work in Progress** - Currently learning and implementing:
- [x] Reading TTF file headers
- [ ] Parsing font tables
- [ ] Extracting glyph data
- [ ] Basic rasterization
- [ ] Rendering to pixel buffer

## Building

```bash
g++ main.cpp -o text_raster
./text_raster
```

## Learning Goals

- Understand TTF file format
- Learn vector to raster conversion
- Build intuition for text rendering pipelines
- Keep it simple and educational

---

*This is a learning project focused on understanding fundamentals rather than performance or completeness.*