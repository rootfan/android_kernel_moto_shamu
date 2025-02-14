#ifndef _BPF_CGROUP_H
#define _BPF_CGROUP_H

#include <linux/bpf.h>
#include <linux/jump_label.h>
#include <uapi/linux/bpf.h>

struct sock;
struct cgroup;
struct sk_buff;

#ifdef CONFIG_CGROUP_BPF

extern struct static_key cgroup_bpf_enabled_key;
#define cgroup_bpf_enabled static_key_false(&cgroup_bpf_enabled_key)

struct bpf_prog_list {
	struct list_head node;
	struct bpf_prog *prog;
};

struct bpf_prog_array;

struct cgroup_bpf {
	/* array of effective progs in this cgroup */
	struct bpf_prog_array __rcu *effective[MAX_BPF_ATTACH_TYPE];

	/* attached progs to this cgroup and attach flags
	 * when flags == 0 or BPF_F_ALLOW_OVERRIDE the progs list will
	 * have either zero or one element
	 * when BPF_F_ALLOW_MULTI the list can have up to BPF_CGROUP_MAX_PROGS
	 */
	struct list_head progs[MAX_BPF_ATTACH_TYPE];
	u32 flags[MAX_BPF_ATTACH_TYPE];

	/* temp storage for effective prog array used by prog_attach/detach */
	struct bpf_prog_array __rcu *inactive;
};

void cgroup_bpf_put(struct cgroup *cgrp);
int cgroup_bpf_inherit(struct cgroup *cgrp);

int __cgroup_bpf_attach(struct cgroup *cgrp, struct bpf_prog *prog,
			enum bpf_attach_type type, u32 flags);
int __cgroup_bpf_detach(struct cgroup *cgrp, struct bpf_prog *prog,
			enum bpf_attach_type type, u32 flags);

/* Wrapper for __cgroup_bpf_*() protected by cgroup_mutex */
int cgroup_bpf_attach(struct cgroup *cgrp, struct bpf_prog *prog,
		      enum bpf_attach_type type, u32 flags);
int cgroup_bpf_detach(struct cgroup *cgrp, struct bpf_prog *prog,
		      enum bpf_attach_type type, u32 flags);

int __cgroup_bpf_run_filter(struct sock *sk,
			    struct sk_buff *skb,
			    enum bpf_attach_type type);

/* Wrappers for __cgroup_bpf_run_filter() guarded by cgroup_bpf_enabled. */
#define BPF_CGROUP_RUN_PROG_INET_INGRESS(sk,skb)			\
({									\
	int __ret = 0;							\
	if (cgroup_bpf_enabled)						\
		__ret = __cgroup_bpf_run_filter(sk, skb,		\
						BPF_CGROUP_INET_INGRESS); \
									\
	__ret;								\
})

#define BPF_CGROUP_RUN_PROG_INET_EGRESS(sock,skb)				\
({									\
	int __ret = 0;							\
	if (cgroup_bpf_enabled && sock && sock == skb->sk) {		\
		if (sk_fullsock(sock))					\
			__ret = __cgroup_bpf_run_filter(sock, skb,	\
						BPF_CGROUP_INET_EGRESS); \
	}								\
	__ret;								\
})

#else

struct cgroup_bpf {};
static inline void cgroup_bpf_put(struct cgroup *cgrp) {}
static inline int cgroup_bpf_inherit(struct cgroup *cgrp) { return 0; }

#define BPF_CGROUP_RUN_PROG_INET_INGRESS(sk,skb) ({ 0; })
#define BPF_CGROUP_RUN_PROG_INET_EGRESS(sk,skb) ({ 0; })

#endif /* CONFIG_CGROUP_BPF */

#endif /* _BPF_CGROUP_H */
