#include "../framework.h"

// Forward decalring
static void PR_SortVertices(float *x1, float *y1, float *x2, float *y2, float *x3, float *y3);
void PR_TriFill(float x1, float y1, float x2, float y2, float x3, float y3, int colorId);


// Helper function to sort three vertices by their Y coordinate
static void PR_SortVertices(float *x1, float *y1, float *x2, float *y2, float *x3, float *y3) {
    float tx, ty;
    if (*y1 > *y2) { tx = *x1; *x1 = *x2; *x2 = tx; ty = *y1; *y1 = *y2; *y2 = ty; }
    if (*y1 > *y3) { tx = *x1; *x1 = *x3; *x3 = tx; ty = *y1; *y1 = *y3; *y3 = ty; }
    if (*y2 > *y3) { tx = *x1; *x1 = *x3; *x3 = tx; ty = *y2; *y2 = *y3; *y3 = ty; }
}

// Software Triangle Rasterizer utilizing horizontal scanline spans
void PR_TriFill(float x1, float y1, float x2, float y2, float x3, float y3, int colorId) {
    // 1. Sort vertices so that y1 <= y2 <= y3
    PR_SortVertices(&x1, &y1, &x2, &y2, &x3, &y3);

    // Guard gate: Avoid rendering completely flat or zero-pixel height triangles
    if ((int)y1 == (int)y3) return;

    // 2. Process Flat-Bottom part (from y1 to y2)
    if ((int)y1 != (int)y2) {
        float invslope1 = (x2 - x1) / (y2 - y1);
        float invslope2 = (x3 - x1) / (y3 - y1);

        float curx1 = x1;
        float curx2 = x1;

        for (int scanline = (int)y1; scanline <= (int)y2; scanline++) {
            PR_Line(curx1, (float)scanline, curx2, (float)scanline, colorId);
            curx1 += invslope1;
            curx2 += invslope2;
        }
    }

    // 3. Process Flat-Top part (from y2 to y3)
    if ((int)y2 != (int)y3) {
        float invslope1 = (x3 - x1) / (y3 - y1);
        float invslope2 = (x3 - x2) / (y3 - y2);

        float curx1 = x3 - ((y3 - y2) * invslope1);
        float curx2 = x2;

        for (int scanline = (int)y2; scanline <= (int)y3; scanline++) {
            PR_Line(curx1, (float)scanline, curx2, (float)scanline, colorId);
            curx1 += invslope1;
            curx2 += invslope2;
        }
    }
}
