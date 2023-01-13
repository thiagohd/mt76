# Introdução

Deve-se ter um sistema linux preparado para compilação, seguindo as instruções abaixo. Atenção, nenhuma ação deve ser feita em usuário root, pois alguns pacotes a serem compilados sofrem erros por conta disso.
https://openwrt.org/docs/guide-developer/toolchain/use-buildsystem

Feitos os procedimentos abaixo, o projeto pode ser compilado de forma mais automática usando os scripts BATCH presentes neste repositório.
Pra tudo funcionar, precisa-se ter um repositório no Github com este projeto commitado lá, e deve-se substituir o caminho do repositório nos arquivos BATCH, pois a toolchain vai buscar o driver mt76 lá no Github.
Pra entender como fazer na mão basta olhar os scripts BATCH.

# Clonar projeto OpenWRT

Clone a branch desejada, neste caso v22.03.0.

`
git clone -b v22.03.0 --single-branch http://github.com/openwrt/openwrt.git
`

# IP e Configurações Padrão de Pacotes de Rede

Os arquivos de configuração padrão devem ser colocados na pasta do projeto conforme ficam no linux. Por exemplo, para os arquivos da pasta `/etc/config`, devem ser colocados em:

`
openwrt/files/etc/config/
`

Isso fará com que eles saiam "embutidos" já no Linux da forma como quiser.

Neste repositório, esses arquivos estão na pasta `files` e são enviados automaticamente pelo BATCH.


## Opção 2

Existe a opção de alterar o uci-defaults: https://openwrt.org/docs/guide-developer/uci-defaults


## Opção 3

Existe a opção de alterar o ip padrão no arquivo abaixo (procurando 192.168.1.1): 
`
openwrt/package/base-files/files/bin/config_generate 
`

# Atualizando e Incluindo Pacotes para Compilação

`
openwrt/scripts/feeds update -a
`

Pode-se escolher por instalar todos os packages, desta forma estarão disponíveis no menuconfig:

`
openwrt/scripts/feeds install -a
`

Ou então instalar um por um:

```
openwrt/scripts/feeds install -a -p kmod-batman-adv
openwrt/scripts/feeds install -a -p batctl
openwrt/scripts/feeds install -a -p batctl-full ---> nao precisa
openwrt/scripts/feeds install -a -p luci
openwrt/scripts/feeds install -a -p iperf --> ou netperf (dá pra usar Flent) - não precisa
openwrt/scripts/feeds install -a -p htop --> não precisa
openwrt/scripts/feeds install -a -p auc --> não preicsa
openwrt/scripts/feeds install -a -p netdata --> não cabe na memória
openwrt/scripts/feeds install luci-proto-batman-adv --> nao precisa
openwrt/scripts/feeds install luci-app-firewall --> não precisa
```

# Configurando Target e Pacotes para Compilação

## Opção 1 - Utilizar configuração existente

A configuração atualmente utilizada se encontra na pasta `arquivos_config_openwrt`.
Caso utilizar a branch v22.03.0 do OpenWRT, basta copiar o arquivo `.config` para a pasta clonada do projeto (ATENÇÃO: no repositório está sem o . no nome do arquivo).

Também tem-se a opção de usar o arquivo diffconfig, que foi gerado a partir do comando:

`
openwrt/scripts/diffconfig.sh > diffconfig
`

Esse arquivo diffconfig é bom pois mantém a compatibilidade do arquivo de configuração entre releases antigas e novas da toolchain. Caso usar uma branch diferente da v22.03.0 recomenda-se gerar o arquivo `.config` a partir do comando:

```
# Substitui o arquivo de configuração
cp diffconfig .config
 
# Faz a magia acontecer
make defconfig
```

## Opção 2 - Configurar do zero

`
make menuconfig
`

-- Selecionar Target System: Mediatek Ralink MIPS

-- Selecionar Subtarget: MT76x8 based boards

-- Selecionar Target Profile: Wavlink WL-WN570HA1


## Rede mesh (precisa)

Desabilitar wpad-basic-openssl/wolfssl, por problemas com incompatibilidades:

> Network > WirelessAPD > < >wpad-basic[-openssl/wolfssl]

Habilitar wpad-mesh-openssl:

> Network > WirelessAPD > <*>wpad-mesh-openssl

Habilitar kmod-batman-adv:

> Kernel modules > Network Support > <*>kmod-batman-adv

## Luci / página web (precisa)

Habilitar luci:

> LuCI > Collections > <*> luci

## Testes (não precisa)

Habilitar iperf para testes:

> Network > iperf <*>

## Ferramenta de update assistido (não precisa)

Habilitar attended upgrade

> Base system > <*> auc

> LuCI > Applications > <*> luci-app-attendedsysupgrade

## Ferramentas wireless (não precisa)

Habilitar wireless-tools (iwpriv, iwconfig, iwlist)


# Compilando

Para compilar tudo.

`
make -j1
`

Para compilar um package específico, por exemplo para batman-adv: https://openwrt.org/docs/guide-developer/toolchain/single.package

```
make package/batman-adv/clean
make package/batman-adv/compile V=99
```


# Para testes de desenvolvimento

Para alterar um feed de algum pacote, como o Luci por exemplo, pode-se utilizar uma branch mais atualizada para testes.
Basta alterar o arquivo que fica na pasta raíz do openwrt, na linha correspondente ao luci:

`
nano feeds.conf.default
`