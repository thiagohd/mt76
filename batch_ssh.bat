@ECHO off
cls

set commit_msg =
set /p commit_msg=Digite a mensagem e commit: 
echo %commit_msg%
cd "C:\Users\xpert\OneDrive\XPERT\Deicke\GitLab\XPtec\Firmware\mt76_deicke" 
git add --all
git commit -m "%commit_msg%"

pause