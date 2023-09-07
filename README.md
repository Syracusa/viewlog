# Viewlog
+ It's like a GNU tail with -f option but...
  + Able to stop/restart 
  + Can easily change target file.

# Build
```sh
mkdir build && cd build
cmake ..
make
```
# Usage
+ ./viewlog
+ Press 'R' to stop/restart
+ Press '`' to change input mode and type filepath to change target file

# Test
+ Write dummy log periodically
  + ./dummy {filename(default = /tmp/viewlog/test1.log)}