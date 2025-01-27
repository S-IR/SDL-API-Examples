CC = gcc
CFLAGS = -Wall -I"D:/utils/msys64/mingw64/include"
LDFLAGS = -L"D:/utils/msys64/mingw64/bin" -lSDL3

triangle: src/hello_triangle.c
	$(CC) $(CFLAGS) src/hello_triangle.c -o build/triangle.exe $(LDFLAGS)
