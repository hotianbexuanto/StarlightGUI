#pragma once

#include "pch.h"

#define IOCTL_AX_ENUM_PROCESSES				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x700, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_AX_TERMINATE_PROCESS		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_AX_HIDE_PROCESS			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_AX_SET_PPL				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _PROCESS_INFO {
    ULONG Pid;
} PROCESS_INFO, * PPROCESS_INFO;

typedef struct _PROCESS_SET_PPL {
    ULONG Pid;
    UCHAR SignatureSigner;
} PROCESS_SET_PPL, * PPROCESS_SET_PPL;

typedef struct _PROCESS_DATA {
    ULONG64 Eprocess;
    ULONG Pid;
    ULONG ParentPid;
    ULONG64 VirtualSize;
    ULONG64 WorkingSetSize;
    ULONG64 WorkingSetPrivateSize;
    BOOLEAN IsWow64;
    WCHAR ImageName[256];
    WCHAR ImagePath[1024];
} PROCESS_DATA, * PPROCESS_DATA;

typedef struct _ENUM_PROCESS {
    PVOID Buffer;
    ULONG BufferSize;
    ULONG ProcessCount;
} ENUM_PROCESS, * PENUM_PROCESS;

typedef enum _PS_PROTECTED_SIGNER
{
    PsProtectedSignerNone = 0,
    PsProtectedSignerAuthenticode,
    PsProtectedSignerCodeGen,
    PsProtectedSignerAntimalware,
    PsProtectedSignerLsa,
    PsProtectedSignerWindows,
    PsProtectedSignerWinTcb,
    PsProtectedSignerWinSystem,
    PsProtectedSignerApp,
    PsProtectedSignerMax
} PS_PROTECTED_SIGNER, * PPS_PROTECTED_SIGNER;