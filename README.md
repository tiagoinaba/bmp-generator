# BMP Generator

This is a project made for my computer graphics course in college. Reads a file
that contains a matrix of two bit numbers and spits out a `.bmp`.

The four possible colors are:

```
+--------+-----------+
| Number |   Color   |
+--------+-----------+
|   00   |  Yellow   |
|   01   |  Black    |
|   10   |  Blue     |
|   11   |  Green    |
+--------+-----------+
```

## Example

```bash
make
./draw image
```

## TODO

- Make the bitmap size dynamic. Right now it's capped at `3072` bytes.
    - Involves changing the implementation to two passes: one for determining
    the size of the input matrix (so that the data matrix can be allocated), and
    another for populating the data matrix.
- Accept output file name as a parameter.
