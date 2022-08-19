@ECHO off
cls

set commit_msg =
set /p commit_msg=Digite a mensagem e commit: 
echo %commit_msg%
cd "C:\Users\xpert\OneDrive\XPERT\Deicke\GitLab\XPtec\Firmware\mt76_deicke" 
git add --all
git commit -m "%commit_msg%"
git push 

git log -n 1 --date=short --pretty=format:PKG_SOURCE_VERSION:=%%H%%n > "C:\Users\xpert\OneDrive\XPERT\Deicke\GitLab\XPtec\Firmware\mt76_deicke\hash.txt"
git log -n 1 --date=short --pretty=format:PKG_SOURCE_DATE:=%%ad%%n > "C:\Users\xpert\OneDrive\XPERT\Deicke\GitLab\XPtec\Firmware\mt76_deicke\data.txt"

set /p HASH=<"C:\Users\xpert\OneDrive\XPERT\Deicke\GitLab\XPtec\Firmware\mt76_deicke\hash.txt"
set /p DATA=<"C:\Users\xpert\OneDrive\XPERT\Deicke\GitLab\XPtec\Firmware\mt76_deicke\data.txt"

del "C:\Users\xpert\OneDrive\XPERT\Deicke\GitLab\XPtec\Firmware\mt76_deicke\hash.txt"
del "C:\Users\xpert\OneDrive\XPERT\Deicke\GitLab\XPtec\Firmware\mt76_deicke\data.txt"


plink -ssh -batch -pw asd123 fabio@192.168.1.49:~/Downloads/openwrt "sed -e '/PKG_SOURCE_VERSION:/{r %HASH%' -e 'd}' package/kernel/mt76/Makefile > package/kernel/mt76/makefile ; cp package/kernel/mt76/makefile package/kernel/mt76/Makefile ; sed -e '/PKG_SOURCE_DATE:/{r %DATA%' -e 'd}' package/kernel/mt76/Makefile > package/kernel/mt76/makefile ; cp package/kernel/mt76/makefile package/kernel/mt76/Makefile"

plink -ssh -batch -pw asd123 fabio@192.168.1.49:~/Downloads/openwrt "make package/kernel/mt76/clean ; make package/kernel/mt76/compile V=99"


pause