// SPDX-License-Identifier: ISC

#include <linux/of.h>
#include "mt7603.h"
#include "eeprom.h"

static int
mt7603_efuse_read(struct mt7603_dev *dev, u32 base, u16 addr, u8 *data)
{
	u32 val;
	int i;

	val = mt76_rr(dev, base + MT_EFUSE_CTRL);
	val &= ~(MT_EFUSE_CTRL_AIN |
		 MT_EFUSE_CTRL_MODE);
	val |= FIELD_PREP(MT_EFUSE_CTRL_AIN, addr & ~0xf);
	val |= MT_EFUSE_CTRL_KICK;
	mt76_wr(dev, base + MT_EFUSE_CTRL, val);

	if (!mt76_poll(dev, base + MT_EFUSE_CTRL, MT_EFUSE_CTRL_KICK, 0, 1000))
		return -ETIMEDOUT;

	udelay(2);

	val = mt76_rr(dev, base + MT_EFUSE_CTRL);
	if ((val & MT_EFUSE_CTRL_AOUT) == MT_EFUSE_CTRL_AOUT ||
	    WARN_ON_ONCE(!(val & MT_EFUSE_CTRL_VALID))) {
		memset(data, 0xff, 16);
		return 0;
	}

	for (i = 0; i < 4; i++) {
		val = mt76_rr(dev, base + MT_EFUSE_RDATA(i));
		put_unaligned_le32(val, data + 4 * i);
	}

	return 0;
}

static int
mt7603_efuse_init(struct mt7603_dev *dev)
{
	u32 base = mt7603_reg_map(dev, MT_EFUSE_BASE);
	int len = MT7603_EEPROM_SIZE;
	void *buf;
	int ret, i, ret_aux = 0;
	printk("[deicke] EFUSE INIT !!!!!!!");
	if (mt76_rr(dev, base + MT_EFUSE_BASE_CTRL) & MT_EFUSE_BASE_CTRL_EMPTY){
		printk("[deicke] EFUSE EMPTY !!!!!!!");
		return 0;
	}

	dev->mt76.otp.data = devm_kzalloc(dev->mt76.dev, len, GFP_KERNEL);
	dev->mt76.otp.size = len;
	if (!dev->mt76.otp.data)
		return -ENOMEM;

	buf = dev->mt76.otp.data;
	for (i = 0; i + 16 <= len; i += 16) {
		ret = mt7603_efuse_read(dev, base, i, buf + i);
		printk("[deicke] READ 0x%08X = 0x%08X 0x%08X 0x%08X 0x%08X", (unsigned int)(u32 *)(base+i), *((u32 *)(buf)), *((u32 *)(buf+(unsigned int)4)), *((u32 *)(buf+(unsigned int)8)), *((u32 *)(buf+(unsigned int)12)));
		if (ret)
			ret_aux = ret;
	}
	if(ret_aux)
		return ret_aux;

	return 0;
}

static bool
mt7603_has_cal_free_data(struct mt7603_dev *dev, u8 *efuse)
{
	if (!efuse[MT_EE_TEMP_SENSOR_CAL])
		return false;

	if (get_unaligned_le16(efuse + MT_EE_TX_POWER_0_START_2G) == 0)
		return false;

	if (get_unaligned_le16(efuse + MT_EE_TX_POWER_1_START_2G) == 0)
		return false;

	if (!efuse[MT_EE_CP_FT_VERSION])
		return false;

	if (!efuse[MT_EE_XTAL_FREQ_OFFSET])
		return false;

	if (!efuse[MT_EE_XTAL_WF_RFCAL])
		return false;

	return true;
}

