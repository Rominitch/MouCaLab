#include "Dependancies.h"

#include <LibRT/include/RTMesh.h>
#include <LibRT/include/RTBufferCPU.h>

#include <LibGUI/include/GUIWidget.h>

//Memory Leak Linker
#ifndef NDEBUG
#   ifdef DEBUG_NEW
#       undef DEBUG_NEW
#   endif
#   define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#   define new DEBUG_NEW
#endif

GUIWidget::GUIWidget(void):
m_pMesh(NULL)
{}

GUIWidget::~GUIWidget(void)
{
    Release();
}

void GUIWidget::Initialize(const RT::BoundingBox& bBox)
{
    Release();

    //Build Mesh
    m_pMesh = new RT::Mesh();

    RT::BufferCPUSPtr VBOQuad(new RT::BufferCPU());
    RT::GeoFloat* pVertexQuad = (RT::GeoFloat*)VBOQuad->create(RT::Mesh::getStandardVBOFormat(), 4);
    RT::GeoFloat pVBO [] =
    {//-     Point      -      Normal     -  TexCoord -
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,	1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,	0.0f, 1.0f
    };
    memcpy(pVertexQuad, pVBO, sizeof(pVBO));

    RT::BufferCPUSPtr IBOQuad(new RT::BufferCPU());
    unsigned int* pIndexQuad = (unsigned int*)IBOQuad->create(RT::Mesh::getStandardIBOFormat(), 2);
    unsigned int pIBO[] =
    {
        0, 1, 2,
        2, 3, 0
    };
    memcpy(pIndexQuad, pIBO, sizeof(pIBO));
    
    RT::Mesh::Descriptors descriptors;
    m_pMesh->initialize(VBOQuad, IBOQuad, descriptors);
}

void GUIWidget::Release()
{
    delete m_pMesh;
    m_pMesh=NULL;
}
