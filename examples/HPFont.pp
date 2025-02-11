#pragma mode( separator(.,,) integer(h64) )

#include <pplang>

    regex `\bfont.bitmap\b` font[1]
    regex `\bfont.glyphs\[([\w\d.:])\]` font[2,$1+1]
    regex `\bfont.first\b` font[3]
    regex `\bfont.last\b` font[4]
    regex `\bfont.yAdvance\b` font[5]
    regex `\bglyph.xAdvance\b` BITAND(BITSR(glyph, 32), 255)
    regex `\bglyph.width\b` BITAND(BITSR(glyph, 16), 255)
    regex `\bglyph.height\b` BITAND(BITSR(glyph, 24), 255)
    regex `\bglyph.dX\b` BITAND(BITSR(glyph, 40), 255)
    regex `\bglyph.dY\b` BITAND(BITSR(glyph, 48), 255) - 256
    regex `\bglyph.bitmapOffset\b` BITAND(glyph, 65535)

GLYPH_P:drawGlyph(trgt, g:glyph, x, y, font, color, sX:sizeX, sY:sizeY)
begin
    auto bitmapOffset;
 
    var auto:glyph = font.glyphs[glyph];
    
    var h = glyph.height;
    var w = glyph.width;
    
    x = x + glyph.dX;
    y = y + (glyph.dY + font.yAdvance) * sizeY;
    
    var auto:bitmap = font.bitmap;
    var auto:bitmapOffset = glyph.bitmapOffset >> 3 + 1
    
    var auto:bits = bitmap[bitmapOffset];
    var auto:bitPosition = glyph.bitmapOffset & 7 * 8;
    
    bits >> bitPosition;
    while h do
        auto xx;
        for xx = 0; xx < w; xx += 1 do
            if bitPosition & 63 == 0 then
                bits = bitmap[bitmapOffset];
                bitmapOffset = bitmapOffset + 1;
            endif;

            if bits & -1 then
                if sizeX == 1 and sizeY == 1 then
                    PIXON_P(trgt, x + xx, y, color);
                else
                    //RECT_P(trgt, x + xx * sizeX, y, sizeX, sizeY, color);
                endif;
            endif;

            bits >> 1;
        next;

        y += sizeY;
        h -= 1;
    wend;
  
    return glyph.xAdvance;
end;

export FONT_P(trgt, text, x, y, auto:font, color)
begin
    var i, auto:asciiList := ASC(text);
 
    for i := 1 ... SIZE(l) do
        if x >= 320 then
            break;
        endif;
        
        if asciiList[i] < font.first or asciiList[i] > font.last then
            continue;
        endif;
        
        x := x + drawGlyph(trgt, l[i] - font.first + 1, x, y, font, color, 1, 1);
    next;
end;
