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


plink -ssh -batch -pw asd123 fabio@192.168.1.49 "cd ~/Downloads/openwrt ; sed -i '/PKG_SOURCE_VERSION/c\%HASH%' ~/Downloads/openwrt/package/kernel/mt76/Makefile ; sed -i '/PKG_SOURCE_DATE/c\%DATA%' ~/Downloads/openwrt/package/kernel/mt76/Makefile"

plink -ssh -batch -pw asd123 fabio@192.168.1.49 "cd ~/Downloads/openwrt ; make package/kernel/mt76/clean ; make -j1"

scp fabio@192.168.1.49:~/Downloads/openwrt/bin/targets/ramips/mt76x8/openwrt-ramips-mt76x8-wavlink_wl-wn570ha1-squashfs-sysupgrade.bin .

scp openwrt-ramips-mt76x8-wavlink_wl-wn570ha1-squashfs-sysupgrade.bin root@192.168.171.1:/tmp/firmware.bin

plink -ssh -batch root@192.168.171.1 "sysupgrade -v /tmp/firmware.bin"

pause