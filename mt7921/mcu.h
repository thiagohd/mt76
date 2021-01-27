/* SPDX-License-Identifier: ISC */
/* Copyright (C) 2020 MediaTek Inc. */

#ifndef __MT7921_MCU_H
#define __MT7921_MCU_H

#include "../mt76_connac_mcu.h"

struct mt7921_mcu_txd {
	__le32 txd[8];

	__le16 len;
	__le16 pq_id;

	u8 cid;
	u8 pkt_type;
	u8 set_query; /* FW don't care */
	u8 seq;

	u8 uc_d2b0_rev;
	u8 ext_cid;
	u8 s2d_index;
	u8 ext_cid_ack;

	u32 reserved[5];
} __packed __aligned(4);

/**
 * struct mt7921_uni_txd - mcu command descriptor for firmware v3
 * @txd: hardware descriptor
 * @len: total length not including txd
 * @cid: command identifier
 * @pkt_type: must be 0xa0 (cmd packet by long format)
 * @frag_n: fragment number
 * @seq: sequence number
 * @checksum: 0 mean there is no checksum
 * @s2d_index: index for command source and destination
 *  Definition              | value | note
 *  CMD_S2D_IDX_H2N         | 0x00  | command from HOST to WM
 *  CMD_S2D_IDX_C2N         | 0x01  | command from WA to WM
 *  CMD_S2D_IDX_H2C         | 0x02  | command from HOST to WA
 *  CMD_S2D_IDX_H2N_AND_H2C | 0x03  | command from HOST to WA and WM
 *
 * @option: command option
 *  BIT[0]: UNI_CMD_OPT_BIT_ACK
 *          set to 1 to request a fw reply
 *          if UNI_CMD_OPT_BIT_0_ACK is set and UNI_CMD_OPT_BIT_2_SET_QUERY
 *          is set, mcu firmware will send response event EID = 0x01
 *          (UNI_EVENT_ID_CMD_RESULT) to the host.
 *  BIT[1]: UNI_CMD_OPT_BIT_UNI_CMD
 *          0: original command
 *          1: unified command
 *  BIT[2]: UNI_CMD_OPT_BIT_SET_QUERY
 *          0: QUERY command
 *          1: SET command
 */
struct mt7921_uni_txd {
	__le32 txd[8];

	/* DW1 */
	__le16 len;
	__le16 cid;

	/* DW2 */
	u8 reserved;
	u8 pkt_type;
	u8 frag_n;
	u8 seq;

	/* DW3 */
	__le16 checksum;
	u8 s2d_index;
	u8 option;

	/* DW4 */
	u8 reserved2[4];
} __packed __aligned(4);

/* event table */
enum {
	MCU_EVENT_REG_ACCESS = 0x05,
	MCU_EVENT_SCAN_DONE = 0x0d,
	MCU_EVENT_BSS_ABSENCE  = 0x11,
	MCU_EVENT_BSS_BEACON_LOSS = 0x13,
	MCU_EVENT_CH_PRIVILEGE = 0x18,
	MCU_EVENT_SCHED_SCAN_DONE = 0x23,
	MCU_EVENT_DBG_MSG = 0x27,
};

/* ext event table */
enum {
	MCU_EXT_EVENT_RATE_REPORT = 0x87,
};

struct mt7921_mcu_rxd {
	__le32 rxd[6];

	__le16 len;
	__le16 pkt_type_id;

	u8 eid;
	u8 seq;
	__le16 __rsv;

	u8 ext_eid;
	u8 __rsv1[2];
	u8 s2d_index;
};

struct mt7921_mcu_eeprom_info {
	__le32 addr;
	__le32 valid;
	u8 data[16];
} __packed;

#define MT_RA_RATE_NSS			GENMASK(8, 6)
#define MT_RA_RATE_MCS			GENMASK(3, 0)
#define MT_RA_RATE_TX_MODE		GENMASK(12, 9)
#define MT_RA_RATE_DCM_EN		BIT(4)
#define MT_RA_RATE_BW			GENMASK(14, 13)

#define MCU_PQ_ID(p, q)			(((p) << 15) | ((q) << 10))
#define MCU_PKT_ID			0xa0

enum {
	MCU_Q_QUERY,
	MCU_Q_SET,
	MCU_Q_RESERVED,
	MCU_Q_NA
};

enum {
	MCU_S2D_H2N,
	MCU_S2D_C2N,
	MCU_S2D_H2C,
	MCU_S2D_H2CN
};

