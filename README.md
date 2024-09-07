# P+
P+ is a proprietary programming language designed to mitigate the challenges associated with code maintenance and comprehensibility in the HP Programming Language (PPL). P+ serves as an intermediary language that addresses these issues by introducing support for pre-processing and facilitating code organization. The final output is a PPL program file with a minimal footprint, optimized to conserve the limited storage space available on the HP Prime.

**Variable & Constant**

Using longer, more meaningful variable and function names can enhance code readability and ease maintenance. However, when targeting the HP Prime's PPL (Programming Language) with UTF16-LE text-based files, employing lengthy names can lead to larger .hpprgm files, which poses a downside. This is where substitution proves beneficial in P+.

> [!NOTE]
Please look at using **PPL Minifier** to handle long names in some situations.
```
// PPL Minifier
var minimum;
// P+
var min:minimum;
// P+ for :=
var min:minimum := 0;
```

```
var a:indexA;
const b:indexB:=1;
indexA += indexB; // Using subtitution.
a := a+b; // PPL results in a smaller .hpprgm file.
```
> [!WARNING]
Deprecated: The use of **:=** will be dropped in v2.0 to simplifie the language and rely on the use of **PPL Minifier**.


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
> [!NOTE]
Advised to use PPL style **case** instead of P+ style.
```
var m:menuSelection;
switch menuSelection
    case 0 do
        // statement/s
    end
end
```
**if** _condition_ **do** _statement/s_ **else** _statement/s_ **endif**
> [!WARNING]
Deprecated: Please use **then** instead of **do**.
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
if text=="" then return; // end; is optional as the pre-processor will automatically include end; if omitted.
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
```
> [!WARNING]
Deprecated: `#pragma ( minify -1 )` Please start using **PPL Minifier**.


**Obj-C Style**

> [!WARNING]
Deprecated: It must now be enabled `#pragma ( messages )`.
```
[Color ConvertToHSVFromRed:255 blue:127 green:0];

// Instead of:
Color::ConvertToHSV(255,127,0);
```
**Pre-Calc**
```
#define SCREEN_WIDTH 320
var a = #[SCREEN_WIDTH / #[8 - 6]];
// PPL LOCAL a:=160;
var b = #[SCREEN_WIDTH / 4];
b = #[SCREEN_WIDTH / 2]:2; // Pre-Calc #[]:scale
// PPL b:=160.00;
b = <calc>(SCREEN_WIDTH / 2:2);
```
**Type Casting**

> [!NOTE]
It may be removed in later updates, experimental purposes.
```
var R = 0.6;
I = <int>(R);
S = <string>(R);
```

> [!TIP]
In P+ the use of **;** after end, endif, wend, loop ... is optional as the pre-processor will automatically include them if omitted.

> [!TIP]
In P+ the use of **=** for := is optional as the pre-processor will automatically covert all = to := Pascal & PPL style.

> [!IMPORTANT]
In P+ **=** is treated as := were in PPL **=** is treated as ==

>[!NOTE]
The P+ proprietary programming language is susceptible to change, while also maintaining full compatibility with previous versions.
