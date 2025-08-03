# Linux System Programming Project 1: ssu-cleanup
`ssu-cleanup` is a CLI tool that **organizes files by extension** and **displays directory structures**.
It provides a custom shell with built-in commands: `tree`, `arrange`, `help`, and `exit`.


## Implementation
All features are implemented using Linux system calls - no external libraries or `system()` calls.
- Operates only within the user's `$HOME`
- Uses **linked list** structures to manage directory contents
- Respects system limits (e.g., `PATH_MAX`, `NAME_MAX`)
- Handles invalid paths, commands, and options with usage feedback


## Commands
### `tree <DIR_PATH> [OPTIONS]`
Displays the directory tree recursively.
- `-s`: show file size
- `-p`: show file permissions
- `-sp` or `-ps`: show both


### `arrange <DIR_PATH> [OPTIONS]`
Organizes files in the directory by extension into `<DIR_PATH>_arranged`.
- `-d <output_path>`: set output directory
- `-t <seconds>`: only files modified before given `seconds`
- `-x <exclude_paths>`: skip specific subdirectories
- `-e <extensions>`: only include specific extensions
Handles name conflicts with interactive options: `select`, `diff`, `vi`, `skip`.


### `help`
Prints usage for all commands.


### `exit`
Exits the program.


## Build
- Compile
```
$ make
```
- Execute
```
$ ./ssu_cleanup
20232372>
20232372> exit
$
```
- Remove
```
 $ make clean
```
