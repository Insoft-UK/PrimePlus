The HP Prime stores its glyph data as a list of 64-bit unsigned integers. The bitmap, however, is stored in a specific bit order (little-endian) and where each byte of the 64-bit value is mirror-flipped.

e.g.
```
01101100 #6Ch to 00110110 #36h
11111110 #FEh to 01111111 #7Fh
11111110 #FEh to 01111111 #7Fh
11111110 #FEh to 01111111 #7Fh
01111110 #7Eh to 01111110 #7Eh
00111000 #38h to 00011100 #1Ch
00010000 #10h to 00001000 #08h
00000000 #00h to 00000000 #00h := #00081C7E7F7F7F36:64h
```

### Little-Endian
`#00081C7E7F7F7F36:64h`
#### Glyphs
|-|Y Positiond|X Positiond|X Advance|Height|Width|Offset Address|
|-|---|---|---|---|---|---|
|#00:8h|#00:8h|#00:8h|#00:8h|#00:8h|#00:8h|#0000:16h|


`#--YYXXAAHHWWOOOO:64h = #--:8h #00:8h #00:8h #00:8h #00:8h #00:8h #0000:16h`

HP.prgm
```
LOCAL Glyph(trgtG, ascii, x, y, fnt, color, sizeX, sizeY)
BEGIN
 LOCAL g := fnt[2, ascii];
 LOCAL xAdvance := BITAND(BITSR(g,32), 255) * sizeX;
 
  IF BITAND(g,#FFFFFFFF)==0 THEN
    RETURN xAdvance;
  END;
  
  LOCAL w, h, dX, dY, xx;
  LOCAL yAdvance := fnt[5];

  w := BITAND(BITSR(g, 16), 255);
  h := BITAND(BITSR(g, 24), 255);
 
  dX := BITAND(BITSR(g, 40), 255);
  dY := BITAND(BITSR(g, 48), 255) - 256;
 
  x := x + dX * sizeX;
  y := y + yAdvance + dY;
  
  LOCAL bitmap := fnt[1];

  LOCAL offset := BITAND(g, 65535);
  LOCAL bitPosition := BITAND(offset, 7) * 8;
  offset := BITSR(offset, 3) + 1;
  LOCAL bits := BITSR(bitmap[offset], bitPosition);
  
  REPEAT
    FOR xx FROM 1 TO w DO
      IF bitPosition == 64 THEN
        bitPosition := 0;
        offset := offset + 1;
        bits := bitmap[offset];
      END;
     
      IF BITAND(bits, 1) == 1 THEN
        IF sizeX == 1 AND sizeY == 1 THEN
          PIXON_P(trgtG, x + xx,y, color);
        ELSE
          RECT_P(trgtG, x + xx * sizeX, y, x + xx * sizeX + sizeX - 1, y + sizeY - 1, color);
        END;
      END;
      
      bitPosition := bitPosition + 1;
      bits := BITSR(bits, 1);
    END;
   
    y + sizeY ▶ y;
    h - 1 ▶ h;
  UNTIL h == 0;
  
  RETURN xAdvance;
END;

EXPORT Text(trgt, text, x, y, fnt, color, sizeX, sizeY)
BEGIN
  LOCAL i, l := ASC(text);
 
  FOR i := 1 TO SIZE(l) DO
    IF x >= 320 THEN
      BREAK;
    END;
    IF l[i] < fnt[3] OR l[i] > fnt[4] THEN
      CONTINUE;
    END;
    x := x + Glyph(trgt, l[i] - fnt[3] + 1, x, y, fnt, color, sizeX, sizeY);
  END;
END;
```
