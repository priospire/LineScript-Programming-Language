#define NOMINMAX
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr int kWinW = 760;
constexpr int kWinH = 610;
constexpr int kIdPath = 1001;
constexpr int kIdBrowse = 1002;
constexpr int kIdInstall = 1003;
constexpr int kIdCancel = 1004;
constexpr int kIdMath = 1101;
constexpr int kIdText = 1102;
constexpr int kIdCollections = 1103;
constexpr int kIdGame = 1104;
constexpr int kIdHttp = 1105;
constexpr int kIdWindow = 1106;
constexpr int kIdUi = 1107;
constexpr int kIdGraphics2d = 1108;
constexpr int kIdEngine3d = 1109;
constexpr int kIdPhysics = 1110;
constexpr int kIdVscode = 1120;
constexpr int kIdTests = 1121;
constexpr int kIdSelectAllLibs = 1122;
constexpr int kIdCoreLibs = 1123;
constexpr int kIdStatus = 1201;

struct Options {
  bool math = true;
  bool text = true;
  bool collections = true;
  bool game = true;
  bool http = true;
  bool window = true;
  bool ui = true;
  bool graphics2d = true;
  bool engine3d = true;
  bool physics = true;
  bool vscode = true;
  bool tests = false;
};

struct Controls {
  HWND path = nullptr;
  HWND status = nullptr;
  HFONT font = nullptr;
  HFONT titleFont = nullptr;
};

HINSTANCE gInst = nullptr;
Controls gControls;
fs::path gSourceRoot;

std::wstring toLower(std::wstring s) {
  for (wchar_t &c : s) c = static_cast<wchar_t>(towlower(c));
  return s;
}

std::string narrow(const std::wstring &s) {
  if (s.empty()) return {};
  int size = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (size <= 0) return {};
  std::string out(static_cast<std::size_t>(size - 1), '\0');
  WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, out.data(), size, nullptr, nullptr);
  return out;
}

fs::path exeDir() {
  wchar_t buf[MAX_PATH];
  DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
  if (n == 0 || n >= MAX_PATH) throw std::runtime_error("failed to locate installer");
  return fs::path(buf).parent_path();
}

fs::path defaultInstallDir() {
  wchar_t buf[MAX_PATH];
  DWORD n = GetEnvironmentVariableW(L"LOCALAPPDATA", buf, MAX_PATH);
  if (n > 0 && n < MAX_PATH) return fs::path(buf) / L"LineScript";
  return fs::path(L"C:\\LineScript");
}

fs::path detectSourceRoot() {
  fs::path dir = exeDir();
  if (fs::exists(dir / L"payload" / L"lsc.exe")) return dir / L"payload";
  if (fs::exists(dir / L"lsc.exe")) return dir;
  if (fs::exists(dir.parent_path() / L"lsc.exe")) return dir.parent_path();
  return dir;
}

bool hasCliIntent() {
  std::wstring cmd = GetCommandLineW();
  return cmd.find(L"--") != std::wstring::npos;
}

void requirePayload(const fs::path &sourceRoot) {
  if (!fs::exists(sourceRoot / L"lsc.exe")) {
    throw std::runtime_error("installer payload is missing lsc.exe");
  }
  if (!fs::exists(sourceRoot / L"linescript.cmd")) {
    throw std::runtime_error("installer payload is missing linescript.cmd");
  }
}

std::wstring getText(HWND h) {
  int len = GetWindowTextLengthW(h);
  std::wstring out(static_cast<std::size_t>(len), L'\0');
  GetWindowTextW(h, out.data(), len + 1);
  return out;
}

bool checked(HWND parent, int id) {
  return SendMessageW(GetDlgItem(parent, id), BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void setChecked(HWND parent, int id, bool value) {
  SendMessageW(GetDlgItem(parent, id), BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0);
}

void setFont(HWND h) {
  if (gControls.font) SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(gControls.font), TRUE);
}

void setTitleFont(HWND h) {
  if (gControls.titleFont) SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(gControls.titleFont), TRUE);
}

HWND addControl(HWND parent, const wchar_t *klass, const wchar_t *text, DWORD style, int x, int y, int w, int h,
                int id) {
  HWND ctrl = CreateWindowExW(0, klass, text, style | WS_CHILD | WS_VISIBLE, x, y, w, h, parent,
                              reinterpret_cast<HMENU>(static_cast<intptr_t>(id)), gInst, nullptr);
  setFont(ctrl);
  return ctrl;
}

