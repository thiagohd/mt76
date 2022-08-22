@ECHO off
cls
set openwrt_remote_root_dir=/home/fabio/Downloads/openwrt

set vm_user_ip=fabio@192.168.1.89

set router_ip=192.168.171.
set ip_msg =
set /p ip_msg=Digite o final do IP do roteador (1, 10, 20, 30 ...): 

set router_ip=%router_ip%%ip_msg%

echo Maquina virtual: %vm_user_ip%:%openwrt_remote_root_dir% 
echo Roteador a atualizar: %router_ip%
echo.
set commit_msg =
echo Digite uma mensagem para commitar o projeto, ou digite as opcoes abaixo e pressione enter:
echo [ 1 ] para cancelar operacao
echo [ 2 ] para pular commit e seguir para compilacao/gravacao
set /p commit_msg=

if %commit_msg%==1 (
	echo Cancelado!
	goto finish
)
echo continuar
pause
del "C:\Users\xpert\.ssh\known_hosts"
cd "C:\Users\xpert\OneDrive\XPERT\Deicke\GitLab\XPtec\Firmware\mt76_deicke" 
if NOT %commit_msg%==2 (
	echo Commitando projeto...
	git add --all
	git commit -m "%commit_msg%"
	git push 

	git log -n 1 --date=short --pretty=format:PKG_SOURCE_VERSION:=%%H%%n > "hash.txt"
	git log -n 1 --date=short --pretty=format:PKG_SOURCE_DATE:=%%ad%%n > "data.txt"
	set /p HASH=<"hash.txt"
	set /p DATA=<"data.txt"

	del "hash.txt"
	del "data.txt"
	
	plink -ssh -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; sed -i '/PKG_SOURCE_VERSION/c\%HASH%' %openwrt_remote_root_dir%/package/kernel/mt76/Makefile ; sed -i '/PKG_SOURCE_DATE/c\%DATA%' %openwrt_remote_root_dir%/package/kernel/mt76/Makefile"
)

plink -ssh -batch -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; sed -r -i 's/\b192.168.171.[0-9]{1,3}\b/%router_ip%/g' %openwrt_remote_root_dir%/files/etc/config/network"

plink -ssh -batch -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; sed -r -i 's/\bOpenWrt_([0-9]{1,3}.){4}\b/OpenWrt_%router_ip%/g' %openwrt_remote_root_dir%/files/etc/config/system"

plink -ssh -batch -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; make package/kernel/mt76/clean ; make package/kernel/mt76/compile V=99 ; make -j1"

pscp -pw asd123 "%vm_user_ip%:%openwrt_remote_root_dir%/bin/targets/ramips/mt76x8/openwrt-ramips-mt76x8-wavlink_wl-wn570ha1-squashfs-sysupgrade.bin" ./%router_ip%.bin

scp ./%router_ip%.bin root@%router_ip%:/tmp/firmware.bin

echo y | plink -ssh root@%router_ip% "sysupgrade -v -n /tmp/firmware.bin"

:finish
pause