static void
mt7603_apply_cal_free_data(struct mt7603_dev *dev, u8 *efuse)
{
	static const u8 cal_free_bytes[] = {
		MT_EE_TEMP_SENSOR_CAL,
		MT_EE_CP_FT_VERSION,
		MT_EE_XTAL_FREQ_OFFSET,
		MT_EE_XTAL_WF_RFCAL,
		/* Skip for MT7628 */
		MT_EE_TX_POWER_0_START_2G,
		MT_EE_TX_POWER_0_START_2G + 1,
		MT_EE_TX_POWER_1_START_2G,
		MT_EE_TX_POWER_1_START_2G + 1,
	};
	struct device_node *np = dev->mt76.dev->of_node;
	u8 *eeprom = dev->mt76.eeprom.data;
	int n = ARRAY_SIZE(cal_free_bytes);
	int i;

	if (!np || !of_property_read_bool(np, "mediatek,eeprom-merge-otp"))
		return;

	if (!mt7603_has_cal_free_data(dev, efuse)){
		
		printk("[deicke] DONT HAVE CALL FREE DATA");
		return;
	}

	if (is_mt7628(dev)){
		printk("[deicke] is_mt7628");
		n -= 4;
	}else{
		printk("[deicke] NOT is_mt7628");
	}

	for (i = 0; i < n; i++) {
		int offset = cal_free_bytes[i];

		eeprom[offset] = efuse[offset];
	}
}

static int
mt7603_eeprom_load(struct mt7603_dev *dev)
{
	int ret;

	ret = mt76_eeprom_init(&dev->mt76, MT7603_EEPROM_SIZE);
	if (ret < 0){
		if(ret == 0) printk("[deicke] mt76_eeprom_init ret < 0");
		return ret;
	}
	if(ret == 0) printk("[deicke] Não foi possivel carregar da EEPROM");
	else printk("[deicke] Parece que foi possivel carregar da EEPROM");

	return mt7603_efuse_init(dev);
}

static int mt7603_check_eeprom(struct mt76_dev *dev)
{
	u16 val = get_unaligned_le16(dev->eeprom.data);

	switch (val) {
	case 0x7628:
	case 0x7603:
	case 0x7600:
		return 0;
	default:
		return -EINVAL;
	}
}

static inline bool is_mt7688(struct mt7603_dev *dev)
{
	return mt76_rr(dev, MT_EFUSE_BASE + 0x64) & BIT(4);
}

#define FORCE_USE_OTP_DATA 1

int mt7603_eeprom_init(struct mt7603_dev *dev)
{
	u8 *eeprom;
	int ret;

	ret = mt7603_eeprom_load(dev);
	printk("[deicke] mt7603 eeprom load = %d", ret);
	if (ret < 0)
		return ret;

	if (dev->mt76.otp.data) {
		printk("[deicke] otp.data valid");
		if (!FORCE_USE_OTP_DATA && mt7603_check_eeprom(&dev->mt76) == 0){
			printk("[deicke] check_eeprom == 0");
			mt7603_apply_cal_free_data(dev, dev->mt76.otp.data);
		}else{
			printk("[deicke] check_eeprom != 0");
			if(!FORCE_USE_OTP_DATA){
				memcpy(dev->mt76.eeprom.data, dev->mt76.otp.data, MT7603_EEPROM_SIZE);
			}
		}	
	}

	eeprom = (u8 *)dev->mt76.eeprom.data;
	dev->mphy.cap.has_2ghz = true;
	memcpy(dev->mphy.macaddr, eeprom + MT_EE_MAC_ADDR, ETH_ALEN);
	if(FORCE_USE_OTP_DATA){
		printk("[deicke] FORCING OTP DATA");
		memcpy(dev->mt76.eeprom.data, dev->mt76.otp.data, MT7603_EEPROM_SIZE);
		eeprom = (u8 *)dev->mt76.eeprom.data;
	}

	/* Check for 1SS devices */
	dev->mphy.antenna_mask = 3;
	if (FIELD_GET(MT_EE_NIC_CONF_0_RX_PATH, eeprom[MT_EE_NIC_CONF_0]) == 1 ||
	    FIELD_GET(MT_EE_NIC_CONF_0_TX_PATH, eeprom[MT_EE_NIC_CONF_0]) == 1 ||
	    is_mt7688(dev))
		dev->mphy.antenna_mask = 1;

	mt76_eeprom_override(&dev->mphy);

	return 0;
}
