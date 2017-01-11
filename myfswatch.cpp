#include <string>
#include <Windows.h>

void Usage(LPTSTR self) {
  wprintf(L"Watches for any changes in directory and subdirectories.\nUsage: %s [-1|--one-event] <dir>\n", self);
}

void OnFileSystemChanged(const std::wstring & dir) {
  wprintf(L"Content of (%s) directory has changed.\n", dir.c_str());
}

void WatchDirectory(const std::wstring & dir, bool exitOnFirstChange) {
  wchar_t drive[4];
  wchar_t file[_MAX_FNAME];
  wchar_t ext[_MAX_EXT];
  _wsplitpath_s(dir.c_str(), drive, 4, NULL, 0, file, _MAX_FNAME, ext, _MAX_EXT);
  drive[2] = L'\\';
  drive[3] = L'\0';

  // Watch the directory and it's subdirectories for any modifications. 
  HANDLE dwChangeHandle = FindFirstChangeNotificationW(
      dir.c_str(),                    // directory to watch 
      TRUE,                           // watch subtree 
      FILE_NOTIFY_CHANGE_FILE_NAME |
      FILE_NOTIFY_CHANGE_DIR_NAME |
      FILE_NOTIFY_CHANGE_SIZE |
      FILE_NOTIFY_CHANGE_LAST_WRITE |
      FILE_NOTIFY_CHANGE_CREATION);
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
    if (exitOnFirstChange) {
      break;
    }
  }
}

int wmain(int argc, wchar_t * argv[]) {
  if (argc < 2) {
    Usage(argv[0]);
    return 1;
  }

  bool exitOnFirstChange = false;
  std::wstring dir;
  for (int i = 1; i < argc; ++i) {
    const std::wstring param(argv[i]);
    if (param == L"-1" || param == L"--one-event") {
      exitOnFirstChange = true;
    }
    else if (param[0] == L'-') {
      // Ignore all other params.
    } else {
      dir = param;
    }
  }
  if (dir.empty()) {
    Usage(argv[0]);
    return 1;
  }
  WatchDirectory(dir, exitOnFirstChange);
  // Correct exit in case of --one-event.
  return 0;
}