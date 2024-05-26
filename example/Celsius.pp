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

#pragma ( minify 0, reduce )

// PPL Style Used!

struct Color
    r(1);
    g(2);
    b(3);
end;

auto:interpolateColor(a:colorA, b:colorB, f:factor)
begin
    struct Color colorA, colorB, r:result;
    
    result.r := <int>( ((1 - factor) * colorA.r + factor * colorB.r) );
    result.g := <int>( ((1 - factor) * colorA.g + factor * colorB.g) );
    result.b := <int>( ((1 - factor) * colorA.b + factor * colorB.b) );
    
    return result;
end;

export C→RGB:celsiusToRGB(t:temperature)
begin
    struct Color c:color;
    
    var auto:colors := {{255,255,255},{255,127,255},{127,0,127},{0,0,255},{0,127,255},{0,255,255},{0,255,127},{255,255,0},{255,127,0},{255,0,0},{127,0,0},{0,0,0}};
    temperature += 40;
    var auto:index := <int>(temperature / 10) + 1;
    
    case
        if temperature <= 0.0 then
            color := colors(1);
        end;
        
        default
            try
                color := interpolateColor(colors(index), colors(index + 1), temperature % 10 / 10);
            catch
                color := colors(12);
            end;
    end;
    
    return RGB(color.r, color.g, color.b);
end;

