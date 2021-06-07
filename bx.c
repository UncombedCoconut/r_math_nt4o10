#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DIGITS 48
const int BLACK = 0, GRAY = 0x80, WHITE = 0xff;

int digits[MAX_DIGITS];
int base, exp0;
int img_w, img_h;
uint8_t *img_data;
double img_c0, img_dc, img_r0, img_dr;

static inline int min(int a, int b) { return a<b ? a : b; }
static inline int max(int a, int b) { return a>b ? a : b; }
static inline double fmodpos(double x, double y) { return fmod(fmod(x, y) + y, y); }

int darken_rect(int c0, int c1, int r0, int r1, int color)
{
    int brightest = 0;
    for (int i = c0; i < c1; i++)
        for (int j = r0; j < r1; j++) {
            int index = img_w * (img_h-1-j) + i;
            img_data[index] = min(img_data[index], color);
            brightest = max(brightest, img_data[index]);
        }
    return brightest;
}

double expansion(int expi, double base_pos, double base_neg)
{
    double y = 0;
    for (int i = 0; i < expi; i++) {
        int e = exp0 - i;
        y += digits[i] * pow(e>0 ? base_pos : base_neg, e);
    }
    return y;
}

void search(double x0, int expi, double power)
{
    double x1;
    int c0, c1, r0_outer, r0_inner, r1_inner, r1_outer;
    x1 = x0 + base * power;
    /* Try to exit early, due to out-of-bounds. */
    c0 = max(floor(img_c0 + img_dc * x0), 0);
    c1 = min(ceil(img_c0 + img_dc * x1), img_w);
    if (c1 <= c0)
        return;
    /* Try to draw and exit. */
    if (c1-c0 == 1 || expi+1 == MAX_DIGITS) {
        if (x0 >= 1) {
            r0_outer = max(0,     floor(img_r0 + img_dr * expansion(expi, x0, x1)));
            r0_inner = max(0,     floor(img_r0 + img_dr * expansion(expi, x1, x0)));
            r1_inner = min(img_h,  ceil(img_r0 + img_dr * expansion(expi, x0, x1) + power*(base-1)/(x1-1)));
            r1_outer = min(img_h,  ceil(img_r0 + img_dr * expansion(expi, x1, x0) + power*(base-1)/(x0-1)));

            darken_rect(c0, c1, r0_inner, r1_inner, BLACK);
            if (!darken_rect(c0, c1, r0_outer, r1_outer, GRAY) || expi+1 == MAX_DIGITS)
                return;
        }
        else {
            /* Draw the exact point (x0, B(x0)) and see if any further decimal places will toss us out the window. */
            double y0 = x0 ? expansion(expi, x0, x0) : 0;
            r0_inner = max(0,     0+floor(img_r0 + img_dr * y0));
            r1_inner = min(img_h, 1+floor(img_r0 + img_dr * y0));

            darken_rect(c0, c0+1, r0_inner, r1_inner, BLACK);
            if (expi+1 == MAX_DIGITS)
                return;

            /* This takes a while. Explain that we aren't dead. */
            if (expi < 8) printf("Progress along (0,1): x=%g\n", x0);

            digits[expi] = 1;
            double y1_outer = expansion(expi+1, x0, x1);
            if (img_r0 + img_dr * y1_outer >= (double)img_h)
                return;
        }
    }
    /* Some unknowns remain and we haven't hit the iteration limit yet. */
    for (int d = 0; d < base; d++) {
        digits[expi] = d;
        search(x0 + d * power, expi + 1, power / base);
    }
}

void grid(double size, uint8_t color)
{
    for (double c = fmodpos(img_c0, img_dc * size); c < img_w; c += img_dc * size)
        darken_rect(c, c+1, 0, img_h, color);
    for (double r = fmodpos(img_r0, img_dr * size); r < img_h; r += img_dr * size)
        darken_rect(0, img_w, r, r+1, color);
}

int main(int argc, char **argv)
{
    /* Parse arguments: base w x0 x1 h y0 y1 */
    assert(argc == 8);
    base = atoi(argv[1]);
    img_w = atoi(argv[2]);
    double img_x0 = atof(argv[3]);
    double img_x1 = atof(argv[4]);
    img_h = atoi(argv[5]);
    double img_y0 = atof(argv[6]);
    double img_y1 = atof(argv[7]);
    assert(img_w > 0 && img_h > 0 && img_x0 < img_x1 && img_y0 < img_y1);
    /* Set up */
    img_dc = img_w / (img_x1 - img_x0);
    img_dr = img_h / (img_y1 - img_y0);
    img_c0 = -img_x0 * img_dc;
    img_r0 = -img_y0 * img_dr;
    img_data = malloc(img_w*img_h);
    exp0 = floor(log(img_x1) / log(base));

    /* Draw */
    memset(img_data, WHITE, img_w*img_h);
    search(0, 0, pow(base, exp0));
    for (int p = -2; p <= 2; p++)
        grid(pow(base, p), 0xc0 - p*0x1f);

    /* Save */
    char fn[4096];
    snprintf(fn, sizeof(fn), "base %d %dx%d [%g, %g]x[%g, %g].pgm", base, img_w, img_h, img_x0, img_x1, img_y0, img_y1);
    FILE *f = fopen(fn, "wb");
    fprintf(f, "P5\n%d %d\n255\n", img_w, img_h);
    fwrite(img_data, img_w, img_h, f);
    fclose(f);
    return 0;
}
