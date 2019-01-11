call clean
copy i:\sys\sci\vcs.cfg
copy i:\sys\sci\vcscfg.bat
what c "Get Makefiles? " yn
if %what% == N goto SrcFiles
copy i:makefile
copy i:makeinit
:SrcFiles
what c "Get Source files? " yn
if %what% == N goto WinFiles
copy i:\sys\sci\sierra.err
copy i:\sys\sci\*.i
copy i:\sys\sci\*.h
copy i:\sys\sci\*.asm
copy i:\sys\sci\*.?pp
:WinFiles
what c "Get Windows files? " yn
if %what% == N goto ObjFiles
copy i:*.rc
copy i:*.res
copy i:*.rex
copy i:\sys\sci\*.ico
:ObjFiles
what c "Get object files? " yn
if %what% == N goto End
copy i:*.obj
:End
