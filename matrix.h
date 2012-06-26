#ifndef INC_MATRIX_H
#define INC_MATRIX_H

/*
** 行列計算
*/
extern void copyMatrix(const double m1[], double m2[]);
extern void loadIdentity(double m[]);
extern void transform(const double v1[], const double m[], double v2[]);
extern void multiply(const double m1[], const double m2[], double m3[]);
extern void translate(const double x, const double y, const double z, double t[]);
extern void rotate(const double a, const double x, const double y, const double z, double r[]);
extern int inverse(const double m1[], double m2[]);
extern void printMatrix(const double m[]);

#endif