void copyFileIfExists(const fs::path &sourceRoot, const fs::path &destRoot, const wchar_t *name) {
  fs::path src = sourceRoot / name;
  if (!fs::exists(src)) return;
  fs::create_directories(destRoot);
  fs::copy_file(src, destRoot / name, fs::copy_options::overwrite_existing);
}

void copyDirIfExists(const fs::path &sourceRoot, const fs::path &destRoot, const wchar_t *name) {
  fs::path src = sourceRoot / name;
  if (!fs::exists(src)) return;
  fs::path dst = destRoot / name;
  fs::create_directories(dst);
  fs::copy(src, dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
}

std::wstring findTool(const wchar_t *name) {
  wchar_t buf[MAX_PATH];
  DWORD n = SearchPathW(nullptr, name, nullptr, MAX_PATH, buf, nullptr);
  if (n == 0 || n >= MAX_PATH) return L"missing";
  return buf;
}

void writeDependencyStatus(const fs::path &destRoot) {
  fs::path depDir = destRoot / L"dependencies";
  fs::create_directories(depDir);
  std::ofstream out(depDir / L"DEPENDENCY_STATUS.txt", std::ios::binary);
  out << "LineScript dependency status\n";
  out << "============================\n\n";
  out << "Installed bundled tools:\n";
  out << "- lsc.exe\n";
  out << "- linescript.cmd / linescript.ps1 / linescript.sh when present\n\n";
  out << "External backend compiler checks:\n";
  out << "- clang.exe: " << narrow(findTool(L"clang.exe")) << "\n";
  out << "- clang++.exe: " << narrow(findTool(L"clang++.exe")) << "\n";
  out << "- g++.exe: " << narrow(findTool(L"g++.exe")) << "\n\n";
  out << "LineScript can check code with lsc.exe alone. Building native binaries requires clang or g++.\n";
}

void writeLaunchers(const fs::path &destRoot) {
  std::ofstream cmd(destRoot / L"LineScript Shell.cmd", std::ios::binary);
  cmd << "@echo off\r\n";
  cmd << "cd /d \"%~dp0\"\r\n";
  cmd << "lsc.exe --shell\r\n";

  std::ofstream readme(destRoot / L"INSTALL_RECEIPT.txt", std::ios::binary);
  readme << "LineScript was installed here.\n\n";
  readme << "Run shell:\n";
  readme << "  LineScript Shell.cmd\n\n";
  readme << "Run a file:\n";
  readme << "  linescript.cmd main.lsc -O4 --cc clang\n\n";
  readme << "Preset libraries live in the libraries folder when selected.\n";
}

void installLineScript(const fs::path &sourceRoot, const fs::path &destRoot, const Options &opt) {
  requirePayload(sourceRoot);
  fs::create_directories(destRoot);

  for (const wchar_t *name : {L"lsc.exe", L"lsc.cpp", L"README.md", L"linescript.cmd", L"linescript.ps1",
                              L"linescript.sh"}) {
    copyFileIfExists(sourceRoot, destRoot, name);
  }

  for (const wchar_t *name : {L"docs", L"examples", L"scripts", L"editor-configs", L".vscode"}) {
    copyDirIfExists(sourceRoot, destRoot, name);
  }

  if (opt.tests) copyDirIfExists(sourceRoot, destRoot, L"tests");
  if (opt.vscode) copyDirIfExists(sourceRoot, destRoot, L"vscode-extension");

  fs::path srcLib = sourceRoot / L"libraries";
  fs::path dstLib = destRoot / L"libraries";
  if (fs::exists(srcLib)) {
    fs::create_directories(dstLib);
    copyFileIfExists(srcLib, dstLib, L"README.md");
    if (opt.math) copyDirIfExists(srcLib, dstLib, L"math");
    if (opt.text) copyDirIfExists(srcLib, dstLib, L"text");
    if (opt.collections) copyDirIfExists(srcLib, dstLib, L"collections");
    if (opt.game) copyDirIfExists(srcLib, dstLib, L"game");
    if (opt.http) copyDirIfExists(srcLib, dstLib, L"http");
    if (opt.window) copyDirIfExists(srcLib, dstLib, L"window");
    if (opt.ui) copyDirIfExists(srcLib, dstLib, L"ui");
    if (opt.graphics2d) copyDirIfExists(srcLib, dstLib, L"graphics2d");
    if (opt.engine3d) copyDirIfExists(srcLib, dstLib, L"engine3d");
    if (opt.physics) copyDirIfExists(srcLib, dstLib, L"physics");
  }

  writeDependencyStatus(destRoot);
  writeLaunchers(destRoot);
}

void browseForFolder(HWND hwnd) {
  BROWSEINFOW bi{};
  bi.hwndOwner = hwnd;
  bi.lpszTitle = L"Choose LineScript install folder";
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
  PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);
  if (!pidl) return;
  wchar_t path[MAX_PATH];
  if (SHGetPathFromIDListW(pidl, path)) SetWindowTextW(gControls.path, path);
  CoTaskMemFree(pidl);
}

