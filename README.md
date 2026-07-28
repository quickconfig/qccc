# QCCC

QCCC is the official command-line compiler for the QuickConfig language, built on top of LibQCC. It compiles `.qcf` source files into `.qbin` binaries, decompiles compiled binaries back into source, and formats QuickConfig files.

## Features

- Compile `.qcf` files to `.qbin`
- Decompile `.qbin` files back to `.qcf`
- Format individual files or entire directories
- Colored diagnostics with source highlighting
- Fast command-line interface powered by LibQCC
- Easy export to json and toml

## Commands

```bash
qccc compile <file.qcf>
qccc decompile <file.qbin>
qccc fmt [path="."] # file or folder
qccc export <json|toml> <file.qcf>
qccc version
qccc help
```

## Docs

See the QCCC documentation:

Open [docs/index.html](docs/index.html) in a browser to view the documentation site.

## Build

```bash
c++ main.cpp -lqcc -o qccc # For linux
```
