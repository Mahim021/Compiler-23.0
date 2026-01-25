# Simple Lexer - Beginner Friendly

## 📖 What This Does

This is a **simple tokenizer** that reads code from `input.txt` and identifies:
- Keywords: `if`, `else`, `while`, `int`
- Operators: `+`, `-`, `*`, `=`
- Numbers: `5`, `10`, `42`
- Identifiers: `x`, `sum`, `counter`
- Delimiters: `;`, `(`, `)`, `{`, `}`

## 🔧 How to Build and Run

### Step 1: Build the Lexer
**Windows:**
```powershell
flex simple_lexer.l
gcc lex.yy.c -o simple_lexer.exe
```

**Linux/Mac:**
```bash
flex simple_lexer.l
gcc lex.yy.c -o simple_lexer
```

### Step 2: Run It
**Windows:**
```powershell
.\simple_lexer.exe
```

**Linux/Mac:**
```bash
./simple_lexer
```

### Step 3: Check Output
Open `output.txt` to see all the tokens!

## 📂 Files

- `simple_lexer.l` - The Flex source code (heavily commented)
- `input.txt` - Your code to analyze
- `output.txt` - Results (generated automatically)

## 🎯 Understanding the Code

### The `.l` file has 5 sections:

1. **Options** - Configure Flex behavior
2. **C Code** - Variables and includes
3. **Definitions** - Name patterns (like shortcuts)
4. **Rules** - What to do when patterns match
5. **Main** - Opens files and runs the lexer

### Key Concepts Demonstrated:

✅ **Regular Definitions** - Reusable patterns
```
DIGIT  [0-9]
NUMBER {DIGIT}+
```

✅ **yytext** - The matched text
```c
"if" { printf("Found: %s", yytext); }  // yytext = "if"
```

✅ **yyin** - Input file
```c
yyin = fopen("input.txt", "r");
```

✅ **Pattern Matching Order** - First match wins!
- Keywords MUST come before identifiers
- Why? "if" is also a valid identifier pattern

## 🧪 Try This

1. Edit `input.txt` with your own code
2. Run the lexer again
3. Check `output.txt` for results

### Example Input:
```
int age = 25;
if (age) {
    age = age + 1;
}
```

### Example Output:
```
KEYWORD: int (Line 1)
IDENTIFIER: age (Line 1)
OPERATOR: ASSIGN (Line 1)
NUMBER: 25 (Line 1)
DELIMITER: SEMICOLON (Line 1)
...
```

## 📝 Exercise Ideas

1. Add new keyword: `return`
2. Add new operator: `==` (equal comparison)
3. Count how many identifiers you found
4. Support float numbers: `3.14`

## 🐛 Common Issues

**"Cannot open input.txt"**
- Make sure `input.txt` is in the same folder as the executable

**"flex: command not found"**
- Install Flex first

**"gcc: command not found"**
- Install GCC/MinGW first

## 🚀 Next Steps

Once you understand this:
1. Add more token types
2. Handle comments
3. Add string literals
4. Build a symbol table
5. Move on to parser (Bison)
