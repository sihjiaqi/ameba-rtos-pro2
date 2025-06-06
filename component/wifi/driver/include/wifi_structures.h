/******************************************************************************
 * Copyright (c) 2013-2016 Realtek Semiconductor Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
  *******************************************************************************/

/**
  ******************************************************************************
  * @file    wifi_structures.h
  * @author
  * @version
  * @brief   This file provides the data structures used for wlan API.
  ******************************************************************************
  */
//
#ifndef _WIFI_STRUCTURES_H
#define _WIFI_STRUCTURES_H

/** @addtogroup WIFI
 *  @brief      WIFI module
 *  @{
 */

//#include <freertos/freertos_service.h>
#include "wifi_constants.h"
#include "dlist.h"
#ifdef	__cplusplus
extern "C" {
#endif

#if defined(__IAR_SYSTEMS_ICC__) || defined (__GNUC__) || defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
#pragma pack(1)
#endif

/** @addtogroup WIFI_Exported_Types WIFI Exported Types
  * @{
  */


/** @defgroup Structs
   *@{
   */
/**
  * @brief  The structure is used to describe the SSID.
  */
typedef struct {
	unsigned char len;     /**< SSID length */
	unsigned char val[33]; /**< SSID name (AP name)  */
} rtw_ssid_t;
#if defined(__IAR_SYSTEMS_ICC__) || defined (__GNUC__) || defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
#pragma pack()
#endif

#if defined(__IAR_SYSTEMS_ICC__) || defined (__GNUC__) || defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
#pragma pack(1)
#endif

/**
  * @brief  The structure is used to describe the unique 6-byte MAC address.
  */
typedef struct {
	unsigned char octet[6]; /**< Unique 6-byte MAC address */
} rtw_mac_t;
#if defined(__IAR_SYSTEMS_ICC__) || defined (__GNUC__) || defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
#pragma pack()
#endif

/**
  * @brief  The structure is used to describe the setting about SSID,
  *			security type, password and default channel, used to start AP mode.
  * @note  The data length of string pointed by ssid should not exceed 32,
  *        and the data length of string pointed by password should not exceed 64.
  */
typedef struct {
	rtw_ssid_t 			ssid;
	unsigned char		hidden_ssid;
	rtw_security_t		security_type;
	unsigned char 		*password;
	unsigned char 		password_len;
	unsigned char		channel;
} rtw_softap_info_t;

typedef struct customized_chl_cfg {
	unsigned char
	chnl_index;//for 2.4G:{1,2,3,4,5,6,7,8,9,10,11,12,13,14}. for 5G:{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140,144,149,153,157,161,165}
	unsigned char scan_type; //1:SCAN_PASSIVE. 2:SCAN_ACTIVE. 3:SCAN_MIX
} customized_chl_cfg_t;

typedef void (*ap_channel_switch_callback_t)(unsigned char channel, rtw_channel_switch_res_t ret);

typedef void (*rtw_joinstatus_callback_t)(\
		rtw_join_status_t join_status);

/**
  * @brief  The structure is used to describe the connection setting about SSID,
  *			security type and password,etc., used when connecting to an AP.
  * @note  The data length of string pointed by ssid should not exceed 32,
  *        and the data length of string pointed by password should not exceed 64.
  * @note  If this struct is used for wifi connect, the channel is used to config
  *        whether it is a full channel scan(when channel is set to 0), or it will
  *        only scan one channel(do active scan on the configured channel).
  * @note  pscan_option set to PSCAN_FAST_SURVEY means do fast survey on the specified channel
  *        set to 0 means do normal scan on the specified channel or full channel.
  */
typedef struct {
	rtw_ssid_t 					ssid;
	rtw_mac_t					bssid;
	rtw_security_t				security_type;
	unsigned char 				*password;
	int 						password_len;
	int							key_id;
	unsigned char				channel;        /**< set to 0 means full channel scan, set to other value means only scan on the specified channel */
	unsigned char				pscan_option;   /**< used when the specified channel is set, set to 0 for normal partial scan, set to PSCAN_FAST_SURVEY for fast survey*/
	unsigned char				roam_en;
	unsigned char				band;			//used for wifi connection to fixed band: 0: 2.4G/5G, 1: 2.4G only, 2: 5G only
	rtw_joinstatus_callback_t	joinstatus_user_callback;   /**< user callback for processing joinstatus, please set to NULL if not use it */
} rtw_network_info_t;

#if defined(__IAR_SYSTEMS_ICC__) || defined(__GNUC__) || defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
#pragma pack(1)
#endif

typedef int (*wifi_do_fast_connect_ptr)(void);
typedef int (*write_fast_connect_info_ptr)(unsigned int data1, unsigned int data2);
typedef void (*p_wlan_autoreconnect_hdl_t)(rtw_security_t, char *, int, char *, int, int);

/**
  * @brief  The structure is used to describe the scan result of the AP.
  */
typedef struct {
	rtw_ssid_t              SSID;             /**< Service Set Identification (i.e. Name of Access Point)                    */
	rtw_mac_t               BSSID;            /**< Basic Service Set Identification (i.e. MAC address of Access Point)       */
	signed short		                  signal_strength;  /**< Receive Signal Strength Indication in dBm. <-90=Very poor, >-30=Excellent */
	rtw_bss_type_t          bss_type;         /**< Network type                                                              */
	rtw_security_t          security;         /**< Security type                                                             */
	rtw_wps_type_t          wps_type;         /**< WPS type                                                                  */
	unsigned int                      channel;          /**< Radio channel that the AP beacon was received on                          */
	rtw_802_11_band_t       band;             /**< Radio band                                                                */
#ifdef CONFIG_P2P_NEW
	unsigned char			p2p_role;
#endif
#ifdef CONFIG_IEEE80211K
	unsigned int			free_cnt;		/** 11k need this information to generate ie**/
#endif
} rtw_scan_result_t;

#if defined(__IAR_SYSTEMS_ICC__) || defined(__GNUC__) || defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
#pragma pack()
#endif

/**
* @brief channel scan time
*/
typedef struct {
	unsigned short active_scan_time;      /*!< active scan time per channel, units: millisecond, default is 100ms */
	unsigned short passive_scan_time;     /*!< passive scan time per channel, units: millisecond, default is 110ms */
	unsigned short home_scan_time;     /*!< home channel scan time, units: millisecond, default is 100ms */
	unsigned char  probe_cnt;
	unsigned char  probe_cnt_interval;
} rtw_channel_scan_time_t;

typedef rtw_result_t (*scan_user_callback_t)(\
		unsigned int ap_num, void *user_data);

typedef rtw_result_t (*scan_report_each_mode_user_callback_t)(\
		rtw_scan_result_t *scanned_ap_info, void *user_data);

/**
  * @brief  The structure is used to describe the scan parameters used for scan,
  * @note  The data length of string pointed by ssid should not exceed 32,
  *        and the data length of string pointed by password should not exceed 64.
  */
typedef struct {
	rtw_scan_option_t						options;
	char									*ssid;
	unsigned char							*channel_list;
	unsigned char							channel_list_num;
	rtw_channel_scan_time_t 				chan_scan_time;
	unsigned short							max_ap_record_num;     /**< config the max number of recorded AP, when set to 0, use default value 64 */
	void									*scan_user_data;
	scan_user_callback_t					scan_user_callback;   /**< used for normal asynchronized mode */
	scan_report_each_mode_user_callback_t	scan_report_each_mode_user_callback; /*used for RTW_SCAN_REPORT_EACH mode */
	unsigned char							scan_user_setting;	/* used for wifi scan pecific setting */
	char									band;				/** 0: => dual band, band: 1 => 2.4G, 2 => 5G**/
	char									mutiple_ssid[2][33];
	unsigned char							mutiple_ssid_num;
} rtw_scan_param_t;

#if defined(__IAR_SYSTEMS_ICC__) || defined(__GNUC__) || defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
#pragma pack(1)
#endif


/**
  * @brief  The structure is used to store the WIFI setting gotten from WIFI driver.
  */
typedef struct {
	rtw_mode_t			mode;   /**< the mode of current wlan interface */
	unsigned char 		ssid[33];   /**< the ssid of connected AP or softAP */
	unsigned char		bssid[6];   /**< the bssid of connected AP or softAP */
	unsigned char		channel;
	rtw_security_t		security_type;   /**< the security type of connected AP or softAP */
	unsigned char 		password[RTW_MAX_PSK_LEN + 1]; /**< the password of connected AP or softAP */
	unsigned char		key_idx;
} rtw_wifi_setting_t;
#if defined(__IAR_SYSTEMS_ICC__) || defined(__GNUC__) || defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
#pragma pack()
#endif

/**
  * @brief  The structure is used to describe the setting when configure the network.
  */
typedef struct {
	unsigned int		boot_mode;
	unsigned char 		ssid[32];
	unsigned char		ssid_len;
	unsigned char		security_type;
	unsigned char		password[RTW_MAX_PSK_LEN + 1];
	unsigned char		password_len;
	unsigned char		channel;
} rtw_wifi_config_t;

/**
  * @brief  The structure is used to describe the maclist.
  */
typedef struct {
	unsigned int    count;         /**< Number of MAC addresses in the list    */
	rtw_mac_t mac_list[1];   /**< Variable length array of MAC addresses */
} rtw_maclist_t;

/**
  * @brief  The structure is used to describe the bss info of the network.\n
  *			It include the version, BSSID, beacon_period, capability, SSID,
  *			channel, atm_window, dtim_period, RSSI e.g.
  */
typedef struct {
	unsigned int      version;                ///< version field
	unsigned int      length;                 ///< byte length of data in this record
	/* starting at version and including IEs */
	rtw_mac_t     BSSID;
	unsigned short      beacon_period;          ///< units are Kusec
	unsigned short      capability;             ///< Capability information
	unsigned char       SSID_len;
	unsigned char       SSID[32];
	unsigned char       channel;
	unsigned short      atim_window;            ///< units are Kusec
	unsigned char       dtim_period;            ///< DTIM period
	signed short       RSSI;                   ///< receive signal strength (in dBm)

	unsigned char       n_cap;                  ///< BSS is 802.11N Capable
	unsigned int      nbss_cap;               ///< 802.11N BSS Capabilities (based on HT_CAP_*)
	unsigned char       basic_mcs[MCSSET_LEN];  ///< 802.11N BSS required MCS set

	unsigned short      ie_offset;              ///< offset at which IEs start, from beginning
	unsigned int      ie_length;             ///< byte length of Information Elements
} rtw_bss_info_t;

/**
  * @brief  The structure is used to set WIFI packet filter pattern.
  */
typedef struct {
	unsigned short	offset;     ///< Offset in bytes to start filtering (referenced to the start of the ethernet packet)
	unsigned short	mask_size;  ///< Size of the mask in bytes
	unsigned char	*mask;       ///< Pattern mask bytes to be ANDed with the pattern eg. "\xff00" (must be in network byte order)
	unsigned char	*pattern;    ///< Pattern bytes used to filter eg. "\x0800"  (must be in network byte order)
} rtw_packet_filter_pattern_t;

/**
  * @brief  The structure is used to describe the 802.11 frame info
  */
typedef struct {
	unsigned short i_fc;
	unsigned short i_dur;
	unsigned char i_addr1[6];
	unsigned char i_addr2[6];
	unsigned char i_addr3[6];
	unsigned short i_seq;
	unsigned char bssid[6];
	unsigned char encrypt;
	signed char rssi;
#if (defined(CONFIG_UNSUPPORT_PLCPHDR_RPT) && CONFIG_UNSUPPORT_PLCPHDR_RPT) || defined __DOXYGEN__
	rtw_rx_type_t type;
#endif
} ieee80211_frame_info_t;

#if (defined(CONFIG_UNSUPPORT_PLCPHDR_RPT) && CONFIG_UNSUPPORT_PLCPHDR_RPT) || defined __DOXYGEN__
/**
  * @brief  The structure is used to describe the plcp header
  */
typedef struct {
	unsigned short length;	///< length without FCS
	unsigned char filter;		///< 1: HT-20 2T and not LDPC pkt; 2: HT-40 2T and not LDPC pkt; 3: LDPC pkt
	signed char rssi;	///< -128~-1
	unsigned short channel;	///< channel whick this pkt in
unsigned char agg:
	1;		///< aggregation pkt or not. If an AMPDU contains only one MPDU then above 'length' is the antual pkt length without FCS, buuut if it contains multiple MPDUs then above 'length' is useless because it cannot tell how many MPDUs are contained and how long is each MPDU.
	unsigned char mcs: 7;		///< mcs index
} rtw_rx_info_t;

/**
  * @brief  The structure is used to describe the plcp info
  */
struct rtw_plcp_info {
	struct rtw_plcp_info *prev;
	struct rtw_plcp_info *next;
	rtw_rx_info_t rtw_plcp_info;
};

/**
  * @brief  The structure is used to describe therx buffer
  */
struct rtw_rx_buffer {
	struct rtw_plcp_info *head; ///< head
	struct rtw_plcp_info *tail;   ///< tail
};
#endif

/**
  * @brief  The structure is used to describe the packet filter info
  */
typedef struct {
	char filter_id;
	rtw_packet_filter_pattern_t patt;
	rtw_packet_filter_rule_t rule;
	unsigned char enable;
} rtw_packet_filter_info_t;

/**
  * @brief  The structure is used to describe the mac filter list
  */
typedef struct {
	struct list_head node;
	unsigned char mac_addr[6];
} rtw_mac_filter_list_t;

#if defined CONFIG_RTL8735B || defined __DOXYGEN__
/**
  * @brief  The structure is used to describe the wowlan pattern
  */
typedef struct {
	unsigned char eth_da[6];
	unsigned char eth_sa[6];
	unsigned char eth_proto_type[2];
	unsigned char header_len[1];
	unsigned char ip_proto[1];
	unsigned char ip_sa[4];
	unsigned char ip_da[4];
	unsigned char src_port[2];
	unsigned char dest_port[2];
	unsigned char flag2[1];
	unsigned char mask[6];
	unsigned char window[2];
	unsigned char checksum[2];
	unsigned char urgent_pointer[2];
	unsigned char payload[64];
	unsigned char payload_mask[9];
} wowlan_pattern_t;
#else

/**
  * @brief  The structure is used to describe the wowlan pattern
  */
typedef struct {
	unsigned char eth_da[6];
	unsigned char eth_sa[6];
	unsigned char eth_proto_type[2];
	unsigned char header_len[1];
	//unsigned char header_content[8];
	unsigned char ip_proto[1];
	//unsigned char checksum[2];
	unsigned char ip_sa[4];
	unsigned char ip_da[4];
	unsigned char src_port[2];
	unsigned char dest_port[2];
	unsigned char mask[5];
} wowlan_pattern_t;
#endif

/**
  * @brief  The structure is used to describe the psk info
  */
struct psk_info {
	unsigned char index;                  ///<  index
	unsigned char psk_essid[32 + 4]; ///< refer to NDIS_802_11_LENGTH_SSID + 4
	unsigned char psk_passphrase[RTW_MAX_PSK_LEN + 1]; ///< refer to IW_PASSPHRASE_MAX_SIZE + 1
	unsigned char wpa_global_PSK[20 * 2]; ///< refer to A_SHA_DIGEST_LEN * 2
};

/**
  * @brief  The structure is used to describe the sw statistics
  */
typedef struct { /* software statistics for tx and rx*/
	unsigned long   rx_packets;             /*!< total packets received       */
	unsigned long   tx_packets;             /*!<total packets transmitted    */
	unsigned long   rx_dropped;             /*!< no space in buffers    */
	unsigned long   tx_dropped;             /*!< no space available  */
	unsigned long   rx_bytes;               /*!< total bytes received         */
	unsigned long   tx_bytes;               /*!< total bytes transmitted      */
	unsigned long   rx_overflow;            /*!< rx fifo overflow count       */
	unsigned int    max_skbbuf_used_number; /*!< max skb buffer used number       */
	unsigned int    skbbuf_used_number;     /*!< current used skbbuf number       */
	unsigned int    max_skbdata_used_number;/*!< max skb data used number       */
	unsigned int    skbdata_used_number;    /*!< current used skbdata number       */
} rtw_sw_statistics_t;

/**
  * @brief  The structure is used to describe the phy statistics
  */
typedef struct {
	signed char rssi;          /*!<average rssi in 1 sec */
	signed char snr;          /*!< average snr in 1 sec (not include cck rate)*/
	/* todo*/
	unsigned int false_alarm_cck;
	unsigned int false_alarm_ofdm;
	unsigned int cca_cck;
	unsigned int cca_ofdm;
	unsigned int tx_total;
	unsigned int tx_retry;
	unsigned short tx_ok;
	unsigned short tx_drop;
	unsigned int rx_drop;
} rtw_phy_statistics_t;

/**
  * @brief  The structure is used to describe the data description
  */
typedef struct {
	unsigned char wlan_idx;      /**< index of wlan interface which will transmit */
	unsigned char *buf;          /**< poninter of buf where raw data is stored*/
	unsigned short buf_len;      /**< the length of raw data*/
	unsigned short flags;        /**< send options*/
	unsigned char tx_rate;       /**< specific tx rate, please refer to enum MGN_RATE in wifi_constants.h*/
	unsigned char retry_limit;   /**< retry limit configure, when set to 0, will use default retry limit 12*/
	/* todo*/
	unsigned int tx_power;
} raw_data_desc_t;

/**
  * @brief  The structure is used to describe the wifi user configuration
  */
struct  wifi_user_conf {
	unsigned char rtw_adaptivity_en;
	unsigned char rtw_adaptivity_mode;
	unsigned char rtw_adaptivity_th_l2h_ini;

	unsigned char rtw_tx_pwr_lmt_enable;	///< 0: disable, 1: enable, 2: Depend on efuse(flash)
	unsigned char rtw_tx_pwr_by_rate;	///< 0: disable, 1: enable, 2: Depend on efuse(flash)
	unsigned char rtw_trp_tis_cert_en;

	unsigned char rtw_powersave_en;

	unsigned char rtw_cmd_tsk_spt_wap3;
	unsigned char rtw_ignore_security;

	unsigned char g_user_ap_sta_num;

	/* power save */
	unsigned char lps_dtim;
	unsigned char lps_enter_threshold;
	unsigned char rtw_power_mgnt;
	unsigned char rtw_lps_level;
	unsigned char rtw_ps_timeout;
	unsigned char smart_ps;

	/* AP */
	unsigned char bForwardingDisabled;

	unsigned char bChDeauthDisabled;

	unsigned char bAcceptAddbaReq;
	unsigned char bIssueAddbaReq;	///< 0: disable issue addba request, 1: enable issue addba request
	unsigned char addba_ampdu_size;
	unsigned char addba_tx_num_th;	// issue ADDBA TX number threshold, 0: issue addba request after connection

	unsigned char bCheckDestAddress; ///< 0: don't check dest mac and ip address for station, 1: check dest mac and ip address for station

	/*
	The ap_compatibilty_enabled is used to configure the wlan settings, each bit controls one aspect.
	bit 0: (0: follow 802.11 spec, do not issue deauth, 1(default): issue deauth in 1st REAUTH_TO to be compatible with ap)
	bit 1: (0: do not check beacon info to connect with AP with multiple SSID, 1(default): check beacon info)
	bit 2: (0(default): do not issue deauth at start of auth, 1: issue deauth at start of auth)
	bit 3: (0: do not switch WEP auth algo unless WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG, 1(default): switch WEP auth algo from shared key to open system in 1st REAUTH_TO)
	other bits: reserved
	bit 4: (0: enable reordering immediately upon receiving ADDBA REQ, 1(default): enable reordering after REORDER_WAIT_TIME to fix issue that some data packets with "invalid" seqnum are sent between ADDBA REQ and ADDBA RSP)
	bit 5: (0(default): disable issue encryption wpa3 deauth, 1: enable issue encryption wpa3 deauth)
	bit 6: (0(default): disable gcmp128/gcmp256 wifi connection, 1: enable gcmp128/gcmp256 wifi connection)
	bit 7: (0(default): disable wpa3 transition to force wpa2 security connection, 1: enable wpa3 transition to force wpa2 security connection)
	bit 8: (0(default): disable to check the RSC of group key, 1: enable to check the RSC of group key)
	*/
	unsigned int ap_compatibilty_enabled;

	unsigned char max_roaming_times;

	unsigned char ap_polling_sta;

	unsigned char channel_plan;

	unsigned char country_code;

	unsigned char band_type;	// 0: 2.4g & 5g, 1: 2.4g, 2: 5g

	/*
	The wifi_debug_enabled is used to configure the wlan debug settings, each bit controls one aspect.
	bit 0: (0: disable 4way handshake debug, 1:  enable 4way handshake debug messenge)
	bit 1: (0: disable wifi connection profile info, 1:  enable wifi connection profile info)
	bit 2: (0: show wifi connection state, 1:  show wifi connection state time)
	bit 3: (0: disable wifi connection flow debug, 1:  enable wifi connection flow debug)
	bit 4: (0: disable wifi auto reconnection flow debug, 1:  enable auto reconnection flow debug)
	bit 5: (0: disable to mark wifi SSID/router top three MAC Address, 1:  enable to mark wifi SSID/router top three MAC Address)
	bit 6: (0: don't show wifi monitor environment information, 1:  show wifi monitor environment information)
	*/
	unsigned char wifi_debug_enabled;

	/*
	The active_keepalive_enabled is used to enable active keep alive to send keepalive packet.
	bit 0: (0: disable to send unicast arp request, 1:  enable to send unicast arp request)
	*/
	unsigned char active_keepalive_enabled;

	/*
	The active_keepalive_interval is used to configure active keepalive packet interval(unit: second)
	*/
	unsigned char active_keepalive_interval;

	//provide fast scan retry times for upper layer to revise
	unsigned char fast_pscan_retry_times_max;

	customized_chl_cfg_t chn2_4G[13];
	unsigned char chn2_4G_num;
	customized_chl_cfg_t chn5G[25];
	unsigned char chn5G_num;
	unsigned char regulation2_4G;
	unsigned char regulation5G;
} ;
extern  struct wifi_user_conf wifi_user_config;

struct  wifi_default_conf {
	unsigned short active_scan_timeout;    /*!< active scan time per channel, units: millisecond, default is 100ms */
	unsigned short passive_scan_timeout;   /*!< passive scan time per channel, units: millisecond, default is 110ms */
	unsigned short home_scan_timeout;      /*!< home channel scan time, units: millisecond, default is 100ms */
	unsigned char  scan_probe_cnt;

	// Packet retry limit
	unsigned char short_retry_limit;
	unsigned char long_retry_limit;

	/* RTS/CTS */
	unsigned char rts_cts_en;	// 0: disable, 1: enable, 2: depend on driver
	unsigned int rts_threshold; // default 2437

	/* assoc setting*/
	unsigned short listen_interval;
} ;
extern  struct wifi_default_conf wifi_default_config;

/**
  * @brief  The structure is used to describe the cfg parameters used for csi report,
  * @note  The mac_addr if not specified, the default value must be 0.
  */
typedef struct {
	rtw_csi_group_num group_num;
	rtw_csi_mode mode;
	rtw_csi_action act;
	rtw_csi_accuracy accuracy;
	unsigned char enable;
	unsigned char trig_period;  /* unit:ms*/
	unsigned char data_rate;
	unsigned char mac_addr[6];
} rtw_csi_action_parm_t;

/**
  * @brief  The structure is used to describe the extra info of csi report
  */
typedef struct {
	unsigned char mac_addr[6];  /**< may be sta addr, driver define */
	unsigned char trig_addr[6];  /**< add new,tx addr for trig chan_info */
	unsigned int hw_assigned_timestamp;  /**< rxdesc: u32 r_rx_tsfl */
	unsigned char channel;  /**< driver define */
	unsigned char bandwidth; /**< rxdesc: u8 bw */
	unsigned char rx_data_rate;  /**< rxdesc: u16 rx_rate <ack> */
	unsigned char protocol_mode; /**< add new,ofdm(0)/ht(1)/vht(2)/he(3) */
	unsigned char nc;  /**< ch_rpt_hdr_info */
	unsigned char nr;  /**< ch_rpt_hdr_info */
	unsigned short num_sub_carrier;  /**< cfg param, driver define */
	unsigned char num_bit_per_tone;  /**< cfg param, driver define per I/Q */
	unsigned char avg_idle_noise_pwr;  /**< ch_rpt_hdr_info */
	unsigned char evm[2];  /**< ch_rpt_hdr_info + phy_info_rpt */
	unsigned char rssi[2];  /**< phy_info_rpt */
	unsigned int csi_data_length;  /**< ch_rpt_hdr_info */
	unsigned char rxsc;  /**< phy_info_rpt */
	unsigned char csi_valid;  /**< ch_rpt_hdr_info */
} rtw_csi_header_t;
/** @} */

typedef struct {
	int channel;
	int clm_ratio;
	int nhm_idle_ratio;
	int nhm_tx_ratio;
} rtw_clm_t;

#define WIFI_CONNECT_SCAN_NUM 10
typedef struct {
	int num;
	unsigned char bssid[WIFI_CONNECT_SCAN_NUM][6];
	unsigned char channel[WIFI_CONNECT_SCAN_NUM];
	int rssi[WIFI_CONNECT_SCAN_NUM];
	unsigned int rsn_len[WIFI_CONNECT_SCAN_NUM];
} wifi_connect_scan_list;
#ifdef	__cplusplus
}
#endif

/**
  * @}
  */

/**
  * @}
  */

#endif /* _WIFI_STRUCTURES_H */