void doGuiInstall(HWND hwnd) {
  try {
    Options opt;
    opt.math = checked(hwnd, kIdMath);
    opt.text = checked(hwnd, kIdText);
    opt.collections = checked(hwnd, kIdCollections);
    opt.game = checked(hwnd, kIdGame);
    opt.http = checked(hwnd, kIdHttp);
    opt.window = checked(hwnd, kIdWindow);
    opt.ui = checked(hwnd, kIdUi);
    opt.graphics2d = checked(hwnd, kIdGraphics2d);
    opt.engine3d = checked(hwnd, kIdEngine3d);
    opt.physics = checked(hwnd, kIdPhysics);
    opt.vscode = checked(hwnd, kIdVscode);
    opt.tests = checked(hwnd, kIdTests);

    fs::path dest = getText(gControls.path);
    if (dest.empty()) throw std::runtime_error("install folder is empty");

    SetWindowTextW(gControls.status, L"Installing...");
    EnableWindow(GetDlgItem(hwnd, kIdInstall), FALSE);
    installLineScript(gSourceRoot, dest, opt);
    SetWindowTextW(gControls.status, L"Install complete.");
    MessageBoxW(hwnd, L"LineScript installation completed.", L"LineScript Setup", MB_OK | MB_ICONINFORMATION);
    EnableWindow(GetDlgItem(hwnd, kIdInstall), TRUE);
  } catch (const std::exception &e) {
    EnableWindow(GetDlgItem(hwnd, kIdInstall), TRUE);
    std::wstring msg = L"Install failed:\n";
    std::string raw = e.what();
    msg.append(raw.begin(), raw.end());
    SetWindowTextW(gControls.status, L"Install failed.");
    MessageBoxW(hwnd, msg.c_str(), L"LineScript Setup", MB_OK | MB_ICONERROR);
  }
}

