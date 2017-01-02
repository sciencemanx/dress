# dress

> add symbols back into a stripped ELF binary

# Usage

> ./dress &lt;in-file&gt; &lt;out-file&gt; &lt;sym-file&gt; [-v]

* `in-file`: path to the input (stripped) ELF binary
* `out-file`: path to the desired output location of the dressed binary; this file will have the same permissions as `in-file`
* `sym-file`: path to the symbol file
* `-v`: turns on verbose output; this is generally not helpful unless something is going wrong

# Symbol file format

Currently, there are two types of symbols: global variables and function names. By default, all symbols are globals. However, adding parentheses after the symbol name designates it as a function. The address of the symbol is indicated after the @ sign with an asterix. The address can be in either base 16 or base 10.

## Example `a.syms`:
``` javascript
counter @ *0x601044
main() @ *0x400593
test() @ *0x400566
```

# Known issues and limitations

* While Binary Ninja handles most global variables as intended, IDA has issues recognizing the symbol names. This is most likely due to the improper labeling of corresponding section for their symbols. Ability to specify sections for symbols will be added in the future.
* This version of dress is only compatible with 64 bit ELF files.
