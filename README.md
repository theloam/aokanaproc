2022 note:
It's been more than 2 years since I touched the game or created
this (even the rest of the readme was for a release that I ended
up postponing until today, oops...), so the opus stuff and where
it's used might be a bit inaccurate, but the patches work so
there's that. From what I remember, the game doesn't even use
opus except for the physical release, but the DLL needs to
export the functions anyways or the game might freak out.

Anyways, I'm releasing this code due to popular demand.
I haven't even tried compiling it again, but it should work.
In any case, patches are welcome.

# libaokanaproc

Partial reimplementation of aokanaproc.dll found in NekoNyan's
Aokana and Aokana EXTRA1 ports to Unity, mainly to be able
to run them on linux natively.

You need to have libwebp for 32bit (i386) installed, as well as
opus/opusfile for 32bit if you're building for Aokana EXTRA1.

Compile without opus:
```
gcc -o libaokanaproc.so -m32 -O2 -fPIC -shared -lwebp -L --exclude-libs,ALL
```

Compile with opus (Aokana EXTRA1)
```
gcc -c -m32 -O2 -fPIC -I/usr/include/opus -DAOKANA_EXTRA1 aokanaproc.c`
`gcc -m32 -shared aokanaproc.o -o libaokanaproc.so -Wl,-whole-archive -lopus -lopusfile -lwebp
```

```
gcc -shared -o libaokanaproc.so -m32 -O2 -fPIC -I/usr/include/opus -DAOKANA_EXTRA1 aokanaproc.c -Wl,-whole-archive -lopus -lopusfile -lwebp -Wl,-no-whole-archive
```

```
gcc -fPIC -shared -o libaokanaproc.so aokanaproc.c -lopus -opusfile -lwebp
```

Note (only for EXTRA1): opus include dir might be different, mine
for example is at `/usr/include/opus`. It must be manually defined
because it tries to include some files without the preceding opus
directory.
