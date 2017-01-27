## Very simple file watcher for Windows with MIT license

Monitors directory for any file/subdirectories changes and prints a line to stdout for every change detected.

Inspired by @emcrisostomo [fswatch](https://github.com/emcrisostomo/fswatch) project which did not compile successfully on Windows when it was necessary :)

Usage:
    myfswatch.exe [OPTIONS] <directory to watch>
OPTIONS are optional or any combination of the following:
    `-0` or `--print0`    use `NUL` ('\0') as delimiter instead of newline ('\n').
    `-1` or `--one-event` exit immediately after the first detected directory change.

### Known issues

* Current implementation does not print the name of the changed file.
* In continuous monitoring mode sometimes several events can be detected for one changed file.
