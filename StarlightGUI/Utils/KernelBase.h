#pragma once

#include "pch.h"
#include "IOBase.h"
#include "AstralIO.h"
#include "unordered_set"

enum ZBID
{
	ZBID_DEFAULT = 0,
	ZBID_DESKTOP = 1,
	ZBID_UIACCESS = 2,
	ZBID_IMMERSIVE_IHM = 3,
	ZBID_IMMERSIVE_NOTIFICATION = 4,
	ZBID_IMMERSIVE_APPCHROME = 5,
	ZBID_IMMERSIVE_MOGO = 6,
	ZBID_IMMERSIVE_EDGY = 7,
	ZBID_IMMERSIVE_INACTIVEMOBODY = 8,
	ZBID_IMMERSIVE_INACTIVEDOCK = 9,
	ZBID_IMMERSIVE_ACTIVEMOBODY = 10,
	ZBID_IMMERSIVE_ACTIVEDOCK = 11,
	ZBID_IMMERSIVE_BACKGROUND = 12,
	ZBID_IMMERSIVE_SEARCH = 13,
	ZBID_GENUINE_WINDOWS = 14,
	ZBID_IMMERSIVE_RESTRICTED = 15,
	ZBID_SYSTEM_TOOLS = 16,
	ZBID_LOCK = 17,
	ZBID_ABOVELOCK_UX = 18,
};

namespace winrt::StarlightGUI::implementation {
	class KernelInstance {
	public:
		static bool IsRunningAsAdmin() noexcept;

		// Process
		static BOOL _ZwTerminateProcess(ULONG pid) noexcept;
		static BOOL MurderProcess(ULONG pid) noexcept;

		static BOOL _SuspendProcess(ULONG pid) noexcept;
		static BOOL _ResumeProcess(ULONG pid) noexcept;
		static BOOL HideProcess(ULONG pid) noexcept;
		static BOOL SetPPL(ULONG pid, int level) noexcept;
		static BOOL SetCriticalProcess(ULONG pid) noexcept;
		static BOOL InjectDLLToProcess(ULONG pid, PWCHAR dllPath) noexcept;
		static BOOL ModifyProcessToken(ULONG pid, ULONG type) noexcept;

		// Thread
		static BOOL _ZwTerminateThread(ULONG tid) noexcept;
		static BOOL MurderThread(ULONG tid) noexcept;

		static BOOL _SuspendThread(ULONG tid) noexcept;
		static BOOL _ResumeThread(ULONG tid) noexcept;

		// Driver
		static BOOL UnloadDriver(ULONG64 driverObj) noexcept;
		static BOOL HideDriver(ULONG64 driverObj) noexcept;

		// Enum
		static BOOL EnumProcess(std::unordered_map<DWORD, int> processMap, std::vector<winrt::StarlightGUI::ProcessInfo>& targetList) noexcept;
		static BOOL EnumProcess2(std::unordered_map<DWORD, int> processMap, std::vector<winrt::StarlightGUI::ProcessInfo>& targetList) noexcept;
		static BOOL EnumProcessThread(ULONG64 eprocess, std::vector<winrt::StarlightGUI::ThreadInfo>& threads) noexcept;
		static BOOL EnumProcessHandle(ULONG pid, std::vector<winrt::StarlightGUI::HandleInfo>& handles) noexcept;
		static BOOL EnumProcessModule(ULONG64 eprocess, std::vector<winrt::StarlightGUI::MokuaiInfo>& threads) noexcept;
		static BOOL EnumProcessKernelCallbackTable(ULONG64 eprocess, std::vector<winrt::StarlightGUI::KCTInfo>& threads) noexcept;
		static BOOL EnumDrivers(std::vector<winrt::StarlightGUI::KernelModuleInfo>& kernelModules) noexcept;

		// File
		static BOOL QueryFile(std::wstring path, std::vector<winrt::StarlightGUI::FileInfo>& files) noexcept;
		static BOOL DeleteFileAuto(std::wstring path) noexcept;
		static BOOL _DeleteFileAuto(std::wstring path) noexcept;
		static BOOL MurderFileAuto(std::wstring path) noexcept;
		static BOOL LockFile(std::wstring path) noexcept;
		static BOOL _CopyFile(std::wstring from, std::wstring to, std::wstring name) noexcept;

		// System
		static BOOL EnableCreateProcess() noexcept;
		static BOOL DisableCreateProcess() noexcept;
		static BOOL EnableCreateFile() noexcept;
		static BOOL DisableCreateFile() noexcept;
		static BOOL EnableLoadDriver() noexcept;
		static BOOL DisableLoadDriver() noexcept;
		static BOOL EnableUnloadDriver() noexcept;
		static BOOL DisableUnloadDriver() noexcept;
		static BOOL EnableModifyRegistry() noexcept;
		static BOOL DisableModifyRegistry() noexcept;
		static BOOL ProtectDisk() noexcept;
		static BOOL UnprotectDisk() noexcept;
		static BOOL EnableDSE() noexcept;
		static BOOL DisableDSE() noexcept;
		static BOOL EnableObCallback() noexcept;
		static BOOL DisableObCallback() noexcept;
		static BOOL EnableCmpCallback() noexcept;
		static BOOL DisableCmpCallback() noexcept;
		static BOOL EnableLKD() noexcept;
		static BOOL DisableLKD() noexcept;
		static BOOL DisablePatchGuard(int type) noexcept;
		static BOOL Shutdown();
		static BOOL Reboot();
		static BOOL RebootForce();
		static BOOL BlueScreen(int color);

	private:
		static BOOL GetDriverDevice() noexcept;
		static BOOL GetDriverDevice2() noexcept;
		static BOOL _DeleteFile(std::wstring path) noexcept;
		static BOOL MurderFile(std::wstring path) noexcept;
	};

	class KernelBase {
	public:
		static DWORD64 GetCIBaseAddress();
		static ULONG64 HackCI();
	};

	class DriverUtils {

	public:
		static bool LoadKernelDriver(LPCWSTR kernelPath, std::wstring& dbgMsg) noexcept;

		static bool LoadDriver(LPCWSTR kernelPath, LPCWSTR fileName, std::wstring& dbgMsg) noexcept;
	};
}