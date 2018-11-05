#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include "GobiNetworkManager.h"

#include <unistd.h>


const char* usage_help =
  "\n\n"
  "Options:\n"
  " 1: start network interface.\n"
  " 2: stop network interface.\n"
  " 3: get network status\n"
  " 4: get net interface name\n"
  " 0: Exit.\n"
  "\n\n";


void print_usage(void)
{
    printf("%s", usage_help);
}

int main(int argc, char** argv)
{
    int choice, fd, rc = 0;

    if (argc != 2)
    {
        printf("must have one argument!\n");
        return 0;
    }

    fd = open(argv[1], O_RDWR);

    if (fd < 0)
    {
        printf("fail to open device %s.\n", argv[1]);
        return 0;
    }

    do
    {
        print_usage();
        scanf("%d", &choice);
        switch(choice)
        {
        case 0:
          {
            close(fd);
            printf("Quit!\n");
            return 0;
          }
        case 1:
          {
#if 0
            struct sGobiNMParam gobiNMParam;
            memset(&gobiNMParam, 0, sizeof(struct sGobiNMParam));

            memcpy(gobiNMParam.username, "ztf", 3);
            memcpy(gobiNMParam.passwd, "123456", 6);
            memcpy(gobiNMParam.apn, "apn", 3);
            gobiNMParam.auth_type = AUTH_PROTOCOL_CHAP_PAP;
            gobiNMParam.ip_type = IP_TYPE_IPV6;

            rc = ioctl(fd, IOCTL_QMI_START_NETWORK, &gobiNMParam);
#else
            rc = ioctl(fd, IOCTL_QMI_START_NETWORK, 0);
#endif
            if (rc == 0)
              {
                printf("success to start network interface\n");
              }
            else
              {
                printf("fail to start network interface, errno: %d\n", rc);
              }
            break;
          }
        case 2:
          rc = ioctl(fd, IOCTL_QMI_STOP_NETWORK, 0);
          if (rc == 0)
          {
              printf("success to stop network interface\n");
          }
          else
          {
              printf("fail to stop network interface, errno: %d\n", rc);
          }
          break;
        case 3:
          rc = ioctl(fd, IOCTL_QMI_GET_NETWORK_STATUS, 0);
          switch (rc)
            {
            case NM_STATE_CONNECTED:
              printf("%s\n", "Network Connected!");
              break;
            case NM_STATE_CONNECTING:
              printf("%s\n", "Network Connecting!");
              break;
            case NM_STATE_DISCONNECTED:
              printf("%s\n", "Network Disconnected!");
              break;
            case NM_STATE_DISCONNECTING:
              printf("%s\n", "Network Disconnecting!");
              break;
	    default :
	      printf("%s\n", "rc is wrong value!");
              break;
		
            }
          break;
        case 4:
        {
          char iface_name[IFNAMSIZ];
          rc = ioctl(fd, IOCTL_QMI_GET_NET_INTERFACE_NAME, iface_name);
          if (rc == 0)
            {
              printf("success to get net interface name: %s\n", iface_name);
            }
          else
            {
              printf("fail to get net interface name, error: %d\n", rc);
            }
          break;
        }
        default:
          printf("Invalid choice!\n");
          sleep(1);
        }
    } while (1);
    return 0;
}
