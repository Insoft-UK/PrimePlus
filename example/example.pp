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


#pragma ( messages )

#include <prime>
#include <hp>

Example::Switch()
begin
    gg+=1;
    struct Event auto:event;
    event = hp::waitEvent;
    
    gg = #65h
    gg = 5 & 7;
    
    
    
    if hp::isKeyPressed(event) == true && hp::isKeyPressed(event) != false then
        case
            if event.key==KeyCode.Esc then
                return 0;
            endif;
            
            if event.key==KeyCode.1 then
                return 10;
            endif;
            
            if event.key==KeyCode.2 then
                return 100;
            endif;
            
            if event.key==KeyCode.3 then
                return 1000;
            endif;
        end;
    endif;
    
    switch event.key
        case KeyCode.Esc do
            switch event.key
                case KeyCode.Esc do
                end;
            end;
        end;
    end;
    
    switch event.key
        case KeyCode.Esc do
        end;
    end;
end;

Example::DoLoop(auto: value)
begin
    do
        if value <= 0 then
            return;
        endif;
        
        value += -1;
    loop;
end;

Example::ForNext(value)
begin
    var count = 0;
    
    for count = 0; value <= 0; value -= 1 do
        count += 1;
    next;
    
    for ; value <= 0; value -= 1 do
        count += 1;
    next;
    
    for ; value <= 0; do
        count += 1;
        value -= 1;
    next;
    
    for ;; do
    next;
    
    return count;
end;

export Example:MyLongNameExample(auto:value)
begin
    local piTest = #[2*pi]:0;
    hp::storeScreen;
    
    struct MyStruct
        one[1];
        two[2];
    end;
    
    struct MyStruct auto:myStruct;
    
    Example::DoLoop(Example::Switch);
  
    myStruct = {value, Example::ForNext};
    return myStruct.one + myStruct.two;
    
    if value <= π then
        KILL;
    endif;
    
    R→B(value,4)=>value;
    
    
    local t = IP(0.06);
    var t = <string>(<int>(#[#[1+1]^-4]:2));
  
    
end;



