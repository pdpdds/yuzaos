#include "SGLState.h"
#include "SGLConst.h"

namespace g
{
	SGLState sglState;
};

void sglEnable(int e)
{
	if(e == SGL_ZBUFFER)
	{
		g::sglState.enabledZBuffer = 1;
		g::sglState.enabledZDepthSort = 0;
	}
	else
	if(e == SGL_ZDEPTHSORT)
	{
		g::sglState.enabledZBuffer = 0;
		g::sglState.enabledZDepthSort = 1;
	}
	else
	if(e == SGL_LIGHT)
	{
		g::sglState.enabledLight = 1;
	}
	else
	if(e == SGL_TEXTURE)
	{
		g::sglState.enabledTexture = 1;
	}
	else
	if(e == SGL_ALPHABLEND)
	{
		g::sglState.enabledAlphablend = 1;
	}
	else
	if(e == SGL_CLIP_3D)
	{
		g::sglState.enabledClip3D = 1;
	}
	else
	if(e == SGL_BACKFACE_CULL)
	{
		g::sglState.enabledBackfaceCull = 1;
	}
	else
	if(e == SGL_OBJECT_CULL)
	{
		g::sglState.enabledObjectCull = 1;
	}
}
void sglDisable(int e)
{
	if(e == SGL_ZBUFFER)
	{
		g::sglState.enabledZBuffer = 0;
	}
	else
	if(e == SGL_ZDEPTHSORT)
	{
		g::sglState.enabledZDepthSort = 0;
	}
	else
	if(e == SGL_LIGHT)
	{
		g::sglState.enabledLight = 0;
	}
	else
	if(e == SGL_TEXTURE)
	{
		g::sglState.enabledTexture = 0;
	}
	else
	if(e == SGL_ALPHABLEND)
	{
		g::sglState.enabledAlphablend = 0;
	}
	else
	if(e == SGL_CLIP_3D)
	{
		g::sglState.enabledClip3D = 0;
	}
	else
	if(e == SGL_BACKFACE_CULL)
	{
		g::sglState.enabledBackfaceCull = 0;
	}
	else
	if(e == SGL_OBJECT_CULL)
	{
		g::sglState.enabledObjectCull = 0;
	}
}
bool sglGetState(int e)
{
	if(e == SGL_ZBUFFER)
	{
		return g::sglState.enabledZBuffer;
	}
	else
	if(e == SGL_ZDEPTHSORT)
	{
		return g::sglState.enabledZDepthSort;
	}
	else
	if(e == SGL_LIGHT)
	{
		return g::sglState.enabledLight;
	}
	else
	if(e == SGL_TEXTURE)
	{
		return g::sglState.enabledTexture;
	}
	else
	if(e == SGL_ALPHABLEND)
	{
		return g::sglState.enabledAlphablend;
	}
	else
	if(e == SGL_CLIP_3D)
	{
		return g::sglState.enabledClip3D;
	}
	else
	if(e == SGL_BACKFACE_CULL)
	{
		return g::sglState.enabledBackfaceCull;
	}
	else
	if(e == SGL_OBJECT_CULL)
	{
		return g::sglState.enabledObjectCull;
	}
	return 0;
}
void sglShadeMode(int s)
{
	g::sglState.shadeMode = s;
}
int  sglGetShadeMode(void)
{
	return g::sglState.shadeMode;
}