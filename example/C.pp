/*
Copyright © 2024 Insoft. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#pragma ( minify -1, reduce, newline )

#include <prime>

struct Color
    r[1];
    g[2];
    b[3];
end;

ICOLOR:interpolateColor(auto:a, auto:b, f:factor)
begin
#pragma ( verbose aliases )
    struct Color auto:colorA, auto:colorB;
    struct Color r:result;
#pragma ( verbose aliases )
    colorA = a; colorB = b;
    result.r = <int>( ((1 - factor) * colorA.r + factor * colorB.r) );
end

export C(t:temperature)
begin
    var r:red=255, g:green, b:blue=255;
    def min(max(value,0),255) clamp(value);
    
    return RGB(red, green, blue);
end
