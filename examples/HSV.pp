/*
 The MIT License (MIT)
 
 Copyright (c) 2023-2024 Insoft. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#pragma mode( separator(.,;) integer(h64) )

export HSV(h:hue, s:saturation, v:value)
begin
    hue = hue % 360 / 60;
    saturation = MIN(MAX(saturation, 0), 100) / 100;
    value = MIN(MAX(value, 0), 100) / 100;
    def FLOOR       floor;
    
    var f, p, q, t, m;
    f = hue - floor(h);
    p = value * (1 - saturation);
    q = value * (1 - saturation * f);
    t = value * (1 - saturation * (1 - f));

    var r:red, g:green, b:blue;
    m = floor(h);
    
    if m==0 then red = value; green = t; blue = p; end;
    if m==1 then red = q; green = value; blue = p; end;
    if m==2 then red = p; green = value; blue = t; end;
    if m==3 then red = p; green = q; blue = value; end;
    if m==4 then red = t; green = p; blue = value; end;
    if m==5 then red = value; green = p; blue = q; end;
  
    return RGB(red * 255, green * 255, blue * 255);
end;
