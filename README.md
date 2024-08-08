# Audio interfaces hooks for Windows 

## Alter the behavior of audio-recording or audio-playing software!

You can manipulate the behavior of any application regarding audio with this project.
For example:
* Deny audio recording/reproduction
* Steal audio reproduction from another process
* Reproduce an arbitrary stream
* ... you're only limited by your imagination!

### How to compile
For now you'll need to import this into your Visual Studio project. I'll try to provide a build system ASAP.

You'll also need to install capstone disassembler with vcpkg via
``vcpkg install capstone``

### Hook Architecture
![diagramfix drawio](https://github.com/user-attachments/assets/ec34a47f-762d-4c7a-89b5-9252b8c7f1d8)

### How it works
This program hooks into the common Windows interfaces that are exposed and used by virtually any software that plays or records data streams.

To gather the local method that does that, we search
for the common VTable in the instance of the injectee process. It then gets swapped with our arbitrary hook function that executes our code, and will eventually return the execution by calling the original entry.

All the registers at the beginning of the hook function are automatically saved onto the stack and then popped at the end by the Trampoline64 call. This is to prevent register corruption when returning the execution. That would have happened if we had changed any registers that would have been subsequently read.
