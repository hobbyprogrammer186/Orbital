<p align="center">
  <img src="./static/banner.png" alt="Orbital Banner" />
</p>

# Orbital
Easier To Use Programming Language

## Features
- Easier To Use
- Semicolouns Are Optional
- Console With Inline Suggestion

## Incomplete Feature/Functionality
- `else`, `elif`, `switch`, `class`, `case`, etc Statement. But `and`, `or`, `not`, `True`, `False`, `Greater`, `Shorter`, etc. Is Already Implemented
- File Interpreting

# Contribute
Build And Test The Source Code:
```bash
mkdir build && cd build
cmake ..
make
./src/orbital
```
Create The Rich Text Documention
```
mkdir build && cd build
cmake ..
make docs
```
And Docs Is Located At `build/docs/`

- `io/orbtlio.cpp`: Predefiend Standard Functions.
- `include/`: Header Files Of This Interpreter
- `docs/`: Documentions Of This Programming Language (How To Use?)
- `src/`: Main Program And Perser/Lexer Is Located.