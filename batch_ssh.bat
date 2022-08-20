@ECHO on
cls
set openwrt_remote_root_dir=~/Downloads/openwrt

set vm_user_ip=fabio@192.168.1.49

set router_ip=192.168.171.10
echo "Maquina virtual: " %vm_user_ip% ":/"%openwrt_remote_root_dir% 
echo "Roteador a atualizar: " %router_ip%

set commit_msg =
set /p commit_msg=Digite a mensagem e commit: 
echo %commit_msg%
cd "C:\Users\xpert\OneDrive\XPERT\Deicke\GitLab\XPtec\Firmware\mt76_deicke" 
git add --all
git commit -m "%commit_msg%"
git push 

git log -n 1 --date=short --pretty=format:PKG_SOURCE_VERSION:=%%H%%n > "hash.txt"
git log -n 1 --date=short --pretty=format:PKG_SOURCE_DATE:=%%ad%%n > "data.txt"

set /p HASH=<"hash.txt"
set /p DATA=<"data.txt"

del "hash.txt"
del "data.txt"


plink -ssh -batch -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; sed -r -i 's/\b192.168.171.[0-9]{1,3}\b/%router_ip%/g %openwrt_remote_root_dir%/files/etc/config/network' ; sed -i '/PKG_SOURCE_VERSION/c\%HASH%' %openwrt_remote_root_dir%/package/kernel/mt76/Makefile ; sed -i '/PKG_SOURCE_DATE/c\%DATA%' %openwrt_remote_root_dir%/package/kernel/mt76/Makefile"


plink -ssh -batch -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; make package/kernel/mt76/clean ; make package/kernel/mt76/compile V=99 ; make -j1"

scp %vm_user_ip%:%openwrt_remote_root_dir%/bin/targets/ramips/mt76x8/openwrt-ramips-mt76x8-wavlink_wl-wn570ha1-squashfs-sysupgrade.bin .

scp openwrt-ramips-mt76x8-wavlink_wl-wn570ha1-squashfs-sysupgrade.bin root@%router_ip%:/tmp/firmware.bin

echo y | plink -ssh root@%router_ip% "sysupgrade -v /tmp/firmware.bin"

pause