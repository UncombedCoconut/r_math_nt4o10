This is a tool to create plots of the function B(x) defined in [this Reddit thread](https://old.reddit.com/r/math/comments/nt4o10/a_pretty_cool_but_completely_useless_function_i/)

The program takes 7 parameters (base, image width, low/high x values, image height, low/high y values) and emits a PGM file.
Warning: PGM is a very inefficient image format; at very high resolutions you'll need 1/4 GB available. External tools can compress this to something reasonable.

The included example was produced as follows:
```
gcc reddit_bx.c -o reddit_bx -lm
./bx 10 16383 1 10 16383 0 10
convert 'base 10 16383x16383 [1, 10]x[0, 10].pgm' 'base 10 16383x16383 [1, 10]x[0, 10].png'
```

A similar example using base 3 and smaller x/y boundaries is included for comparison.
