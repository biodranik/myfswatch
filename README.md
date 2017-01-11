## Very simple file watcher for Windows

Monitors directory for any file/subdirectories changes and prints a line to stdout for every change detected.

Usage:
    myfswatch.exe [-1|--one-event] <directory to watch>
With `-1` or `--one-event` argument exits immediately after the first directory change.

### Known issues

In continuous monitoring mode sometimes several events can be detected for one changed file.
