#include <stdio.h>
#include <stdlib.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>

struct message {
  struct nlmsghdr header;
  struct ifinfomsg ifinfo;
};

int main() {
  int rtnetlink_socket = socket(AF_NETLINK, SOCK_RAW|SOCK_CLOEXEC, NETLINK_ROUTE);

  // リクエスト送信
  struct message request = {
    .header.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
    .header.nlmsg_type = RTM_GETLINK,
    .header.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP,
    .header.nlmsg_seq = 0,
    .header.nlmsg_pid = 0,
    .ifinfo.ifi_family = AF_PACKET,
  };

  send(rtnetlink_socket, &request, sizeof(request), 0);

  // データ受信
  char buf[32768];
  struct iovec iov = {
    .iov_base = &buf,
    .iov_len = 32768
  };
  struct sockaddr_nl nladdr = {
    .nl_family = AF_NETLINK
  };
  struct msghdr msg = {
    .msg_name = &nladdr,
    .msg_namelen = sizeof(nladdr),
    .msg_iov = &iov,
    .msg_iovlen = 1,
  };

  recvmsg(rtnetlink_socket, &msg, 0);

  // 出力
  struct message *answer = (struct message *)iov.iov_base;
  while(answer->header.nlmsg_len) {
    struct nlattr *attr = (struct nlattr *)((char *)answer + sizeof(struct message));
    for (unsigned short readed = sizeof(struct message); readed < answer->header.nlmsg_len;) {
      printf("readed: %d\n", readed);
      // インターフェース名の出力 
      if (attr->nla_type == IFLA_IFNAME) {
        char *ifname = (char *)attr + sizeof(struct nlattr);
	printf("%s\n", ifname);
	break;
      }

      readed += attr->nla_len;
      attr = (struct nlattr *)((char *)attr + attr->nla_len);
    }

    iov.iov_base += answer->header.nlmsg_len;
    answer = (struct message *)iov.iov_base;
  }
}
