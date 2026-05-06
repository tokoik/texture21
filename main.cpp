#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if defined(_WIN32)
//#  pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#  include "glut.h"
#  include "glext.h"
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLMULTITEXCOORD3DVPROC glMultiTexCoord3dv;
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/glut.h>
#endif

/* トラックボール処理用関数の宣言 */
#include "trackball.h"

/* 球を描く関数の宣言 */
#include "sphere.h"

/* 法線マップ */
#include "normalmap.h"

/* 正規化マップ */
#include "normalizemap.h"

/*
** 光源
*/
static const GLfloat lightpos[] = { 4.0, 5.0, 6.0, 1.0 }; /* 位置　　　 */
static const GLfloat lightcol[] = { 1.0, 1.0, 1.0, 1.0 }; /* 直接光強度 */
static const GLfloat lightamb[] = { 0.1, 0.1, 0.1, 1.0 }; /* 環境光強度 */

/*
** テクスチャ
*/
#define TEXWIDTH  256                      /* テクスチャの幅　　　 */
#define TEXHEIGHT 256                      /* テクスチャの高さ　　 */

/*
** 初期化
*/
static void init(void)
{
#if defined(_WIN32)
  glActiveTexture =
    (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
  glMultiTexCoord3dv =
    (PFNGLMULTITEXCOORD3DVPROC)wglGetProcAddress("glMultiTexCoord3dv");
#endif
  
  /* テクスチャ画像はワード単位に詰め込まれている */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  
  /*
  ** テクスチャユニット０に法線マップを設定する
  */
  GLubyte texture[TEXHEIGHT * TEXWIDTH * 4];
  
  /* 法線マップの作成 */
  makeNormalMap(texture, TEXWIDTH, TEXHEIGHT, 20.0, "dotbump.raw");
  
  /* テクスチャの割り当て */
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXWIDTH, TEXHEIGHT, 0,
    GL_RGBA, GL_UNSIGNED_BYTE, texture);
  
  /* テクスチャを拡大・縮小する方法の指定 */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  /* テクスチャの繰り返し方法の指定 */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  /* テクスチャユニット０のテクスチャ環境 */
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  
  /*
  ** テクスチャユニット１正規化マップを設定する
  */
  glActiveTexture(GL_TEXTURE1);
  
  /* テクスチャの読み込みに使う配列 */
  static GLubyte t[6][128 * 128 * 4];
  static GLubyte *normalize[] = { t[0], t[1], t[2], t[3], t[4], t[5] };
  
  /* 正規化マップの作成 */
  makeNormalizeMap(normalize, 128, 128);
  
  for (int i = 0; i < 6; ++i) {
    /* テクスチャのターゲット名 */
    static const int target[] = {
      GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
      GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
      GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
      GL_TEXTURE_CUBE_MAP_POSITIVE_X,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    };
    
    /* キューブマッピングのテクスチャの割り当て */
    glTexImage2D(target[i], 0, GL_RGBA, 128, 128, 0, 
      GL_RGBA, GL_UNSIGNED_BYTE, normalize[i]);
  }
  
  /* テクスチャを拡大・縮小する方法の指定 */
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  /* テクスチャの繰り返し方法の指定 */
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  /* テクスチャユニット１のテクスチャ環境 */
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
  
  /* テクスチャユニット０に戻す */
  glActiveTexture(GL_TEXTURE0);
  
  /* 初期設定 */
  glClearColor(0.3, 0.3, 1.0, 0.0);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
}

/*
** シーンの描画
*/
static void scene(void)
{
  /* 法線マップのマッピング開始 */
  glEnable(GL_TEXTURE_2D);
  
  /* 正規化マップのマッピング開始 */
  glActiveTexture(GL_TEXTURE1);
  glEnable(GL_TEXTURE_CUBE_MAP);
  
  /* トラックボール処理による回転 */
  glMultMatrixd(trackballRotation());
  
  /* 球を描く */
  sphere(1.0, 64, 32, lightpos);
  
  /* 正規化マップのマッピング終了 */
  glDisable(GL_TEXTURE_CUBE_MAP);
  
  /* 法線マップのマッピング終了 */
  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_2D);
}


/****************************
** GLUT のコールバック関数 **
****************************/

static void display(void)
{
  /* モデルビュー変換行列の設定 */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  /* 光源の位置を設定 */
  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
  
  /* 視点の移動（物体の方を奥に移動）*/
  glTranslated(0.0, 0.0, -5.0);
  
  /* 画面クリア */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  /* シーンの描画 */
  scene();
  
  /* ダブルバッファリング */
  glutSwapBuffers();
}

static void resize(int w, int h)
{
  /* トラックボールする範囲 */
  trackballRegion(w, h);
  
  /* ウィンドウ全体をビューポートにする */
  glViewport(0, 0, w, h);
  
  /* 透視変換行列の指定 */
  glMatrixMode(GL_PROJECTION);
  
  /* 透視変換行列の初期化 */
  glLoadIdentity();
  gluPerspective(40.0, (double)w / (double)h, 0.1, 10.0);
}

static void idle(void)
{
  /* 画面の描き替え */
  glutPostRedisplay();
}

static void mouse(int button, int state, int x, int y)
{
  switch (button) {
  case GLUT_LEFT_BUTTON:
    switch (state) {
    case GLUT_DOWN:
      /* トラックボール開始 */
      trackballStart(x, y);
      glutIdleFunc(idle);
      break;
    case GLUT_UP:
      /* トラックボール停止 */
      trackballStop(x, y);
      glutIdleFunc(0);
      break;
    default:
      break;
    }
    break;
    default:
      break;
  }
}

static void motion(int x, int y)
{
  /* トラックボール移動 */
  trackballMotion(x, y);
}

static void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case 'q':
  case 'Q':
  case '\033':
    /* ESC か q か Q をタイプしたら終了 */
    exit(0);
  default:
    break;
  }
}

/*
** メインプログラム
*/
int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  init();
  glutMainLoop();
  return 0;
}
