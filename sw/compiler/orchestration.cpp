#include "orchestration.h"

Orchestrator::Orchestrator(const OrchestratorParam& param)
: m_param(param),
  m_matProg(param.matCoreCount),
  m_vecProg(param.vecCoreCount)
{}
