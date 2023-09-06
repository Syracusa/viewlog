# Build
```sh
mkdir build && cd build
cmake ..
make
```
# Usage
+ ./viewlog
+ Inputmode
  + COMMAND input mode
    + `
  + FILENAME input mode
+ Viewmode
  + REALTIME view mode
  + STOP view mode
+ Inputmode and viewmode state machine
```
({input mode}, {viewmode})

START => (COMMAND, REALTIME)
(COMMAND, REALTIME) => Press R => (COMMAND, STOP)
(COMMAND, STOP) => Press R => (COMMAND, REALTIME)
(COMMAND, REALTIME) => Press ` => (FILENAME, REALTIME)
(FILENAME, REALTIME) => Press ` => (COMMAND, REALTIME)
(FILENAME, REALTIME) => Type filename and ENTER => Target file update => (FILENAME, REALTIME)
```

# Test
+ Write dummy log periodically
  + ./dummy {filename(default = /tmp/viewlog/test1.log)}