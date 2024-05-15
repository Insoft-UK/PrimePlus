# P+
P+ is a proprietary programming language designed to mitigate the challenges associated with code maintenance and comprehensibility in the HP Programming Language (PPL). P+ serves as an intermediary language that addresses these issues by introducing support for pre-processing and facilitating code organization.

**Variable & Constant**

Using longer, more meaningful variable and function names can enhance code readability and ease maintenance. However, when targeting the HP Prime's PPL (Programming Language) with UTF16-LE text-based files, employing lengthy names can lead to larger .hpprgm files, which poses a downside. This is where substitution proves beneficial in P+.
```
var a:indexA;
const b:indexB:=1;
indexA += indexB; // Using subtitution.
a := a+b; // PPL results in a smaller .hpprgm file.
```
**for...next**
```
var I:index
for index:=0; index<10; index++ do
    // statement/s
next
```
**do...loop**
```
do
    // statement/s
loop
```
**switch**
```
var m:menuSelection;
switch menuSelection
    case 0 do
        // statement/s
    end
end
```
**if** _condition_ **do** _statement/s_ **else** _statement/s_ **endif**
```
var e:hasError = true;
if hasError==true do
    // statement/s
else
    // statement/s
endif
```
**if** _condition_ **then return**
```
if text=="" then return // end; is implied and optional.
```
**guard** _condition_ **else** _statement/s_ **end**
```
guard key != KeyCode.Esc else
    return
end
```
**while...wend**
```
var r:isRunning = true
while isRunning == true do
    // statement/s
    isRunning = false
wend
```
**(** _condition_ **?** _true_ **:** _false_**)**
```
var a:myValue = 0
a = (X>Y ? 1 : 0);
```
**def eval:... name(...);**
```
var auto:alpha;
/// It is necessary to evaluate, as we are referencing a defined alias 'alpha' in the definition.
def eval:alpha := a setAlpha(a);
setAlpha(50.0);
```
**#pragma**
```
// Turn off C style bitwise operators :- ! ^ can now be used as math operations.
#pragma ( bitwise 0 )
// Minify your .hpprgrm file footprint
#pragma ( minify -1 )
// Turns off the automatic descending ordering of identities/aliases, once off it can't be turned back on.
#pragma ( unorderedness )
// Turns on/off pre-calc for assignments, off by default.
#pragma ( calc 1 )
```
**Obj-C Style**
```
[Color ConvertToHSVFromRed:255 blue:127 green:0];

// Instead of:
Color::ConvertToHSV(255,127,0);
```
**Pre-Clac**
```
#pragma ( calc 1 )
#define SCREEN_WIDTH 320
var a = SCREEN_WIDTH / (8 - 6);
var b;
b = #[SCREEN_WIDTH / 2]; // Pre-Calc #[]
// PPL b:=160;
```
> [!TIP]
In P+ the use of **;** after end, endif, wend, loop ... is optional as the pre-processor will automatically include them if omitted for to allow cleaner looking code to be written.

> [!TIP]
In P+ the use of **=** for := is optional as the pre-processor will automatically covert all = to := Pascal & PPL style.

> [!IMPORTANT]
In P+ **=** is treated as := were in PPL **=** is treated as ==

>[!NOTE]
The P+ proprietary programming language is susceptible to change, while also maintaining full compatibility with previous versions.
