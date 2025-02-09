#include <pplang>
GLYPH_P(trgt, ascii, x, y, auto:font, color)
{
    var g:glyph, btm:bitmap, os:bitmapOffset;
    var w:width, h:height, bP, d, xx;
 
    regex `\bfont.bitmap\b` font[1]
    regex `\bfont.glyphs\[(ascii)\]` font[2,$1]
    regex `\bfont.first\b` font[3]
    regex `\bfont.last\b` font[4]
    regex `\bfont.yAdvance\b` font[5]
    regex `\bglyph.xAdvance\b` BITAND(BITSR(glyph, 32), 255)
    regex `\bglyph.width\b` BITAND(BITSR(glyph, 16), 255)
    regex `\bglyph.height\b` BITAND(BITSR(glyph, 24), 255)
    regex `\bglyph.dX\b` BITAND(BITSR(glyph, 40), 255)
    regex `\bglyph.dY\b` BITAND(BITSR(glyph, 48), 255) - 256
    regex `\bglyph.bitmapOffset\b` BITAND(glyph, 65535)
    

    glyph := font.glyphs[ascii];
 
    if BITAND(g, #FFFFFFFFh) == 0 {
        return xA;
    }
 
    bitmap = font.bitmap;
 
    bitmapOffset = glyph.bitmapOffset;
    width = glyph.width;
    height = glyph.height;

    x = x + glyph.dX;
    y = y + glyph.dY + font.yAdvance;
  
    bP = bitmapOffset & 7 * 8;
    os = bitmapOffset >> 3 + 1;
    d = bitmap[bitmapOffset];
    
    d = d >> bP;
  
    do {
        for xx = 1 ... width {
            if bP == 64 {
                bP = 0;
                bitmapOffset = bitmapOffset + 1;
                d = bitmap[bitmapOffset];
            }

            if d & 1 == 1 {
                PIXON_P(trgt, x + xx,y, color);
            }

            bP = bP + 1;
            d = d >> 1;
        }

        y := y + 1;
        height--;
    } while height;
  
    return glyph.xAdvance;
}

EXPORT FONT_P(trgt, text, x, y, fnt, color)
BEGIN
 LOCAL i, l := ASC(text);
 
 FOR i := 1 TO SIZE(l) DO
  IF x >= 320 THEN BREAK; END;
  IF l[i] < fnt[3] OR l[i] > fnt[4] THEN CONTINUE; END;
  x := x + GLYPH_P(trgt, l[i] - fnt[3] + 1, x, y, fnt, color);
 END;
END;