struct mt7921_mcu_uni_event {
	u8 cid;
	u8 pad[3];
	__le32 status; /* 0: success, others: fail */
} __packed;

struct mt7921_wow_ctrl_tlv {
	__le16 tag;
	__le16 len;
	u8 cmd; /* 0x1: PM_WOWLAN_REQ_START
		 * 0x2: PM_WOWLAN_REQ_STOP
		 * 0x3: PM_WOWLAN_PARAM_CLEAR
		 */
	u8 trigger; /* 0: NONE
		     * BIT(0): NL80211_WOWLAN_TRIG_MAGIC_PKT
		     * BIT(1): NL80211_WOWLAN_TRIG_ANY
		     * BIT(2): NL80211_WOWLAN_TRIG_DISCONNECT
		     * BIT(3): NL80211_WOWLAN_TRIG_GTK_REKEY_FAILURE
		     * BIT(4): BEACON_LOST
		     * BIT(5): NL80211_WOWLAN_TRIG_NET_DETECT
		     */
	u8 wakeup_hif; /* 0x0: HIF_SDIO
			* 0x1: HIF_USB
			* 0x2: HIF_PCIE
			* 0x3: HIF_GPIO
			*/
	u8 pad;
	u8 rsv[4];
} __packed;

struct mt7921_wow_gpio_param_tlv {
	__le16 tag;
	__le16 len;
	u8 gpio_pin;
	u8 trigger_lvl;
	u8 pad[2];
	__le32 gpio_interval;
	u8 rsv[4];
} __packed;

#define MT7921_WOW_MASK_MAX_LEN		16
#define MT7921_WOW_PATTEN_MAX_LEN	128
struct mt7921_wow_pattern_tlv {
	__le16 tag;
	__le16 len;
	u8 index; /* pattern index */
	u8 enable; /* 0: disable
		    * 1: enable
		    */
	u8 data_len; /* pattern length */
	u8 pad;
	u8 mask[MT7921_WOW_MASK_MAX_LEN];
	u8 pattern[MT7921_WOW_PATTEN_MAX_LEN];
	u8 rsv[4];
} __packed;

struct mt7921_suspend_tlv {
	__le16 tag;
	__le16 len;
	u8 enable; /* 0: suspend mode disabled
		    * 1: suspend mode enabled
		    */
	u8 mdtim; /* LP parameter */
	u8 wow_suspend; /* 0: update by origin policy
			 * 1: update by wow dtim
			 */
	u8 pad[5];
} __packed;

struct mt7921_gtk_rekey_tlv {
	__le16 tag;
	__le16 len;
	u8 kek[NL80211_KEK_LEN];
	u8 kck[NL80211_KCK_LEN];
	u8 replay_ctr[NL80211_REPLAY_CTR_LEN];
	u8 rekey_mode; /* 0: rekey offload enable
			* 1: rekey offload disable
			* 2: rekey update
			*/
	u8 keyid;
	u8 pad[2];
	__le32 proto; /* WPA-RSN-WAPI-OPSN */
	__le32 pairwise_cipher;
	__le32 group_cipher;
	__le32 key_mgmt; /* NONE-PSK-IEEE802.1X */
	__le32 mgmt_group_cipher;
	u8 option; /* 1: rekey data update without enabling offload */
	u8 reserverd[3];
} __packed;

struct mt7921_arpns_tlv {
	__le16 tag;
	__le16 len;
	u8 mode;
	u8 ips_num;
	u8 option;
	u8 pad[1];
} __packed;

enum {
	PATCH_NOT_DL_SEM_FAIL,
	PATCH_IS_DL,
	PATCH_NOT_DL_SEM_SUCCESS,
	PATCH_REL_SEM_SUCCESS
};

enum {
	FW_STATE_INITIAL,
	FW_STATE_FW_DOWNLOAD,
	FW_STATE_NORMAL_OPERATION,
	FW_STATE_NORMAL_TRX,
	FW_STATE_WACPU_RDY        = 7
};

enum {
	EE_MODE_EFUSE,
	EE_MODE_BUFFER,
};

enum {
	EE_FORMAT_BIN,
	EE_FORMAT_WHOLE,
	EE_FORMAT_MULTIPLE,
};

enum {
	MCU_PHY_STATE_TX_RATE,
	MCU_PHY_STATE_RX_RATE,
	MCU_PHY_STATE_RSSI,
	MCU_PHY_STATE_CONTENTION_RX_RATE,
	MCU_PHY_STATE_OFDMLQ_CNINFO,
};

