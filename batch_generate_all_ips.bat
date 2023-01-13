@ECHO off
cls

for /l %%a in (171, 1, 175) do (
	CALL :compile_and_copy %%a, 1
	CALL :compile_and_copy %%a, 2
	for /l %%b in (10, 10, 50) do (
		CALL :compile_and_copy %%a, %%b
	)
)
EXIT /B %ERRORLEVEL%


:compile_and_copy
set openwrt_remote_root_dir=/home/fabio/Downloads/openwrt

set GIT_URL=https://github.com/thiagohd/mt76

set vm_user_ip=fabio@192.168.1.89

set router_ip=192.168.%~1.%~2
set curr_router_ip=%router_ip%

set working_dir="C:\Users\xpert\OneDrive\XPERT\Deicke\GitLab\XPtec\Firmware\mt76_deicke"

echo Maquina virtual: %vm_user_ip%:%openwrt_remote_root_dir% 
echo Novo endereco de IP a ser compilado: %router_ip%
echo.

del "C:\Users\xpert\.ssh\known_hosts"

cd %working_dir% 

echo Gerando dados do commit para compilacao!
git log -n 1 --date=short --pretty=format:PKG_SOURCE_VERSION:=%%H%%n > "hash.txt"
git log -n 1 --date=short --pretty=format:PKG_SOURCE_DATE:=%%ad%%n > "data.txt"
set /p HASH=<".\hash.txt"
set /p DATA=<".\data.txt"

del "hash.txt"
del "data.txt"

echo Atualizando PKG_SOURCE_URL para "%GIT_URL%"
plink -ssh -batch -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; sed -i '/PKG_SOURCE_URL/c\PKG_SOURCE_URL:=%GIT_URL%' %openwrt_remote_root_dir%/package/kernel/mt76/Makefile"

echo Atualizando PKG_SOURCE_VERSION para "%HASH%"
echo Atualizando PKG_SOURCE_DATE para "%DATA%"
plink -ssh -batch -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; sed -i '/PKG_SOURCE_VERSION/c\%HASH%' %openwrt_remote_root_dir%/package/kernel/mt76/Makefile ; sed -i '/PKG_SOURCE_DATE/c\%DATA%' %openwrt_remote_root_dir%/package/kernel/mt76/Makefile"


echo Copiando arquivos de configuracao base para o servidor
pscp -r -pw asd123 %working_dir%\files %vm_user_ip%:%openwrt_remote_root_dir%

echo Atualizando IP de configuracao para "%router_ip%"
plink -ssh -batch -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; sed -r -i 's/\b192.168.[0-9]{1,3}.[0-9]{1,3}\b/%router_ip%/g' %openwrt_remote_root_dir%/files/etc/config/network"
	
@REM	echo Atualizando HOSTNAME para "OpenWrt_%ip_msg%"
@REM	plink -ssh -batch -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; sed -r -i 's/\bOpenWrt_([0-9]{1,3}.*){1,4}\b/OpenWrt_%ip_msg%/g' %openwrt_remote_root_dir%/files/etc/config/system"


echo Compilando OpenWrt
plink -ssh -batch -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; make -j1"

echo Copiando binario gerado para diretorio local
pscp -pw asd123 "%vm_user_ip%:%openwrt_remote_root_dir%/bin/targets/ramips/mt76x8/openwrt-ramips-mt76x8-wavlink_wl-wn570ha1-squashfs-sysupgrade.bin" ./%router_ip%.bin


del "C:\Users\xpert\.ssh\known_hosts"

EXIT /B 0