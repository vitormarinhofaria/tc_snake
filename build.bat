set EXE=build/snake
cl main.c /O2 /std:c17 /Fe%EXE%.exe /Fo%EXE%.obj /Fd /Zi /MTd /link User32.lib