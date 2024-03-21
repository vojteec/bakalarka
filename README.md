# Digital electronics 1
## Vojtěch Kuchař
### Summer semester of 2023
_This is written in cursive._<br>
__This is a strong message.__<br>
- This is an unondered list
- And his items
- And even the third one<br>

There can be more than __one__ type of _list_.<br>
1. Ordered list
2. His second part
3. And even also the third part

[This is a link to a well-known internet browser – Google.](https://google.com "Google.com")

|This|Is|A|Table|
|----|--|-|-----|
|This|Is|A|Table|
|Of  |A |Few|Rows|
|And |Collumns|.|.|

```vhdl
architecture dataflow of gates is
begin
    f_orig_o <= (not(b_i) and a_i) or (c_i and not(b_i or not(a_i)));
    f_nand_o <= (not(b_i) and a_i) or (c_i and ((not b_i) and a_i));
    f_nor_o  <= not(b_i or not(a_i)) or (c_i and not(b_i or not(a_i)));
end architecture dataflow;
```
This is the end of this README file.
