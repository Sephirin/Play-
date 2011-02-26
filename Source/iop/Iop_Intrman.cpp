#include "Iop_Intrman.h"
#include "../Log.h"
#include "../COP_SCU.h"
#include "Iop_Intc.h"
#include "IopBios.h"

#define LOGNAME "iop_intrman"

using namespace Iop;
using namespace std;

#define FUNCTION_REGISTERINTRHANDLER    "RegisterIntrHandler"
#define FUNCTION_RELEASEINTRHANDLER		"ReleaseIntrHandler"
#define FUNCTION_ENABLEINTRLINE         "EnableIntrLine"
#define FUNCTION_DISABLEINTRLINE		"DisableIntrLine"
#define FUNCTION_DISABLEINTERRUPTS		"DisableInterrupts"
#define FUNCTION_ENABLEINTERRUPTS		"EnableInterrupts"
#define FUNCTION_SUSPENDINTERRUPTS      "SuspendInterrupts"
#define FUNCTION_RESUMEINTERRUPTS       "ResumeInterrupts"
#define FUNCTION_QUERYINTRCONTEXT		"QueryIntrContext"

CIntrman::CIntrman(CIopBios& bios, uint8* ram) :
m_bios(bios),
m_ram(ram)
{

}

CIntrman::~CIntrman()
{

}

string CIntrman::GetId() const
{
    return "intrman";
}

string CIntrman::GetFunctionName(unsigned int functionId) const
{
    switch(functionId)
    {
    case 4:
        return FUNCTION_REGISTERINTRHANDLER;
        break;
	case 5:
		return FUNCTION_RELEASEINTRHANDLER;
		break;
    case 6:
        return FUNCTION_ENABLEINTRLINE;
        break;
	case 7:
		return FUNCTION_DISABLEINTRLINE;
		break;
	case 8:
		return FUNCTION_DISABLEINTERRUPTS;
		break;
	case 9:
		return FUNCTION_ENABLEINTERRUPTS;
		break;
    case 17:
        return FUNCTION_SUSPENDINTERRUPTS;
        break;
    case 18:
        return FUNCTION_RESUMEINTERRUPTS;
        break;
	case 23:
		return FUNCTION_QUERYINTRCONTEXT;
		break;
    default:
        return "unknown";
        break;
    }
}

void CIntrman::Invoke(CMIPS& context, unsigned int functionId)
{
    switch(functionId)
    {
    case 4:
        context.m_State.nGPR[CMIPS::V0].nD0 = static_cast<int32>(RegisterIntrHandler(
            context.m_State.nGPR[CMIPS::A0].nV0,
            context.m_State.nGPR[CMIPS::A1].nV0,
            context.m_State.nGPR[CMIPS::A2].nV0,
            context.m_State.nGPR[CMIPS::A3].nV0
            ));
        break;
    case 5:
        context.m_State.nGPR[CMIPS::V0].nD0 = static_cast<int32>(ReleaseIntrHandler(
            context.m_State.nGPR[CMIPS::A0].nV0
            ));
        break;
    case 6:
        context.m_State.nGPR[CMIPS::V0].nD0 = static_cast<int32>(EnableIntrLine(
			context,
            context.m_State.nGPR[CMIPS::A0].nV0
            ));
        break;
    case 7:
        context.m_State.nGPR[CMIPS::V0].nD0 = static_cast<int32>(DisableIntrLine(
			context,
            context.m_State.nGPR[CMIPS::A0].nV0,
            context.m_State.nGPR[CMIPS::A1].nV0
            ));
        break;
	case 8:
        context.m_State.nGPR[CMIPS::V0].nD0 = static_cast<int32>(DisableInterrupts(
            context
            ));
		break;
    case 9:
        context.m_State.nGPR[CMIPS::V0].nD0 = static_cast<int32>(EnableInterrupts(
            context
            ));
        break;
    case 17:
        context.m_State.nGPR[CMIPS::V0].nD0 = static_cast<int32>(SuspendInterrupts(
            context,
            reinterpret_cast<uint32*>(&m_ram[context.m_State.nGPR[CMIPS::A0].nV0])
            ));
        break;
    case 18:
        context.m_State.nGPR[CMIPS::V0].nD0 = static_cast<int32>(ResumeInterrupts(
            context,
            context.m_State.nGPR[CMIPS::A0].nV0
            ));
        break;
    case 23:
        context.m_State.nGPR[CMIPS::V0].nD0 = static_cast<int32>(QueryIntrContext(
            context
            ));
        break;
    default:
        CLog::GetInstance().Print(LOGNAME, "%0.8X: Unknown function (%d) called.\r\n", context.m_State.nPC, functionId);
        break;
    }
}