#define STA_TYPE_STA			BIT(0)
#define STA_TYPE_AP			BIT(1)
#define STA_TYPE_ADHOC			BIT(2)
#define STA_TYPE_WDS			BIT(4)
#define STA_TYPE_BC			BIT(5)

#define NETWORK_INFRA			BIT(16)
#define NETWORK_P2P			BIT(17)
#define NETWORK_IBSS			BIT(18)
#define NETWORK_WDS			BIT(21)

#define CONNECTION_INFRA_STA		(STA_TYPE_STA | NETWORK_INFRA)
#define CONNECTION_INFRA_AP		(STA_TYPE_AP | NETWORK_INFRA)
#define CONNECTION_P2P_GC		(STA_TYPE_STA | NETWORK_P2P)
#define CONNECTION_P2P_GO		(STA_TYPE_AP | NETWORK_P2P)
#define CONNECTION_IBSS_ADHOC		(STA_TYPE_ADHOC | NETWORK_IBSS)
#define CONNECTION_WDS			(STA_TYPE_WDS | NETWORK_WDS)
#define CONNECTION_INFRA_BC		(STA_TYPE_BC | NETWORK_INFRA)

struct sec_key {
	u8 cipher_id;
	u8 cipher_len;
	u8 key_id;
	u8 key_len;
	u8 key[32];
} __packed;

struct sta_rec_sec {
	__le16 tag;
	__le16 len;
	u8 add;
	u8 n_cipher;
	u8 rsv[2];

	struct sec_key key[2];
} __packed;

enum mt7921_cipher_type {
	MT_CIPHER_NONE,
	MT_CIPHER_WEP40,
	MT_CIPHER_WEP104,
	MT_CIPHER_WEP128,
	MT_CIPHER_TKIP,
	MT_CIPHER_AES_CCMP,
	MT_CIPHER_CCMP_256,
	MT_CIPHER_GCMP,
	MT_CIPHER_GCMP_256,
	MT_CIPHER_WAPI,
	MT_CIPHER_BIP_CMAC_128,
};

enum {
	CH_SWITCH_NORMAL = 0,
	CH_SWITCH_SCAN = 3,
	CH_SWITCH_MCC = 4,
	CH_SWITCH_DFS = 5,
	CH_SWITCH_BACKGROUND_SCAN_START = 6,
	CH_SWITCH_BACKGROUND_SCAN_RUNNING = 7,
	CH_SWITCH_BACKGROUND_SCAN_STOP = 8,
	CH_SWITCH_SCAN_BYPASS_DPD = 9
};

enum {
	THERMAL_SENSOR_TEMP_QUERY,
	THERMAL_SENSOR_MANUAL_CTRL,
	THERMAL_SENSOR_INFO_QUERY,
	THERMAL_SENSOR_TASK_CTRL,
};

enum {
	MT_EBF = BIT(0),	/* explicit beamforming */
	MT_IBF = BIT(1)		/* implicit beamforming */
};

#define MT7921_WTBL_UPDATE_MAX_SIZE	(sizeof(struct wtbl_req_hdr) +	\
					 sizeof(struct wtbl_generic) +	\
					 sizeof(struct wtbl_rx) +	\
					 sizeof(struct wtbl_ht) +	\
					 sizeof(struct wtbl_vht) +	\
					 sizeof(struct wtbl_hdr_trans) +\
					 sizeof(struct wtbl_ba) +	\
					 sizeof(struct wtbl_smps))

#define MT7921_STA_UPDATE_MAX_SIZE	(sizeof(struct sta_req_hdr) +	\
					 sizeof(struct sta_rec_basic) +	\
					 sizeof(struct sta_rec_ht) +	\
					 sizeof(struct sta_rec_he) +	\
					 sizeof(struct sta_rec_ba) +	\
					 sizeof(struct sta_rec_vht) +	\
					 sizeof(struct sta_rec_uapsd) + \
					 sizeof(struct sta_rec_amsdu) +	\
					 sizeof(struct tlv) +		\
					 MT7921_WTBL_UPDATE_MAX_SIZE)

#define MT7921_WTBL_UPDATE_BA_SIZE	(sizeof(struct wtbl_req_hdr) +	\
					 sizeof(struct wtbl_ba))