void setLibraryChecks(HWND hwnd, bool allGamePacks) {
  setChecked(hwnd, kIdMath, true);
  setChecked(hwnd, kIdText, true);
  setChecked(hwnd, kIdCollections, true);
  setChecked(hwnd, kIdHttp, true);
  setChecked(hwnd, kIdGame, allGamePacks);
  setChecked(hwnd, kIdWindow, allGamePacks);
  setChecked(hwnd, kIdUi, allGamePacks);
  setChecked(hwnd, kIdGraphics2d, allGamePacks);
  setChecked(hwnd, kIdEngine3d, allGamePacks);
  setChecked(hwnd, kIdPhysics, allGamePacks);
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
    case WM_CREATE: {
      gControls.font = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
      gControls.titleFont =
          CreateFontW(24, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                      CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

      HWND title = addControl(hwnd, L"STATIC", L"LineScript Setup", WS_CHILD | WS_VISIBLE, 20, 14, 500, 30, 0);
      setTitleFont(title);
      addControl(hwnd, L"STATIC", L"Install the compiler, editor files, and optional preset libraries.",
                 WS_CHILD | WS_VISIBLE, 22, 48, 650, 22, 0);
      addControl(hwnd, L"STATIC", L"Install folder:", WS_CHILD | WS_VISIBLE, 20, 85, 120, 22, 0);
      gControls.path =
          addControl(hwnd, L"EDIT", defaultInstallDir().c_str(), WS_BORDER | ES_AUTOHSCROLL, 130, 82, 490, 24, kIdPath);
      addControl(hwnd, L"BUTTON", L"Browse...", BS_PUSHBUTTON, 635, 81, 90, 26, kIdBrowse);

      addControl(hwnd, L"BUTTON", L"Core libraries", BS_GROUPBOX, 20, 125, 330, 145, 0);
      addControl(hwnd, L"BUTTON", L"Math helpers", BS_AUTOCHECKBOX, 38, 154, 250, 22, kIdMath);
      addControl(hwnd, L"BUTTON", L"Text/string helpers", BS_AUTOCHECKBOX, 38, 180, 250, 22, kIdText);
      addControl(hwnd, L"BUTTON", L"Collections helpers", BS_AUTOCHECKBOX, 38, 206, 250, 22, kIdCollections);
      addControl(hwnd, L"BUTTON", L"HTTP helpers", BS_AUTOCHECKBOX, 38, 232, 250, 22, kIdHttp);

      addControl(hwnd, L"BUTTON", L"Game engine libraries", BS_GROUPBOX, 380, 125, 345, 200, 0);
      addControl(hwnd, L"BUTTON", L"Game/window base helpers", BS_AUTOCHECKBOX, 398, 154, 275, 22, kIdGame);
      addControl(hwnd, L"BUTTON", L"Window lifecycle helpers", BS_AUTOCHECKBOX, 398, 180, 275, 22, kIdWindow);
      addControl(hwnd, L"BUTTON", L"UI widgets and hit-testing", BS_AUTOCHECKBOX, 398, 206, 275, 22, kIdUi);
      addControl(hwnd, L"BUTTON", L"2D drawing helpers", BS_AUTOCHECKBOX, 398, 232, 275, 22, kIdGraphics2d);
      addControl(hwnd, L"BUTTON", L"3D projection helpers", BS_AUTOCHECKBOX, 398, 258, 275, 22, kIdEngine3d);
      addControl(hwnd, L"BUTTON", L"Physics/camera helpers", BS_AUTOCHECKBOX, 398, 284, 275, 22, kIdPhysics);

      addControl(hwnd, L"BUTTON", L"Tooling", BS_GROUPBOX, 20, 285, 330, 95, 0);
      addControl(hwnd, L"BUTTON", L"VSCode extension files", BS_AUTOCHECKBOX, 38, 314, 250, 22, kIdVscode);
      addControl(hwnd, L"BUTTON", L"Test suites", BS_AUTOCHECKBOX, 38, 340, 250, 22, kIdTests);

      addControl(hwnd, L"BUTTON", L"Select all libraries", BS_PUSHBUTTON, 380, 345, 150, 28, kIdSelectAllLibs);
      addControl(hwnd, L"BUTTON", L"Core libraries only", BS_PUSHBUTTON, 545, 345, 150, 28, kIdCoreLibs);

      addControl(hwnd, L"STATIC",
                 L"Native builds require clang or g++. The installer records tool detection in dependencies\\DEPENDENCY_STATUS.txt.",
                 WS_CHILD | WS_VISIBLE, 20, 405, 690, 36, 0);
      addControl(hwnd, L"STATIC",
                 L"Game/window libraries are safe to install on every machine; visible windows depend on platform support.",
                 WS_CHILD | WS_VISIBLE, 20, 438, 690, 36, 0);
      gControls.status = addControl(hwnd, L"STATIC", L"Ready.", WS_CHILD | WS_VISIBLE, 20, 490, 560, 22, kIdStatus);
      addControl(hwnd, L"BUTTON", L"Install", BS_DEFPUSHBUTTON, 515, 525, 95, 32, kIdInstall);
      addControl(hwnd, L"BUTTON", L"Cancel", BS_PUSHBUTTON, 625, 525, 95, 32, kIdCancel);

      setLibraryChecks(hwnd, true);
      setChecked(hwnd, kIdVscode, true);
      setChecked(hwnd, kIdTests, false);
      return 0;
    }
    case WM_COMMAND:
      switch (LOWORD(wp)) {
        case kIdBrowse:
          browseForFolder(hwnd);
          return 0;
        case kIdInstall:
          doGuiInstall(hwnd);
          return 0;
        case kIdCancel:
          DestroyWindow(hwnd);
          return 0;
        case kIdSelectAllLibs:
          setLibraryChecks(hwnd, true);
          return 0;
        case kIdCoreLibs:
          setLibraryChecks(hwnd, false);
          return 0;
      }
      break;
    case WM_DESTROY:
      if (gControls.titleFont) {
        DeleteObject(gControls.titleFont);
        gControls.titleFont = nullptr;
      }
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProcW(hwnd, msg, wp, lp);
}

int runCli(const std::vector<std::wstring> &args) {
  if (args.size() <= 1) return -1;
  Options opt;
  fs::path dest;
  bool requested = false;
  for (std::size_t i = 1; i < args.size(); ++i) {
    std::wstring a = toLower(args[i]);
    if (a == L"--probe") {
      requirePayload(gSourceRoot);
      return 0;
    }
    if (a == L"--install") {
      if (i + 1 >= args.size()) throw std::runtime_error("--install requires a folder");
      dest = args[++i];
      requested = true;
    } else if (a == L"--all-libraries") {
      opt.math = opt.text = opt.collections = opt.game = opt.http = true;
      opt.window = opt.ui = opt.graphics2d = opt.engine3d = opt.physics = true;
    } else if (a == L"--no-libraries") {
      opt.math = opt.text = opt.collections = opt.game = opt.http = false;
      opt.window = opt.ui = opt.graphics2d = opt.engine3d = opt.physics = false;
    } else if (a == L"--with-math") {
      opt.math = true;
    } else if (a == L"--with-text") {
      opt.text = true;
    } else if (a == L"--with-collections") {
      opt.collections = true;
    } else if (a == L"--with-game") {
      opt.game = true;
    } else if (a == L"--with-http") {
      opt.http = true;
    } else if (a == L"--with-window") {
      opt.window = true;
    } else if (a == L"--with-ui") {
      opt.ui = true;
    } else if (a == L"--with-graphics2d") {
      opt.graphics2d = true;
    } else if (a == L"--with-engine3d") {
      opt.engine3d = true;
    } else if (a == L"--with-physics") {
      opt.physics = true;
    } else if (a == L"--with-vscode") {
      opt.vscode = true;
    } else if (a == L"--without-vscode") {
      opt.vscode = false;
    } else if (a == L"--with-tests") {
      opt.tests = true;
    } else if (a == L"--quiet") {
      continue;
    } else {
      throw std::runtime_error("unknown installer argument");
    }
  }
  if (!requested) return -1;
  installLineScript(gSourceRoot, dest, opt);
  return 0;
}

}  // namespace

