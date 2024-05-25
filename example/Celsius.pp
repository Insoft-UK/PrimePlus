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

#pragma ( minify 0, reduce, newline )

// PPL Style Used! ; & if...then end := () !ONLY

struct Color
    r(1);
    g(2);
    b(3);
end;

ICOLOR:interpolateColor(a:colorA, b:colorB, f:factor)
begin
    def min(max(value,0),255) clamp(value);
    struct Color colorA, colorB, r:result;
    
    result.r := <int>( ((1 - factor) * colorA.r + factor * colorB.r) );
    result.g := <int>( ((1 - factor) * colorA.g + factor * colorB.g) );
    result.b := <int>( ((1 - factor) * colorA.b + factor * colorB.b) );
    
    result.r := clamp(result.r);
    result.g := clamp(result.g);
    result.b := clamp(result.b);
    
    return result;
end;

export CELSIUS(t:temperature)
begin
    struct Color c:color;

    case
        if temperature <= 0.0 then
            color = {0, 174, 255};
        end;
    
        if temperature <= 10 then
            color := interpolateColor({0, 174, 255}, {0, 255, 255}, t / 10);
        end;
    
        if temperature <= 20 then
            color := interpolateColor({0, 255, 255}, {0, 255, 128}, (t - 10) / 10);
        end;
    
        if temperature <= 30 then
            color := interpolateColor({0, 255, 128}, {255, 255, 0}, (t - 20) / 10);
        end;
    
        if temperature <= 40 then
            color := interpolateColor({255, 255, 0}, {255, 165, 0}, (t - 30) / 10);
        end;
    end;
    
    color := interpolateColor({255, 165, 0}, {255, 0, 0}, (t - 40) / 10);
    return RGB(color.r, color.g, color.b);
end;
