#ifndef SGLRASTER_H
#define SGLRASTER_H

class SGLColor;
class SGLObject;
class SGLFaceSet;
void sglClearColor(const SGLColor& color);
void sglDrawPoint(int x, int y, const SGLColor& color);
void sglDrawLine(int x0, int y0, int x1, int y1, const SGLColor& color);
void sglDrawTriangleWithFrame(int x1, int y1, int x2, int y2, int x3, int y3, const SGLColor& color);
void sglDrawTriangleWithFlatShade(int x1, int y1, int x2, int y2, int x3, int y3, const SGLColor& color);
void sglDrawTriangleWithGShade(int x1, int y1, int x2, int y2, int x3, int y3, const SGLColor& c1, const SGLColor& c2, const SGLColor& c3);
void sglDrawTriangleWithTexture(int textureId, int x1, int y1, int x2, int y2, int x3, int y3, float tex_x1, float tex_y1, float tex_x2, float tex_y2, float tex_x3, float tex_y3);
void sglDrawObject(const SGLObject& obj);
void sglDrawFaceSet(const SGLFaceSet& faceset);

#endif