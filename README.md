# xv6 Custom Shell (myshell) — `my_shell.c`

This repository contains _only_ `my_shell.c`, an xv6 user-program that implements a custom shell.  
The xv6 source tree is intentionally _not included_ in this repository. To use this shell, follow the steps below to integrate it into a fresh xv6 checkout and build.

---

## Features
- Prompt: `>>>`
- Built-in `cd`
- Execute simple commands
- I/O redirection: `<` and `>`
- Pipes: `cmd1 | cmd2` (including multi-stage pipelines)
- Sequential commands using `;`
- Tolerant whitespace handling

---

## Requirements (sources)
- Shell source (this repo): https://github.com/<YOUR-USERNAME>/<YOUR-REPO>
- xv6-riscv: https://github.com/mit-pdos/xv6-riscv
- Apptainer (recommended): https://apptainer.org/  
  (Alternative: Singularity: https://sylabs.io/singularity/)

---

## How to build and run

### 1) Clone xv6
```bash
git clone https://github.com/mit-pdos/xv6-riscv.git
cd xv6-riscv
```

---

### 2) Enter a toolchain environment

If you have a local container image:

```bash
singularity shell xv6_tool.simg
```

Or pull a toolchain container and run it:

```bash
singularity pull docker://callaghanmt/xv6-tools:buildx-latest
singularity shell xv6-tools_buildx-latest.sif
```

---

### 3) Download this shell source and copy it into xv6

```bash
git clone https://github.com/<YOUR-USERNAME>/<YOUR-REPO>.git
cp <YOUR-REPO>/my_shell.c user/myshell.c
```

---

### 4) Register the program in the xv6 Makefile

Open `Makefile` in the xv6 root folder and add this line to `UPROGS`:

```makefile
$U/_myshell
```

---

### 5) Build and run xv6

```bash
make clean
make
make qemu
```

---


### 6) Run the custom shell inside xv6

In the xv6 terminal:

```sh
$ myshell
>>> ls
```

---

#### Exit

To quit QEMU: press `ctrl-a` then `x`.


---


## Notes

- This repo contains only `my_shell.c` to keep the public repository focused on the implementation.
- If your container image filename differs, replace `xv6_tool.simg` with your actual file name.

```mathematica 
Replace the placeholder repo URL once you publish it, and you are done.
::contentReference[oaicite:0]{index=0}
```
