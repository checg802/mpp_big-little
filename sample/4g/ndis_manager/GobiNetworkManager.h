#ifndef SIMCOM_NETWORK_MANAGER_HEADER
#define SIMCOM_NETWORK_MANAGER_HEADER

/**********************************IOCTL Request code********************************/
// IOCTL to generate a client ID for this service type
#define IOCTL_QMI_GET_SERVICE_FILE 0x8BE0+1

// IOCTL to get the VIDPID of the device
#define IOCTL_QMI_GET_DEVICE_VIDPID 0x8BE0+2

// IOCTL to get the MEID of the device
#define IOCTL_QMI_GET_DEVICE_MEID 0x8BE0+3

// IOCTL to start network
#define IOCTL_QMI_START_NETWORK 0x8BE0 + 4

// IOCTL to start network
#define IOCTL_QMI_STOP_NETWORK 0x8BE0 + 5

// IOCTL to get network status
#define IOCTL_QMI_GET_NETWORK_STATUS 0x8BE0 + 6

// IOCTL to get net name
#define IOCTL_QMI_GET_NET_INTERFACE_NAME 0x8BE0 + 7
/************************************************************************************/


/************************************State Macro*************************************/
#define    NM_STATE_CONNECTED           0
#define    NM_STATE_CONNECTING          1
#define    NM_STATE_DISCONNECTING       2
#define    NM_STATE_DISCONNECTED        3
/************************************************************************************/


/**********************************Struct Define*************************************/

#define IFNAMSIZ              16
#define SIMCOM_STRING_LENGTH  64

enum eGobiAuthType
  {
    AUTH_PROTOCOL_UNSPECIFIED = -1,
    AUTH_PROTOCOL_NONE,
    AUTH_PROTOCOL_CHAP_ONLY,
    AUTH_PROTOCOL_PAP_ONLY,
    AUTH_PROTOCOL_CHAP_PAP,
    AUTH_PROTOCOL_MAX
  };

enum eGobiIPType
  {
    IP_TYPE_UNSPECIFIED = -1,
    IP_TYPE_IPV4,
    IP_TYPE_IPV6,
    IP_TYPE_MAX
  };

struct sGobiNMParam
{
  char  apn[SIMCOM_STRING_LENGTH];
  char  username[SIMCOM_STRING_LENGTH];
  char  passwd[SIMCOM_STRING_LENGTH];
  enum  eGobiAuthType auth_type;
  enum  eGobiIPType   ip_type;
};

/************************************************************************************/

/*************************************Error Code*************************************/
#define         SIMCOM_SUCCESS                             0
#define         SIMCOM_ERR_NO_EFFECT                       1
#define         SIMCOM_ERR_MALFORMED_MSG                   2
#define         SIMCOM_ERR_NO_MEMORY                       3
#define         SIMCOM_ERR_CALL_FAILED                     4
#define         SIMCOM_ERR_DEVICE_INVALID                  5
#define         SIMCOM_ERR_INTERNEL                        6
#define         SIMCOM_ERR_GET_CLIENT_ID                   7
#define         SIMCOM_ERR_NOT_MATCHED_RESP                8
#define         SIMCOM_ERR_ARG_TOO_LONG                    9
#define         SIMCOM_ERR_INVALID_PROFILE                 10
#define         SIMCOM_ERR_INVALID_TECH_PREF               11
#define         SIMCOM_ERR_INVALID_PDP_TYPE                12
#define         SIMCOM_ERR_ACCESS_DENIED                   13
#define         SIMCOM_ERR_INVALID_IP_FAMILY_PREF          14
#define         SIMCOM_ERR_INVALID_SESSION_ID              15
/************************************************************************************/

#endif /* SIMCOM_NETWORK_MANAGER_HEADER */