uint32 CIntrman::RegisterIntrHandler(uint32 line, uint32 mode, uint32 handler, uint32 arg)
{
#ifdef _DEBUG
    CLog::GetInstance().Print(LOGNAME, FUNCTION_REGISTERINTRHANDLER "(line = %d, mode = %d, handler = 0x%0.8X, arg = 0x%0.8X);\r\n",
        line, mode, handler, arg);
#endif
    return m_bios.RegisterIntrHandler(line, mode, handler, arg) ? 1 : 0;
}

uint32 CIntrman::ReleaseIntrHandler(uint32 line)
{
#ifdef _DEBUG
    CLog::GetInstance().Print(LOGNAME, FUNCTION_RELEASEINTRHANDLER "(line = %d);\r\n",
        line);
#endif
    return m_bios.ReleaseIntrHandler(line) ? 1 : 0;
}

uint32 CIntrman::EnableIntrLine(CMIPS& context, uint32 line)
{
#ifdef _DEBUG
    CLog::GetInstance().Print(LOGNAME, FUNCTION_ENABLEINTRLINE "(line = %d);\r\n",
        line);
#endif
	UNION64_32 mask(
		context.m_pMemoryMap->GetWord(CIntc::MASK0),
		context.m_pMemoryMap->GetWord(CIntc::MASK1));
	mask.f |= 1LL << line;
	context.m_pMemoryMap->SetWord(CIntc::MASK0, mask.h0);
	context.m_pMemoryMap->SetWord(CIntc::MASK1, mask.h1);
	return 0;
}

uint32 CIntrman::DisableIntrLine(CMIPS& context, uint32 line, uint32 res)
{
#ifdef _DEBUG
    CLog::GetInstance().Print(LOGNAME, FUNCTION_DISABLEINTRLINE "(line = %d, res = %0.8X);\r\n",
        line, res);
#endif
	UNION64_32 mask(
		context.m_pMemoryMap->GetWord(CIntc::MASK0),
		context.m_pMemoryMap->GetWord(CIntc::MASK1));
	mask.f &= ~(1LL << line);
	context.m_pMemoryMap->SetWord(CIntc::MASK0, mask.h0);
	context.m_pMemoryMap->SetWord(CIntc::MASK1, mask.h1);
	return 0;
}

uint32 CIntrman::EnableInterrupts(CMIPS& context)
{
#ifdef _DEBUG
    CLog::GetInstance().Print(LOGNAME, FUNCTION_ENABLEINTERRUPTS "();\r\n");
#endif

    uint32& statusRegister = context.m_State.nCOP0[CCOP_SCU::STATUS];
    statusRegister |= CMIPS::STATUS_INT;
    return 0;
}

uint32 CIntrman::DisableInterrupts(CMIPS& context)
{
#ifdef _DEBUG
    CLog::GetInstance().Print(LOGNAME, FUNCTION_DISABLEINTERRUPTS "();\r\n");
#endif

    uint32& statusRegister = context.m_State.nCOP0[CCOP_SCU::STATUS];
    statusRegister &= ~CMIPS::STATUS_INT;
    return 0;
}

uint32 CIntrman::SuspendInterrupts(CMIPS& context, uint32* state)
{
#ifdef _DEBUG
    CLog::GetInstance().Print(LOGNAME, FUNCTION_SUSPENDINTERRUPTS "();\r\n");
#endif
    uint32& statusRegister = context.m_State.nCOP0[CCOP_SCU::STATUS];
    (*state) = statusRegister & CMIPS::STATUS_INT;
    statusRegister &= ~CMIPS::STATUS_INT;
    return 0;
}

uint32 CIntrman::ResumeInterrupts(CMIPS& context, uint32 state)
{
#ifdef _DEBUG
    CLog::GetInstance().Print(LOGNAME, FUNCTION_RESUMEINTERRUPTS "();\r\n");
#endif
    uint32& statusRegister = context.m_State.nCOP0[CCOP_SCU::STATUS];
    if(state)
    {
        statusRegister |= CMIPS::STATUS_INT;
    }
    else
    {
        statusRegister &= ~CMIPS::STATUS_INT;
    }
    return 0;
}

uint32 CIntrman::QueryIntrContext(CMIPS& context)
{
#ifdef _DEBUG
    CLog::GetInstance().Print(LOGNAME, FUNCTION_QUERYINTRCONTEXT "();\r\n");
#endif
    uint32& statusRegister = context.m_State.nCOP0[CCOP_SCU::STATUS];
    return (statusRegister & CMIPS::STATUS_EXL ? 1 : 0);
}
