#include "renderGDI.h"

CRender* CRender::create(RenderType type)
{
    return new CRenderGDI;
}
