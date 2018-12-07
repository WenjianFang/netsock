#include "netsock.h"
#include "netsock-unix-sock.h"
#include "netsock-conn.h"
#include "netsock-utils.h"

#include <stdio.h>

int
main(int argc, char *argv[])
{
    int ret = NETSOCK_EOK;
    struct netsock *netsock = NULL;
    struct netsock_conn *srv_conn = NULL;
    char msg[NETSOCK_UNIX_SOCK_MSG_BUF_LEN] = {0};

    ret = netsock_initialize();
    if (ret < 0) {
        printf("netsock initialize failed, errno %d\n", ret);
        goto err;
    }

    ret = netsock_unix_sock_register();
    if (ret < 0) {
        printf("netsock unix sock register failed, errno %d", ret);
        goto err;
    }

    ret =  netsock_open("unix-sock-client", NETSOCK_CONN_TYPE_CLIENT, "/tmp/netsock/unix_server.sock", "unix_sock", &netsock);
    if (ret < 0) {
        printf("netsock open failed, errno %d\n", ret);
        goto err;
    }

    for (;;) {
        scanf("%s", msg);
        printf("input: %s\n", msg);
        if (!strcmp(msg, "exit")) {
            printf("netsock exit\n");
            break;
        }

        list_for_each_entry (srv_conn, &netsock->conn_list, conn_node) {
            if (srv_conn->fd > 0) {
                break;
            }
        }
        ret = netsock->class->send(srv_conn->fd, netsock, (const void *)msg, sizeof(msg));
        printf("send %d bytes, msg is %s\n", ret, msg);
    }

    ret = netsock_close(netsock);
    if (ret < 0) {
        printf("netsock close failed, errno %d\n", ret);
        goto err;
    }

err:
    return ret;
}