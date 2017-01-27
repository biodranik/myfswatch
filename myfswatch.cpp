#include <iostream>
#include <string>
#include <Windows.h>

using std::wcerr;
using std::wcout;
using std::endl;
using std::wstring;

// Helper to clean up handle on exit.
template <class TCloseFunction> class HandleCloser {
  HANDLE m_handle;
  TCloseFunction m_close;
public:
  HandleCloser(HANDLE handle, TCloseFunction closer)
  : m_handle(handle), m_close(closer) {}
  ~HandleCloser() { m_close(m_handle); }
  operator HANDLE() const { return m_handle; }
};

template <class TCloseFunction>
HandleCloser<TCloseFunction> AutoCloseHandle(HANDLE handle, TCloseFunction closer) {
  return HandleCloser<TCloseFunction>(handle, closer);
}

// The simplest solution to store program options.
bool g_exitOnFirstChange = false;
bool g_useNulAsDelimiter = false;

void Usage(const wchar_t * self) {
  const wchar_t * text = LR"(Watches for any changes in directory and subdirectories.
This is a very simple replacement for original fswatch utility
(see https://github.com/emcrisostomo/fswatch for more details).
Author: Alexander Zolotarev <me@alex.bio> from Minsk, Belarus.

Usage: %s [-1|--one-event] <dir to watch for changes>
  -0, --print0      Use 'NUL' ('\0') to split output instead of default newline ('\n').
  -1, --one-event   Stop watching and exit after any detected change instead of watching indefinitely.
)";
  wprintf(text, self);
}

void OnFileSystemChanged(const wstring & dir) {
  wcout << L"Content of (" << dir << L") directory has changed.";
  if (g_useNulAsDelimiter) {
    wcout.put(0);
  }
  else {
    wcout << endl;
  }
}

void WatchDirectory(const wstring & dir) {
  // Convert relative paths to fully qualified.
  wchar_t fullPath[MAX_PATH];
  if (0 == GetFullPathNameW(dir.c_str(), MAX_PATH, fullPath, NULL)) {
    wcerr << L"ERROR: GetFullPathNameW has failed. Is directory " << dir << L" valid?" << endl;
    ExitProcess(GetLastError());
  }

  // Watch the directory and it's subdirectories for any modifications. 
  const auto dwChangeHandle = AutoCloseHandle(FindFirstChangeNotificationW(
      fullPath,                       // directory to watch
      TRUE,                           // watch subtree 
      FILE_NOTIFY_CHANGE_FILE_NAME |
      FILE_NOTIFY_CHANGE_DIR_NAME |
      FILE_NOTIFY_CHANGE_SIZE |
      FILE_NOTIFY_CHANGE_LAST_WRITE |
      FILE_NOTIFY_CHANGE_CREATION), &FindCloseChangeNotification);
  if (dwChangeHandle == INVALID_HANDLE_VALUE) {
    wprintf(L"ERROR: FindFirstChangeNotification function failed.\n");
    ExitProcess(GetLastError());
  }

  // Change notification is set. Now wait on notification handle and act accordingly. 
  while (TRUE) {
    switch (WaitForSingleObject(dwChangeHandle, INFINITE)) {
    case WAIT_OBJECT_0:
      // Modification is detected.
      OnFileSystemChanged(dir);
      if (FindNextChangeNotification(dwChangeHandle) == FALSE) {
        wprintf(L"ERROR: FindNextChangeNotification function failed.\n");
        ExitProcess(GetLastError());
      }
      break;

    case WAIT_TIMEOUT:
      // A timeout occurred, this would happen if some value other 
      // than INFINITE is used in the Wait call and no changes occur.
      wprintf(L"No changes in the timeout period.\n");
      break;

    default:
      wprintf(L"ERROR: Unhandled dwWaitStatus.\n");
      ExitProcess(GetLastError());
      break;
    }
    // Exit infinite loop after the first change.
    if (g_exitOnFirstChange) {
      break;
    }
    // Helps to display output on some buffered terminals like MSYS.
    wcout.flush();
  }
}

int wmain(int argc, wchar_t * argv[]) {
  if (argc < 2) {
    Usage(argv[0]);
    return 1;
  }

  wstring dir;
  for (int i = 1; i < argc; ++i) {
    const wstring param(argv[i]);
    if (param == L"-1" || param == L"--one-event") {
      g_exitOnFirstChange = true;
    } else if (param == L"-0" || param == L"--print0") {
      g_useNulAsDelimiter = true;
    } else if (param[0] == L'-') {
      // Ignore all other params.
    } else {
      // Any non-param becomes a directory to watch.
      dir = param;
    }
  }
  if (dir.empty()) {
    Usage(argv[0]);
    return 1;
  }
  WatchDirectory(dir);
  // Correct exit in case of --one-event.
  return 0;
}
