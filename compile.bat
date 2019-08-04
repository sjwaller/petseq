@ECHO OFF
SET CC65_HOME=E:\downloads\commodore\PET\Applications\cc65-snapshot-win32
REM PATH=%PATH%;%CC65_HOME%\bin;

%CC65_HOME%\bin\cl65.exe -t pet -o target.prg petseq.c

pause