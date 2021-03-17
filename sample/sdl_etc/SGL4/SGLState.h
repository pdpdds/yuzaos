#ifndef SGLSTATE_H
#define SGLSTATE_H

typedef struct _SGLState
{
	struct
	{
		unsigned int enabledZBuffer		: 1;
		unsigned int enabledZDepthSort	: 1;
		unsigned int enabledLight		: 1;
		unsigned int enabledTexture		: 1;
		unsigned int enabledAlphablend	: 1;
		unsigned int enabledClip3D		: 1;
		unsigned int enabledBackfaceCull: 1;
		unsigned int enabledObjectCull  : 1;
	};
	int shadeMode;
}SGLState;

void sglEnable(int e);
void sglDisable(int e);
bool sglGetState(int e);
void sglShadeMode(int s);
int  sglGetShadeMode(void);
#endif