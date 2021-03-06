#include "netsock.h"
#include "netsock-utils.h"
#include "netsock-list.h"
#include "netsock-epoll.h"

struct netsock_registered_class {
    struct list_head class_node;
    struct netsock_class *class;
};

static struct list_head registered_classes;
static bool netsock_exit = false;

static struct netsock_registered_class *
netsock_lookup_class(char *type)
{
    struct netsock_registered_class *rc = NULL;

    list_for_each_entry (rc, &registered_classes, class_node) {
        if (!strcmp(rc->class->type, type)) {
            return rc;
        }
    }

    return NULL;
}

int
netsock_class_register(struct netsock_class *new_class)
{
    int ret = NETSOCK_EOK;

    if (netsock_lookup_class(new_class->type)) {
        printf("duplicate register netsock class, %s\n", new_class->type);
        ret = -NETSOCK_EEXIST;
        goto out;
    }

    ret = new_class->init() ? new_class->init() : NETSOCK_EOK;
    if (ret) {
        printf("new class init error, errno %d\n", ret);
        goto out;
    }

    struct netsock_registered_class *rc;
    rc = malloc(sizeof(*rc));
    if (!rc) {
        printf("malloc error\n");
        ret = -NETSOCK_ESYSCALL;
        goto out;
    }
    list_add_tail(&(rc->class_node), &registered_classes);
    rc->class = new_class;

out:
    return ret;
}

static void *
netsock_loop(void *arg)
{
    struct netsock *netsock_ = (struct netsock *) arg;

    while (!netsock_exit) {
        netsock_epoll_process(netsock_->epollfd, netsock_->events, MAX_EPOLL_EVENTS, NETSOCK_EPOLL_TIMEOUT);
    }
    return NULL;
}

int
netsock_initialize(void)
{
    int ret = NETSOCK_EOK;

    INIT_LIST_HEAD(&registered_classes);

    return ret;
}

int
netsock_open(char *name, int conn_type, char *path, char *class_type, struct netsock **netsockp)
{
    int ret = NETSOCK_EOK;
    struct netsock_registered_class *rc = NULL;
    struct netsock *netsock_ = NULL;

    rc = netsock_lookup_class(class_type);
    if (!rc) {
        printf("unknown sock class type %s\n", class_type);
        ret = -NETSOCK_ENOEXIST;
        goto err1;
    }
    
    netsock_ = rc->class->alloc();
    if (!netsock_) {
        printf("netsock_ alloc error\n");
        ret = -NETSOCK_ESYSCALL;
        goto err1;
    }

    memset(netsock_, 0, sizeof(struct netsock));
    snprintf(netsock_->name, sizeof(netsock_->name), "%s", name);
    netsock_->conn_type = conn_type;
    snprintf(netsock_->path, sizeof(netsock_->path), "%s", path);
    netsock_->epollfd = netsock_epoll_init();
    netsock_->class = rc->class;
    INIT_LIST_HEAD(&(netsock_->conn_list));

    ret = rc->class->construct(netsock_);
    if (ret) {
        printf("netsock construct error, name %s", name);
        ret = -NETSOCK_ECALL;
        goto err2;
    }

    ret = netsock_pthread_create(&(netsock_->thread_id), "netsock_events_thread", netsock_loop, (void *)netsock_);
    if (ret < 0) {
        printf("netsock_pthread_create failed, errno %d\n", errno);
        goto err2;
    }

    *netsockp = netsock_;

    return ret;

err2:
    rc->class->dealloc(netsock_);
err1:
    return ret;
}

int
netsock_close(struct netsock *netsock_)
{
    int ret = NETSOCK_EOK;

    netsock_exit = true;

    ret = netsock_pthread_join(netsock_->thread_id, NULL);
    if (ret < 0) {
        printf("pthread join failed, errno %d\n", errno);
    }

    ret = netsock_->class->destruct(netsock_);
    if (ret < 0) {
        printf("netsock destruct failed, errno %d\n", errno);
    }

    ret = netsock_->class->dealloc(netsock_);
    if (ret < 0) {
        printf("netsock dealloc failed, errno %d\n", errno);
    }

    return ret;
}