#define STA_CAP_WMM			BIT(0)
#define STA_CAP_SGI_20			BIT(4)
#define STA_CAP_SGI_40			BIT(5)
#define STA_CAP_TX_STBC			BIT(6)
#define STA_CAP_RX_STBC			BIT(7)
#define STA_CAP_VHT_SGI_80		BIT(16)
#define STA_CAP_VHT_SGI_160		BIT(17)
#define STA_CAP_VHT_TX_STBC		BIT(18)
#define STA_CAP_VHT_RX_STBC		BIT(19)
#define STA_CAP_VHT_LDPC		BIT(23)
#define STA_CAP_LDPC			BIT(24)
#define STA_CAP_HT			BIT(26)
#define STA_CAP_VHT			BIT(27)
#define STA_CAP_HE			BIT(28)

struct mt7921_mcu_reg_event {
	__le32 reg;
	__le32 val;
} __packed;

struct mt7921_mcu_scan_ssid {
	__le32 ssid_len;
	u8 ssid[IEEE80211_MAX_SSID_LEN];
} __packed;

struct mt7921_mcu_scan_channel {
	u8 band; /* 1: 2.4GHz
		  * 2: 5.0GHz
		  * Others: Reserved
		  */
	u8 channel_num;
} __packed;

struct mt7921_mcu_scan_match {
	__le32 rssi_th;
	u8 ssid[IEEE80211_MAX_SSID_LEN];
	u8 ssid_len;
	u8 rsv[3];
} __packed;

struct mt7921_hw_scan_req {
	u8 seq_num;
	u8 bss_idx;
	u8 scan_type; /* 0: PASSIVE SCAN
		       * 1: ACTIVE SCAN
		       */
	u8 ssid_type; /* BIT(0) wildcard SSID
		       * BIT(1) P2P wildcard SSID
		       * BIT(2) specified SSID + wildcard SSID
		       * BIT(2) + ssid_type_ext BIT(0) specified SSID only
		       */
	u8 ssids_num;
	u8 probe_req_num; /* Number of probe request for each SSID */
	u8 scan_func; /* BIT(0) Enable random MAC scan
		       * BIT(1) Disable DBDC scan type 1~3.
		       * BIT(2) Use DBDC scan type 3 (dedicated one RF to scan).
		       */
	u8 version; /* 0: Not support fields after ies.
		     * 1: Support fields after ies.
		     */
	struct mt7921_mcu_scan_ssid ssids[4];
	__le16 probe_delay_time;
	__le16 channel_dwell_time; /* channel Dwell interval */
	__le16 timeout_value;
	u8 channel_type; /* 0: Full channels
			  * 1: Only 2.4GHz channels
			  * 2: Only 5GHz channels
			  * 3: P2P social channel only (channel #1, #6 and #11)
			  * 4: Specified channels
			  * Others: Reserved
			  */
	u8 channels_num; /* valid when channel_type is 4 */
	/* valid when channels_num is set */
	struct mt7921_mcu_scan_channel channels[32];
	__le16 ies_len;
	u8 ies[MT7921_SCAN_IE_LEN];
	/* following fields are valid if version > 0 */
	u8 ext_channels_num;
	u8 ext_ssids_num;
	__le16 channel_min_dwell_time;
	struct mt7921_mcu_scan_channel ext_channels[32];
	struct mt7921_mcu_scan_ssid ext_ssids[6];
	u8 bssid[ETH_ALEN];
	u8 random_mac[ETH_ALEN]; /* valid when BIT(1) in scan_func is set. */
	u8 pad[63];
	u8 ssid_type_ext;
} __packed;

#define SCAN_DONE_EVENT_MAX_CHANNEL_NUM	64
struct mt7921_hw_scan_done {
	u8 seq_num;
	u8 sparse_channel_num;
	struct mt7921_mcu_scan_channel sparse_channel;
	u8 complete_channel_num;
	u8 current_state;
	u8 version;
	u8 pad;
	__le32 beacon_scan_num;
	u8 pno_enabled;
	u8 pad2[3];
	u8 sparse_channel_valid_num;
	u8 pad3[3];
	u8 channel_num[SCAN_DONE_EVENT_MAX_CHANNEL_NUM];
	/* idle format for channel_idle_time
	 * 0: first bytes: idle time(ms) 2nd byte: dwell time(ms)
	 * 1: first bytes: idle time(8ms) 2nd byte: dwell time(8ms)
	 * 2: dwell time (16us)
	 */
	__le16 channel_idle_time[SCAN_DONE_EVENT_MAX_CHANNEL_NUM];
	/* beacon and probe response count */
	u8 beacon_probe_num[SCAN_DONE_EVENT_MAX_CHANNEL_NUM];
	u8 mdrdy_count[SCAN_DONE_EVENT_MAX_CHANNEL_NUM];
	__le32 beacon_2g_num;
	__le32 beacon_5g_num;
} __packed;

