# Humanix Standard v1.0

## 1. Manifesto

The POSIX command system is too anti-human: it features confusing abbreviations, inconsistent options, and dangerous default behaviors. We designed Humanix with the goal of creating a **simpler, more consistent, safer, and more modern** command-line standard, and ultimately to provide a lightweight core toolset capable of fully replacing Busybox.

## 2. Core Design Principles

- Command names should be short and clear
- Options should be placed at the very beginning of the command (in the form [option])
- Support smart defaults + explicit options
- Retain common short aliases to reduce migration costs
- Dangerous operations are safe by default and require explicit enablement

## 3. List of Core Commands

### Create
```humanix
crt [d] [f] [force] <name>

- No options: Smart mode (files with common extensions → files; no extension → directory)
- [d]: Force creation of a directory (recursively creates subdirectories by default)
- [f]: Force creation of a file
- [d] and [f] together: Report an error
```

### File/Directory Operations
```humanix
list [human] [sort=name|size|time] [long] <path>  
copy [r] [force] <src> <dst>  
move [force] <src> <dst>  
delete [r] [force] [preview] <target>...
```

### View
```humanix
show [page] [follow] <file>
```

### Navigation
```humanix
cd <path>                  # Supports .. / ... / .... and other multi-level upward navigation  
cd -                       # Return to the previous directory
```

### Processes (Unified)
```humanix
process list [sort=cpu|mem]  
process kill <pid|name> [force]  
process find <name>  
process stop <name>
```

### Permissions (Simplified)
```humanix
perm [r] <mode> <target>  
own  [r] <owner> <target>
```

### Text Processing (Unified as `echo`)
```humanix
echo “Content”  
echo “Content” > file.txt  
echo “Content” >> file.txt  
echo grep “keyword” <file>  
echo replace “old” “new” <file> [preview]  
echo count <file>
```

### Search
```humanix
find [name=*.log] [content=keyword] [path=.]
```

### System Information
```humanix
sys info [cpu|mem|disk]  
sys uptime
```

### Miscellaneous
```humanix
help [command]  
exit
```

## 4. Recommended Parameter Style

**Preferred Style (Options First)**:
```humanix
crt [d] myproject
delete [r] [force] tempdir
list [human] [sort=size]
copy [r] src dst
```

## Optional: Command Line Editing

For better shell experience (history, arrow keys), install libreadline:
```bash
# Ubuntu/Debian
sudo apt install libreadline-dev

# RedHat/CentOS
sudo dnf install readline-devel
```

Then rebuild
```bash
cmake --build .
```

Without readline, the shell still works but without line editing and command history.