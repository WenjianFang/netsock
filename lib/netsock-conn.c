#include "netsock-conn.h"
#include "netsock-epoll.h"
#include "netsock-utils.h"
#include "netsock.h"

int
netsock_conn_construct(int fd, int (*handler)(void *ctx), void *context)
{
    int ret = NETSOCK_EOK;
    struct netsock_conn *new_conn = malloc(sizeof(struct netsock_conn));
    struct netsock *netsock = (struct netsock *) context;

    if (new_conn == NULL) {
        printf("netsock_conn_construct: malloc error, errno %d\n", errno);
        ret = -NETSOCK_ESYSCALL;
        goto out;
    }

    new_conn->fd = fd;
    new_conn->context = context;
    new_conn->handler = handler;
    list_add_tail(&(new_conn->conn_node), &(netsock->conn_list));

    ret = netsock_epoll_ops(netsock->epollfd, EPOLL_CTL_ADD, fd, new_conn);
    if (ret < 0) {
        printf("add new connection to epoll failed, errno %d\n", ret);
        goto out;
    }

out:
    return ret;
}

int
netsock_conn_destruct(int fd, int epollfd, struct list_head *list)
{
    int ret = 0;
    struct netsock_conn *conn = NULL;
    struct netsock_conn *next = NULL;

    list_for_each_entry_safe(conn, next, list, conn_node) {
        if (conn->fd == fd) {
            ret = netsock_epoll_ops(epollfd, EPOLL_CTL_DEL, fd, conn);
            if (ret < 0) {
                printf("del connection from epoll failed %d\n", ret);
                ret = -1;
                goto out;
            }
            list_del(&conn->conn_node);
            break;
        }
    }

out:
    return ret;
}