struct mt7921_sched_scan_req {
	u8 version;
	u8 seq_num;
	u8 stop_on_match;
	u8 ssids_num;
	u8 match_num;
	u8 pad;
	__le16 ie_len;
	struct mt7921_mcu_scan_ssid ssids[MT7921_MAX_SCHED_SCAN_SSID];
	struct mt7921_mcu_scan_match match[MT7921_MAX_SCAN_MATCH];
	u8 channel_type;
	u8 channels_num;
	u8 intervals_num;
	u8 scan_func;
	struct mt7921_mcu_scan_channel channels[64];
	__le16 intervals[MT7921_MAX_SCHED_SCAN_INTERVAL];
	u8 bss_idx;
	u8 pad2[64];
} __packed;

struct mt7921_mcu_tx_config {
	u8 peer_addr[ETH_ALEN];
	u8 sw;
	u8 dis_rx_hdr_tran;

	u8 aad_om;
	u8 pfmu_idx;
	__le16 partial_aid;

	u8 ibf;
	u8 ebf;
	u8 is_ht;
	u8 is_vht;

	u8 mesh;
	u8 baf_en;
	u8 cf_ack;
	u8 rdg_ba;

	u8 rdg;
	u8 pm;
	u8 rts;
	u8 smps;

	u8 txop_ps;
	u8 not_update_ipsm;
	u8 skip_tx;
	u8 ldpc;

	u8 qos;
	u8 from_ds;
	u8 to_ds;
	u8 dyn_bw;

	u8 amdsu_cross_lg;
	u8 check_per;
	u8 gid_63;
	u8 he;

	u8 vht_ibf;
	u8 vht_ebf;
	u8 vht_ldpc;
	u8 he_ldpc;
} __packed;

struct mt7921_mcu_sec_config {
	u8 wpi_flag;
	u8 rv;
	u8 ikv;
	u8 rkv;

	u8 rcid;
	u8 rca1;
	u8 rca2;
	u8 even_pn;

	u8 key_id;
	u8 muar_idx;
	u8 cipher_suit;
	u8 rsv[1];
} __packed;

struct mt7921_mcu_key_config {
	u8 key[32];
} __packed;

struct mt7921_mcu_rate_info {
	u8 mpdu_fail;
	u8 mpdu_tx;
	u8 rate_idx;
	u8 rsv[1];
	__le16 rate[8];
} __packed;

struct mt7921_mcu_ba_config {
	u8 ba_en;
	u8 rsv[3];
	__le32 ba_winsize;
} __packed;

struct mt7921_mcu_ant_id_config {
	u8 ant_id[4];
} __packed;

struct mt7921_mcu_peer_cap {
	struct mt7921_mcu_ant_id_config ant_id_config;

	u8 power_offset;
	u8 bw_selector;
	u8 change_bw_rate_n;
	u8 bw;
	u8 spe_idx;

	u8 g2;
	u8 g4;
	u8 g8;
	u8 g16;

	u8 mmss;
	u8 ampdu_factor;
	u8 rsv[1];
} __packed;

struct mt7921_mcu_rx_cnt {
	u8 rx_rcpi[4];
	u8 rx_cc[4];
	u8 rx_cc_sel;
	u8 ce_rmsd;
	u8 rsv[2];
} __packed;

struct mt7921_mcu_tx_cnt {
	__le16 rate1_cnt;
	__le16 rate1_fail_cnt;
	__le16 rate2_cnt;
	__le16 rate3_cnt;
	__le16 cur_bw_tx_cnt;
	__le16 cur_bw_tx_fail_cnt;
	__le16 other_bw_tx_cnt;
	__le16 other_bw_tx_fail_cnt;
} __packed;

struct mt7921_mcu_wlan_info_event {
	struct mt7921_mcu_tx_config tx_config;
	struct mt7921_mcu_sec_config sec_config;
	struct mt7921_mcu_key_config key_config;
	struct mt7921_mcu_rate_info rate_info;
	struct mt7921_mcu_ba_config ba_config;
	struct mt7921_mcu_peer_cap peer_cap;
	struct mt7921_mcu_rx_cnt rx_cnt;
	struct mt7921_mcu_tx_cnt tx_cnt;
} __packed;

struct mt7921_mcu_wlan_info {
	__le32 wlan_idx;
	struct mt7921_mcu_wlan_info_event event;
} __packed;
#endif