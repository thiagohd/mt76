@ECHO off
cls
set openwrt_remote_root_dir=/home/fabio/Downloads/openwrt

set GIT_URL=https://github.com/thiagohd/mt76

set vm_user_ip=fabio@192.168.1.89

set router_ip=192.168.171.
set ip_msg =
set /p ip_msg=Digite o final do IP desejado para o roteador (1, 10, 20, 30 ...): 
set /p ip_curr=Digite o final do IP atual do roteador (ou deixe vazio para usar o mesmo acima): 

set curr_router_ip=%router_ip%%ip_msg%
IF NOT [%ip_curr%]==[] (
	set curr_router_ip=%router_ip%%ip_curr%
)
set router_ip=%router_ip%%ip_msg%

set working_dir="C:\Users\xpert\OneDrive\XPERT\Deicke\GitLab\XPtec\Firmware\mt76_deicke"

echo Maquina virtual: %vm_user_ip%:%openwrt_remote_root_dir% 
echo Roteador a atualizar: %curr_router_ip%
echo Novo endereco de IP a ser configurado: %router_ip%
echo.
set commit_msg =
echo Digite uma mensagem para commitar o projeto, ou digite as opcoes abaixo e pressione enter:
echo [ 1 ] para cancelar operacao
echo [ 2 ] para pular commit e seguir para compilacao/gravacao
echo [ 3 ] para somente atualizar com o ultimo binario compilado (garanta que seja o mesmo IP!)
set commit_msg=

set /p commit_msg=

set commit_msg="%commit_msg%"


IF %commit_msg%=="1" (
	echo Cancelado!
	goto finish
)


del "C:\Users\xpert\.ssh\known_hosts"



cd %working_dir% 

if NOT %commit_msg%=="2" (
	if NOT %commit_msg%=="3" (
		echo Commitando projeto... mensagem do commit: %commit_msg%
		git add --all
		git commit -m %commit_msg%
		git push 
		echo Projeto commitado!
	)
)

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

if NOT %commit_msg%=="3" (
	echo Copiando arquivos de configuracao base para o servidor
	pscp -r -pw asd123 %working_dir%\files %vm_user_ip%:%openwrt_remote_root_dir%
	
	echo Atualizando IP de configuracao para "%router_ip%"
	plink -ssh -batch -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; sed -r -i 's/\b192.168.171.[0-9]{1,3}\b/%router_ip%/g' %openwrt_remote_root_dir%/files/etc/config/network"
	
	echo Atualizando HOSTNAME para "OpenWrt_%ip_msg%"
	plink -ssh -batch -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; sed -r -i 's/\bOpenWrt_([0-9]{1,3}.*){1,4}\b/OpenWrt_%ip_msg%/g' %openwrt_remote_root_dir%/files/etc/config/system"

	if NOT %commit_msg%=="2" (
		echo Limpando compilacao anterior e recompilando MT76
		plink -ssh -batch -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; make package/kernel/mt76/clean ; make package/kernel/mt76/compile V=99"
	)
	echo Compilando OpenWrt
	plink -ssh -batch -pw asd123 %vm_user_ip% "cd %openwrt_remote_root_dir% ; make -j1"
)

echo Copiando binario gerado para diretorio local
pscp -pw asd123 "%vm_user_ip%:%openwrt_remote_root_dir%/bin/targets/ramips/mt76x8/openwrt-ramips-mt76x8-wavlink_wl-wn570ha1-squashfs-sysupgrade.bin" ./%router_ip%.bin

echo Copiando binario para o roteador %curr_router_ip%
pscp -scp ./%router_ip%.bin root@%curr_router_ip%:/tmp/firmware.bin

del "C:\Users\xpert\.ssh\known_hosts"
echo Enviando comando de atualizacao para o roteador %curr_router_ip%
echo y | plink -ssh -batch root@%curr_router_ip% "sysupgrade -v -n /tmp/firmware.bin"

:finish
pause