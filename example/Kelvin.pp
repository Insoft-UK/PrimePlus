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


#include <prime>

export KELVIN(t:temperature)
begin
    var r:red=255, g:green, b:blue=255;
    def min(max(value,0),255) clamp(value);
    
    temperature /= 100.0;
    
    if temperature > 66 do
        red = temperature - 60;
        red = 329.698727446 * red^-0.1332047592;
        red = clamp(red);
        
        green = temperature - 60;
        green = 288.1221695283 * green^0.0755148492;
        green = clamp(green);
    else
        green = temperature;
        green = 99.4708025861 * log(green) - 161.1195681661;
        green = clamp(green);
#pragma ( cstyle )
        if (temperature <= 19) {
            return RGB(red, green, 0);
        }
#pragma ( cstyle )
        blue = temperature - 10;
        blue = 138.5177312231 * log(blue) - 305.0447927307;
        blue = clamp(blue);
    endif
    
    
    return RGB(red, green, blue);
end
