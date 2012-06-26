#include <math.h>
#if defined(WIN32)
#  include "glut.h"
#  include "glext.h"
extern PFNGLMULTITEXCOORD2DPROC glMultiTexCoord2d;
extern PFNGLMULTITEXCOORD3DVPROC glMultiTexCoord3dv;
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/glut.h>
#endif

#include "sphere.h"
#include "matrix.h"

#define PI 3.14159265358979323846

/*
** ローカル座標系から n を法線ベクトルとする接空間の座標系への変換行列 t を求める
*/
static void localToTangent(const double n[3], double t[16])
{
  double l = n[0] * n[0] + n[2] * n[2];
  double a = sqrt(l);

  /* 接空間のＸ軸 = (0, 1, 0) × n */
  if (a > 0) {
    t[ 0] = n[2] / a;
    t[ 8] = -n[0] / a;
  }
  else {
    t[ 0] = 0.0;
    t[ 8] = 0.0;
  }
  t[ 4] = 0.0;
  t[ 3] = 0.0;

  /* 接空間のＹ軸 = Ｚ軸 (= n) × Ｘ軸 */
  t[ 1] = -n[1] * n[0];
  t[ 5] = l;
  t[ 9] = -n[1] * n[2];
  a = sqrt(t[ 1] * t[ 1] + t[ 5] * t[ 5] + t[ 9] * t[ 9]);
  if (a > 0) {
    t[ 1] /= a;
    t[ 5] /= a;
    t[ 9] /= a;
  }
  else {
    t[ 1] = 0.0;
    t[ 5] = 0.0;
    t[ 9] = 0.0;
  }
  t[ 7] = 0.0;

  /* 接空間のＺ軸 (= n) */
  t[ 2] = n[0];
  t[ 6] = n[1];
  t[10] = n[2];
  t[11] = 0.0;

  t[12] = 0.0;
  t[13] = 0.0;
  t[14] = 0.0;
  t[15] = 1.0;
}

/*
** 正規化マップのテクスチャ座標を設定する
** 　n: その点における法線ベクトル
** 　p: その点の位置
** 　ll: ローカル座標系における光源位置
*/
static void normalizeTexCoord(double n[], double p[], double ll[])
{
  /* 接空間における光源位置 */
  double lt[4] = { ll[0], ll[1], ll[2], ll[3] };
  /* ローカル座標系から接空間の座標系への変換行列 */
  double t[16];

  /* 平行光線でなければ光源位置の実座標を求めておく */
  if (lt[3] != 0.0) {
    lt[0] = lt[0] / lt[3] - p[0];
    lt[1] = lt[1] / lt[3] - p[1];
    lt[2] = lt[2] / lt[3] - p[2];
  }

  /* ローカル座標系から接空間の座標系への変換行列を求める */
  localToTangent(n, t);
  
  /* 接空間における光源位置を求める */
  transform(lt, t, lt);
  
  /* 接空間における光源位置をテクスチャ座標に設定する */
  glMultiTexCoord3dv(GL_TEXTURE1, lt);
}

/*
** 球の描画
*/
void sphere(double radius, int slices, int stacks, const float l[])
{
  /* ローカル座標系における光源位置 */
  double ll[4] = { l[0], l[1], l[2], l[3] };
  /* モデルビュー変換行列の逆行列 */
  double m[16];
  
  /* 現在のモデルビュー変換行列の逆行列を求める */
  glGetDoublev(GL_MODELVIEW_MATRIX, m);
  inverse(m, m);
  
  /* ローカル座標系における光源位置を求める */
  transform(ll, m, ll);
  
  /* 球を描く */
  for (int j = 0; j < stacks; ++j) {
    double t0 = (double)j / (double)stacks;
    double t1 = (double)(j + 1) / (double)stacks;
    double r0 = sin(PI * t0);
    double r1 = sin(PI * t1);
    double n[2][3], p[2][3];
    
    /* 法線単位ベクトルの y 成分 */
    n[0][1] = -cos(PI * t0);
    n[1][1] = -cos(PI * t1);
    
    /* 頂点の y 座標値 */
    p[0][1] = radius * n[0][1];
    p[1][1] = radius * n[1][1];
    
    /* 法線マップのテクスチャ座標の算出 */
    t0 *= 4.0;
    t1 *= 4.0;
    
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; ++i) {
      double s = (double)i / (double)slices;
      double a = -2.0 * PI * s;
      
      /* 法線単位ベクトルの x, z 成分 */
      n[0][0] = r0 * cos(a);
      n[0][2] = r0 * sin(a);
      n[1][0] = r1 * cos(a);
      n[1][2] = r1 * sin(a);
      
      /* 頂点の x, z 座標値 */
      p[0][0] = radius * n[0][0];
      p[0][2] = radius * n[0][2];
      p[1][0] = radius * n[1][0];
      p[1][2] = radius * n[1][2];
      
      /* 法線マップのテクスチャ座標の算出 */
      s *= 8.0;
      
      /* 法線マップのテクスチャ座標を設定する */
      glTexCoord2d(s, t0);
      
      /* 正規化マップのテクスチャ座標を設定する */
      normalizeTexCoord(n[0], p[0], ll);
      
      /* 拡散反射係数のテクスチャ座標を設定する */
      glMultiTexCoord2d(GL_TEXTURE2, s, t0);
      
      /* 頂点位置 */
      glVertex3dv(p[0]);
      
      /* 法線マップのテクスチャ座標を設定する */
      glTexCoord2d(s, t1);
      
      /* 正規化マップのテクスチャ座標を設定する */
      normalizeTexCoord(n[1], p[1], ll);
      
      /* 拡散反射係数のテクスチャ座標を設定する */
      glMultiTexCoord2d(GL_TEXTURE2, s, t1);
      
      /* 頂点位置 */
      glVertex3dv(p[1]);
    }
    glEnd();
  }
}
