@ECHO off
cls

set commit_msg =
set /p commit_msg=Digite a mensagem e commit: 
echo %commit_msg%
cd "C:\Users\xpert\OneDrive\XPERT\Deicke\GitLab\XPtec\Firmware\mt76_deicke" 
git add --all
git commit -m "%commit_msg%"
git push 

git log -n 1 --date=short --pretty=format:'PKG_SOURCE_VERSION:=%%H%%n' > %HASH%
git log -n 1 --date=short --pretty=format:'PKG_SOURCE_DATE:=%%ad%%n' > %DATA%

echo %HASH%
echo %DATA%


pause