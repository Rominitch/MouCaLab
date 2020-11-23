/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibRT/include/RTDataEnvironment.h"

namespace RT
{

DataEnvironment::DataEnvironment(void):
m_pContextExecution(NULL)
{}

DataEnvironment::~DataEnvironment(void)
{
	Release();
}

void DataEnvironment::Initialize(const ContextExecution* pContextExecution, const size_t szNbBuffers, const size_t szNbPrograms, const size_t szNbProgramPipelines, const size_t szNbGeometryFactory)
{
	Release();

	BT_ASSERT(pContextExecution!=NULL);

	m_pContextExecution = pContextExecution;

	//Resize of vector
	m_vBuffer.resize(szNbBuffers);
	m_vProgram.resize(szNbPrograms);
	m_vProgramPipeline.resize(szNbProgramPipelines);
	m_vGeometryFactory.resize(szNbGeometryFactory);
}

void DataEnvironment::Release()
{
	if(m_pContextExecution!=NULL)
	{
		//Clear each vector
		m_vBuffer.clear();

		m_vProgram.clear();

		m_vGeometryFactory.clear();
	}
}

}