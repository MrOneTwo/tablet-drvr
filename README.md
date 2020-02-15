Based on the Handmade Hero streams.

Last episode implemented:  Day 005

## Useful commands

Make a drive out of a directory:

```
subst w: j:\work
```

Running Visual Studio through cmd line (this will make a dummy solution):

```
devenv main.exe
```

Finding files

```
dir /s xinput1_3.dll
```

## Preface

On Windows run _shell.bat_. This will make the compiler accessible in the shell.

It may say that there is no _vcvarsall.bat_. ATM this file is in (it depends on the Visual Studio installed; you might have to poked around to find it):

```
C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build
```

Run ```devenv main.exe``` and set working directory to the _data_ folder.

## Building

There is a _bat_ script called _build.bat_ in the _code_ folder. This script runs the compiler.


## Visual Studio debugger

F11 - start and break on the entry point