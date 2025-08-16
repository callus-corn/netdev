#include <stdio.h>
#include <stdlib.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>

struct request {
  struct nlmsghdr header;
  struct ifinfomsg ifinfo;
};

int main() {
  int rtnetlink_socket = socket(AF_NETLINK, SOCK_RAW|SOCK_CLOEXEC, NETLINK_ROUTE);

  // リクエスト送信
  struct request req = {
    .header.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
    .header.nlmsg_type = RTM_GETLINK,
    .header.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP,
    .header.nlmsg_seq = 0,
    .header.nlmsg_pid = 0,
    .ifinfo.ifi_family = AF_PACKET,
  };

  send(rtnetlink_socket, &req, sizeof(req), 0);

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

  int n = recvmsg(rtnetlink_socket, &msg, 0);

  // 出力
  for (struct nlmsghdr *answer = (struct nlmsghdr *)msg.msg_iov->iov_base; NLMSG_OK(answer, n); answer = NLMSG_NEXT(answer, n)) {
    int attrlen = answer->nlmsg_len - sizeof(struct request);
    for (struct rtattr *attr = (struct rtattr *)((void *)answer + sizeof(struct request)); RTA_OK(attr, attrlen); attr = RTA_NEXT(attr, attrlen)) {
      if (attr->rta_type == IFLA_IFNAME) {
	printf("%s\n", (char *)RTA_DATA(attr));
      }
    }
  }
}
