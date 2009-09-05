#ifndef DEBUG_H
#define DEBUG_H

class TCpuMgr
{
    static const int Count;
    static Z80* Cpus[];
    static Z80 PrevCpus[];
    static int CurrentCpu;
public:
    static Z80 &Cpu() { return *Cpus[CurrentCpu]; }
    static Z80 &PrevCpu() { return PrevCpus[CurrentCpu]; }
    static void SwitchCpu();
    static int GetCurrentCpu() { return CurrentCpu; }
    static void CopyToPrev();
};

extern TCpuMgr CpuMgr;
#endif
