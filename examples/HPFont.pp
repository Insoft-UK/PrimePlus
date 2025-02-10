#include <pplang>

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

GLYPH_P(trgt, c:charactor, x, y, font, color, sX:sizeX, sY:sizeY)
begin
    var os:bitmapOffset;
    var w:width, h:height, bP, d, xx;
 
    auto glyph = font.glyphs[charactor - font.first];
    
    var h = glyph.height;
    var w = glyph.width;
    
    x = x + glyph.dX;
    y = y + (glyph->dY + font.yAdvance) * sizeY;
    
    var BM:bitmap = font.bitmap;
    var o:bitmapOffset = glyph->bitmapOffset >> 3 + 1
    
    var b:bits = bitmap[bitmapOffset];
    var p:bitPosition = glyph->bitmapOffset & 7 * 8;
    
    bits >> bitPosition;
    while h do
        var xx;
        for xx = 0; xx < w; xx := xx + 1 do
            if bitPosition & 63 == 0 then
                bits = bitmap[bitmapOffset];
                bitmapOffset += 1;
            endif;

            if bits & 1 then
                if (sizeX == 1 && sizeY == 1) {
                    PIXON_P(trgt, x + xx, y, color);
                else
                    //RECT_P(trgt, x + xx * sizeX, y, sizeX, sizeY, color);
                endif;
            endif;

            bits >> 1;
        next;

        y = y + ySize;
        h = h - 1;
    wend;
  
    return glyph.xAdvance;
end;

export FONT_P(trgt, text, x, y, auto:font, color)
begin
    var i, l := ASC(text);
 
    for i := 1 ... SIZE(l) do
        if x >= 320 then break; end;
        if l[i] < font.first or l[i] > font.last then continue; endif;
        x := x + GLYPH_P(trgt, l[i] - font.first + 1, x, y, font, color);
    next;
end;
