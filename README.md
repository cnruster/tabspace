# Introducing TabSpace

Yiping Cheng

Beijing Jiaotong University, China. Email: [ypcheng@bjtu.edu.cn](mailto:ypcheng@bjtu.edu.cn)

[https://www.researchgate.net/profile/Yiping-Cheng/research](https://www.researchgate.net/profile/Yiping-Cheng/research)

[https://github.com/cnruster](https://github.com/cnruster)

Different text readers/editors have different interpretations of Tab, there are roughly two schools of thought: "1 Tab = 8 spaces" and "1 Tab = 4 spaces". So, if you use Tab, the program that originally looked great in your editor will appear messy elsewhere. Therefore, many programmers tend to use space-only for guaranteed visual look, but that will cause a lot of waste of disk space. Is there an ideal solution?

There IS!

We can enforce a rule to all program files:

**In each line, before the first non-space, non-tab character, use TAB only! And after that character, use SPACE only!**

The advantage of this rule is that, first of all, regardless of how many spaces the tab's interpretation is, the code is always uniformly indented, and since tabs and spaces do not mix together, the appearance of the code line is fixed. Secondly, compared to the all-space solution, it can reduce file size dramatically. I consider this to be the perfect solution.

Therefore I wrote a small utility TabSpace (currently, only under Windows) for this purpose: It helps programmers to convert source files to files that comply with the above tab-space rule. TabSpace performs this conversion on all the files within the current directory (including all its subdirectories) that match particular patterns you provide. For example, if you want to convert Go files and Python files, simply type:

Tabspace \*. go \*. py

The code beautification functions of TabSpace include:

1. Tab-Space rule enforcement

2. Convert white space (isspace returns true) characters to standard space (0x20)

3. Remove trailing spaces in a line

4. Standardize all line breaks to the UNIX standard: single LF

If the conversion is successful, the original file will be renamed to a backup file with the same basic file name and an extension of the original extension plus bak. So this program won't let you lose anything.

In TabSpace, when converting a tab to spaces or vice versa, it is stipulated that 1 Tab = 4 spaces. This rule is currently fixed. A majority of coding guidelines from major software companies and major languages also enforce this rule.

TabSpace refuses to perform conversion for the patterns "\*" and ".\*", since the conversion is meaningful only for code files.

So just enjoy! Should you find any bugs, feel free to contact me via the address provided above.

**Jan 21, 2024 update: Some programmers still like the aligned all-space rule I previously adopted before thinking of the tab-space rule, as they are are willing to trade the waste of disk space for absolute visual consistency. So I now readd the aligned all-space rule. Aligned means the number of leading spaces (before first non-space non-tab character) is always a multiple of 4.**
