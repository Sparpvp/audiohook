# Audio interfaces hooks for Windows 

## Alter the behavior of audio-recording or audio-playing software!

You can manipulate the behavior of any application regarding audio with this project.
For example:
* Deny audio recording/reproduction
* Steal audio reproduction from another process
* Reproduce an arbitrary stream
* ... you're only limited by your imagination!

_Originally, with this project I wanted to cypher the audio stream sent to VoIP applications' servers with an asymmetric key that would have been agreed upon between the two parties. However, now that the project is so modular, I may add it in the future to an examples/projects directory._

### How to compile
You'll need to install capstone disassembler, I recommend doing it with vcpkg via  
``vcpkg install capstone``

Then, you can directly build this project with the Visual Studio (2022/v143) vcxproj by specifying Release x64, or you can use CMake as follows:
```shell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --target ALL_BUILD --config Release -- /nologo /verbosity:minimal /maxcpucount
```

### Hook Architecture
![diagram](https://github.com/user-attachments/assets/ec34a47f-762d-4c7a-89b5-9252b8c7f1d8)

### How it works
This program hooks into the common Windows interfaces that are exposed and used by virtually any software that plays or records data streams.

To gather the local method that does that, we search
for the common VTable in the instance of the injectee process. It then gets swapped with our arbitrary hook function that executes our code, and will eventually return the execution by calling the original entry.

All the registers at the beginning of the hook function are automatically saved onto the stack and then popped at the end by the Trampoline64 call. This is to prevent register corruption when returning the execution. That would have happened if we had changed any registers that would have been subsequently read.

### TODO
- [ ] Switch from BeaEngine to Capstone completely
- [ ] Change dynamic library (.lib) search path from the injectee executable's directory
- [ ] Audio cypher/decipher example