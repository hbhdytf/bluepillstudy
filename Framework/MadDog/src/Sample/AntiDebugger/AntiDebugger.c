/* 
 * Copyright holder: Invisible Things Lab
 * 
 * This software is protected by domestic and International
 * copyright laws. Any use (including publishing and
 * distribution) of this software requires a valid license
 * from the copyright holder.
 *
 * This software is provided for the educational use only
 * during the Black Hat training. This software should not
 * be used on production systems.
 *
 */

#include "AntiDebugger.h"
#include "Hook.h"
#include "Util.h"

//extern PHYSICAL_ADDRESS g_PageMapBasePhysicalAddress;
//extern BOOLEAN g_bDisableComOutput;

static NTSTATUS Initialization();

static MadDog_Control md_Control = 
{
	NULL,
	NULL,
	&HvmSetupVMControlBlock,
	&VmxRegisterTraps
};

VOID NTAPI Finalize()
{
	HvMmShutdownManager();
	DbgDisposeComponent();
}

NTSTATUS DriverUnload (
    PDRIVER_OBJECT DriverObject
)
{
    //FIXME: do not turn SVM/VMX when it has been turned on by the guest in the meantime (e.g. VPC, VMWare)
    NTSTATUS Status;

    Print(("\r\n"));
    Print(("NEWBLUEPILL: Unloading started\n"));

    if (!NT_SUCCESS (Status = MadDog_UninstallHypervisor())) 
    {
        Print(("NEWBLUEPILL: UninstallHypervisor() failed with status 0x%08hX\n",Status));
		Finalize();
        return Status;
    }

    Print(("NEWBLUEPILL: Unloading finished\n"));

	Finalize();
	//UnHook(KDEAddr);
    return STATUS_SUCCESS;
}

NTSTATUS DriverEntry (
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
)
{
    NTSTATUS Status;
    //CmDebugBreak();
   // ULONG ulOldCR3;

    DbgInitComponent();
	InitSpinLock();
    //__asm { int 3 }

	//Initialization();
	
	Status = HvMmInitManager();
    if (!NT_SUCCESS (Status)) 
    {
 			Print(("HELLOWORLD: MadDog_MmInitManager() failed with status 0x%08hX\n", Status));
 			Finalize();
			return Status;
    }

    if (!NT_SUCCESS (Status = MadDog_HypervisorInit())) 
    {
        Print(("HELLOWORLD: MadDog_HypervisorInit() failed with status 0x%08hX\n", Status));
		Finalize();
		return Status;
    }
	Print(("HELLOWORLD: Successful in execute HvmInit()"));


    if (!NT_SUCCESS (Status = MadDog_InstallHypervisor(&md_Control,DriverObject))) //<------------------1 Finish
    {
        Print(("HELLOWORLD: InstallHypervisor() failed with status 0x%08hX\n", Status));
		Finalize();
		return Status;
    }

     DriverObject->DriverUnload = DriverUnload;
	Print(("HELLOWORLD: Initialization finished\n"));
	#if DEBUG_LEVEL>1
		Print(("HELLOWORLD: EFLAGS = %#x\n", RegGetRflags ()));
	#endif

    return STATUS_SUCCESS;
}