std::wstring widenArg(const char *s) {
  if (!s) return {};
  int size = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
  if (size <= 0) {
    std::string raw(s);
    return std::wstring(raw.begin(), raw.end());
  }
  std::wstring out(static_cast<std::size_t>(size - 1), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, s, -1, out.data(), size);
  return out;
}

int main(int argc, char **argv) {
  HINSTANCE hInst = GetModuleHandleW(nullptr);
  gInst = hInst;
  std::wstring rawCommand = GetCommandLineW();
  if (rawCommand.find(L"--probe") != std::wstring::npos) {
    try {
      gSourceRoot = detectSourceRoot();
      requirePayload(gSourceRoot);
      return 0;
    } catch (...) {
      return 1;
    }
  }
  std::vector<std::wstring> args;
  for (int i = 0; i < argc; ++i) args.push_back(widenArg(argv[i]));
  try {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    gSourceRoot = detectSourceRoot();
    int cli = runCli(args);
    if (cli >= 0) {
      CoUninitialize();
      return cli;
    }
    if (args.size() > 1) throw std::runtime_error("invalid installer command line");

    FreeConsole();
    requirePayload(gSourceRoot);
    const wchar_t *cls = L"LineScriptSetupWindow";
    WNDCLASSW wc{};
    wc.lpfnWndProc = wndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = cls;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(0, cls, L"LineScript Setup", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                CW_USEDEFAULT, CW_USEDEFAULT, kWinW, kWinH, nullptr, nullptr, hInst, nullptr);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    CoUninitialize();
    return 0;
  } catch (const std::exception &e) {
    if (args.size() > 1 || hasCliIntent()) return 1;
    std::wstring msg = L"LineScript Setup failed:\n";
    std::string raw = e.what();
    msg.append(raw.begin(), raw.end());
    MessageBoxW(nullptr, msg.c_str(), L"LineScript Setup", MB_OK | MB_ICONERROR);
    return 1;
  }
}
