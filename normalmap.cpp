#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if defined(WIN32)
#  include "glut.h"
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include "normalmap.h"

/*
** 高さマップをもとに法線マップを作成する
*/
void makeNormalMap(GLubyte *tex, int width, int height, double nz, const char *name)
{
  FILE *fp = fopen(name, "rb");
  
  if (fp) {
    unsigned char *map = (unsigned char *)malloc(width * height);
    
    if (map) {
      unsigned long size = width * height;
      
      /* 高さマップを読み込む */
      fread(map, height, width, fp);
      fclose(fp);
      
      for (unsigned long y = 0; y < size; y += width) {
        for (int x = 0; x < width; ++x) {
          /* 隣接する画素との値の差を法線ベクトルの成分に用いる */
          double nx = map[y + x] - map[y + (x + 1) % width];
          double ny = map[y + x] - map[(y + width) % size + x];
          /* 法線ベクトルの長さを求めておく */
          double nl = sqrt(nx * nx + ny * ny + nz * nz);
          
          *(tex++) = (GLubyte)(nx * 127.5 / nl + 127.5);
          *(tex++) = (GLubyte)(ny * 127.5 / nl + 127.5);
          *(tex++) = (GLubyte)(nz * 127.5 / nl + 127.5);
          *(tex++) = 255;
        }
      }
      
      free(map);
    }
  }
}
