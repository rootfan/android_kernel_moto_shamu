/* Copyright (c) 2011-2014 PLUMgrid, http://plumgrid.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 */
#ifndef _UAPI__LINUX_BPF_H__
#define _UAPI__LINUX_BPF_H__

#include <linux/types.h>
#include <linux/bpf_common.h>

/* Extended instruction set based on top of classic BPF */

/* instruction classes */
#define BPF_ALU64	0x07	/* alu mode in double word width */

/* ld/ldx fields */
#define BPF_DW		0x18	/* double word */
#define BPF_XADD	0xc0	/* exclusive add */

/* alu/jmp fields */
#define BPF_MOV		0xb0	/* mov reg to reg */
#define BPF_ARSH	0xc0	/* sign extending arithmetic shift right */

/* change endianness of a register */
#define BPF_END		0xd0	/* flags for endianness conversion: */
#define BPF_TO_LE	0x00	/* convert to little-endian */
#define BPF_TO_BE	0x08	/* convert to big-endian */
#define BPF_FROM_LE	BPF_TO_LE
#define BPF_FROM_BE	BPF_TO_BE

#define BPF_JNE		0x50	/* jump != */
#define BPF_JSGT	0x60	/* SGT is signed '>', GT in x86 */
#define BPF_JSGE	0x70	/* SGE is signed '>=', GE in x86 */
#define BPF_CALL	0x80	/* function call */
#define BPF_EXIT	0x90	/* function return */

/* Register numbers */
enum {
	BPF_REG_0 = 0,
	BPF_REG_1,
	BPF_REG_2,
	BPF_REG_3,
	BPF_REG_4,
	BPF_REG_5,
	BPF_REG_6,
	BPF_REG_7,
	BPF_REG_8,
	BPF_REG_9,
	BPF_REG_10,
	__MAX_BPF_REG,
};

/* BPF has 10 general purpose 64-bit registers and stack frame. */
#define MAX_BPF_REG	__MAX_BPF_REG

struct bpf_insn {
	__u8	code;		/* opcode */
	__u8	dst_reg:4;	/* dest register */
	__u8	src_reg:4;	/* source register */
	__s16	off;		/* signed offset */
	__s32	imm;		/* signed immediate constant */
};

/* BPF syscall commands, see bpf(2) man-page for details. */
enum bpf_cmd {
	BPF_MAP_CREATE,
	BPF_MAP_LOOKUP_ELEM,
	BPF_MAP_UPDATE_ELEM,
	BPF_MAP_DELETE_ELEM,
	BPF_MAP_GET_NEXT_KEY,
	BPF_PROG_LOAD,
	BPF_OBJ_PIN,
	BPF_OBJ_GET,
	BPF_PROG_ATTACH,
	BPF_PROG_DETACH,
};

enum bpf_map_type {
	BPF_MAP_TYPE_UNSPEC,
	BPF_MAP_TYPE_HASH,
	BPF_MAP_TYPE_ARRAY,
	BPF_MAP_TYPE_PROG_ARRAY,
	BPF_MAP_TYPE_PERF_EVENT_ARRAY,
	BPF_MAP_TYPE_PERCPU_HASH,
	BPF_MAP_TYPE_PERCPU_ARRAY,
	BPF_MAP_TYPE_STACK_TRACE,
	BPF_MAP_TYPE_CGROUP_ARRAY,
};

enum bpf_prog_type {
	BPF_PROG_TYPE_UNSPEC,
	BPF_PROG_TYPE_SOCKET_FILTER,
	BPF_PROG_TYPE_KPROBE,
	BPF_PROG_TYPE_SCHED_CLS,
	BPF_PROG_TYPE_SCHED_ACT,
	BPF_PROG_TYPE_TRACEPOINT,
	BPF_PROG_TYPE_XDP,
	BPF_PROG_TYPE_PERF_EVENT,
	BPF_PROG_TYPE_CGROUP_SKB,
};

enum bpf_attach_type {
	BPF_CGROUP_INET_INGRESS,
	BPF_CGROUP_INET_EGRESS,
	__MAX_BPF_ATTACH_TYPE
};

#define MAX_BPF_ATTACH_TYPE __MAX_BPF_ATTACH_TYPE

/* cgroup-bpf attach flags used in BPF_PROG_ATTACH command
 *
 * NONE(default): No further bpf programs allowed in the subtree.
 *
 * BPF_F_ALLOW_OVERRIDE: If a sub-cgroup installs some bpf program,
 * the program in this cgroup yields to sub-cgroup program.
 *
 * BPF_F_ALLOW_MULTI: If a sub-cgroup installs some bpf program,
 * that cgroup program gets run in addition to the program in this cgroup.
 *
 * Only one program is allowed to be attached to a cgroup with
 * NONE or BPF_F_ALLOW_OVERRIDE flag.
 * Attaching another program on top of NONE or BPF_F_ALLOW_OVERRIDE will
 * release old program and attach the new one. Attach flags has to match.
 *
 * Multiple programs are allowed to be attached to a cgroup with
 * BPF_F_ALLOW_MULTI flag. They are executed in FIFO order
 * (those that were attached first, run first)
 * The programs of sub-cgroup are executed first, then programs of
 * this cgroup and then programs of parent cgroup.
 * When children program makes decision (like picking TCP CA or sock bind)
 * parent program has a chance to override it.
 *
 * A cgroup with MULTI or OVERRIDE flag allows any attach flags in sub-cgroups.
 * A cgroup with NONE doesn't allow any programs in sub-cgroups.
 * Ex1:
 * cgrp1 (MULTI progs A, B) ->
 *    cgrp2 (OVERRIDE prog C) ->
 *      cgrp3 (MULTI prog D) ->
 *        cgrp4 (OVERRIDE prog E) ->
 *          cgrp5 (NONE prog F)
 * the event in cgrp5 triggers execution of F,D,A,B in that order.
 * if prog F is detached, the execution is E,D,A,B
 * if prog F and D are detached, the execution is E,A,B
 * if prog F, E and D are detached, the execution is C,A,B
 *
 * All eligible programs are executed regardless of return code from
 * earlier programs.
 */
#define BPF_F_ALLOW_OVERRIDE	(1U << 0)
#define BPF_F_ALLOW_MULTI	(1U << 1)

#define BPF_PSEUDO_MAP_FD	1

/* flags for BPF_MAP_UPDATE_ELEM command */
#define BPF_ANY		0 /* create new element or update existing */
#define BPF_NOEXIST	1 /* create new element if it didn't exist */
#define BPF_EXIST	2 /* update existing element */

#define BPF_F_NO_PREALLOC	(1U << 0)

/* Flags for accessing BPF object */
#define BPF_F_RDONLY		(1U << 3)
#define BPF_F_WRONLY		(1U << 4)

union bpf_attr {
	struct { /* anonymous struct used by BPF_MAP_CREATE command */
		__u32	map_type;	/* one of enum bpf_map_type */
		__u32	key_size;	/* size of key in bytes */
		__u32	value_size;	/* size of value in bytes */
		__u32	max_entries;	/* max number of entries in a map */
		__u32	map_flags;	/* prealloc or not */
	};

	struct { /* anonymous struct used by BPF_MAP_*_ELEM commands */
		__u32		map_fd;
		__aligned_u64	key;
		union {
			__aligned_u64 value;
			__aligned_u64 next_key;
		};
		__u64		flags;
	};

	struct { /* anonymous struct used by BPF_PROG_LOAD command */
		__u32		prog_type;	/* one of enum bpf_prog_type */
		__u32		insn_cnt;
		__aligned_u64	insns;
		__aligned_u64	license;
		__u32		log_level;	/* verbosity level of verifier */
		__u32		log_size;	/* size of user buffer */
		__aligned_u64	log_buf;	/* user supplied buffer */
		__u32		kern_version;	/* checked when prog_type=kprobe */
	};

	struct { /* anonymous struct used by BPF_OBJ_* commands */
		__aligned_u64	pathname;
		__u32		bpf_fd;
		__u32		file_flags;
	};

	struct { /* anonymous struct used by BPF_PROG_ATTACH/DETACH commands */
		__u32		target_fd;	/* container object to attach to */
		__u32		attach_bpf_fd;	/* eBPF program to attach */
		__u32		attach_type;
		__u32		attach_flags;
	};
} __attribute__((aligned(8)));

/* integer value in 'imm' field of BPF_CALL instruction selects which helper
 * function eBPF program intends to call
 */
enum bpf_func_id {
	BPF_FUNC_unspec,
	BPF_FUNC_map_lookup_elem, /* void *map_lookup_elem(&map, &key) */
	BPF_FUNC_map_update_elem, /* int map_update_elem(&map, &key, &value, flags) */
	BPF_FUNC_map_delete_elem, /* int map_delete_elem(&map, &key) */
	BPF_FUNC_probe_read,      /* int bpf_probe_read(void *dst, int size, void *src) */
	BPF_FUNC_ktime_get_ns,    /* u64 bpf_ktime_get_ns(void) */
	BPF_FUNC_trace_printk,    /* int bpf_trace_printk(const char *fmt, int fmt_size, ...) */
	BPF_FUNC_get_prandom_u32, /* u32 prandom_u32(void) */
	BPF_FUNC_get_smp_processor_id, /* u32 raw_smp_processor_id(void) */

	/**
	 * skb_store_bytes(skb, offset, from, len, flags) - store bytes into packet
	 * @skb: pointer to skb
	 * @offset: offset within packet from skb->mac_header
	 * @from: pointer where to copy bytes from
	 * @len: number of bytes to store into packet
	 * @flags: bit 0 - if true, recompute skb->csum
	 *         other bits - reserved
	 * Return: 0 on success
	 */
	BPF_FUNC_skb_store_bytes,

	/**
	 * l3_csum_replace(skb, offset, from, to, flags) - recompute IP checksum
	 * @skb: pointer to skb
	 * @offset: offset within packet where IP checksum is located
	 * @from: old value of header field
	 * @to: new value of header field
	 * @flags: bits 0-3 - size of header field
	 *         other bits - reserved
	 * Return: 0 on success
	 */
	BPF_FUNC_l3_csum_replace,

	/**
	 * l4_csum_replace(skb, offset, from, to, flags) - recompute TCP/UDP checksum
	 * @skb: pointer to skb
	 * @offset: offset within packet where TCP/UDP checksum is located
	 * @from: old value of header field
	 * @to: new value of header field
	 * @flags: bits 0-3 - size of header field
	 *         bit 4 - is pseudo header
	 *         other bits - reserved
	 * Return: 0 on success
	 */
	BPF_FUNC_l4_csum_replace,

	/**
	 * bpf_tail_call(ctx, prog_array_map, index) - jump into another BPF program
	 * @ctx: context pointer passed to next program
	 * @prog_array_map: pointer to map which type is BPF_MAP_TYPE_PROG_ARRAY
	 * @index: index inside array that selects specific program to run
	 * Return: 0 on success
	 */
	BPF_FUNC_tail_call,

	/**
	 * bpf_clone_redirect(skb, ifindex, flags) - redirect to another netdev
	 * @skb: pointer to skb
	 * @ifindex: ifindex of the net device
	 * @flags: bit 0 - if set, redirect to ingress instead of egress
	 *         other bits - reserved
	 * Return: 0 on success
	 */
	BPF_FUNC_clone_redirect,

	/**
	 * u64 bpf_get_current_pid_tgid(void)
	 * Return: current->tgid << 32 | current->pid
	 */
	BPF_FUNC_get_current_pid_tgid,

	/**
	 * u64 bpf_get_current_uid_gid(void)
	 * Return: current_gid << 32 | current_uid
	 */
	BPF_FUNC_get_current_uid_gid,

	/**
	 * bpf_get_current_comm(char *buf, int size_of_buf)
	 * stores current->comm into buf
	 * Return: 0 on success
	 */
	BPF_FUNC_get_current_comm,

	/**
	 * bpf_get_cgroup_classid(skb) - retrieve a proc's classid
	 * @skb: pointer to skb
	 * Return: classid if != 0
	 */
	BPF_FUNC_get_cgroup_classid,
	BPF_FUNC_perf_event_read=22,	/* u64 bpf_perf_event_read(&map, index) */
	/**
	 * bpf_redirect(ifindex, flags) - redirect to another netdev
	 * @ifindex: ifindex of the net device
	 * @flags: bit 0 - if set, redirect to ingress instead of egress
	 *         other bits - reserved
	 * Return: TC_ACT_REDIRECT
	 */
	BPF_FUNC_redirect,

	/**
	 * bpf_get_route_realm(skb) - retrieve a dst's tclassid
	 * @skb: pointer to skb
	 * Return: realm if != 0
	 */
	BPF_FUNC_get_route_realm,

	/**
	 * bpf_perf_event_output(ctx, map, index, data, size) - output perf raw sample
	 * @ctx: struct pt_regs*
	 * @map: pointer to perf_event_array map
	 * @index: index of event in the map
	 * @data: data on stack to be output as raw data
	 * @size: size of data
	 * Return: 0 on success
	 */
	BPF_FUNC_perf_event_output,
	BPF_FUNC_skb_load_bytes,

	/**
	 * bpf_get_stackid(ctx, map, flags) - walk user or kernel stack and return id
	 * @ctx: struct pt_regs*
	 * @map: pointer to stack_trace map
	 * @flags: bits 0-7 - numer of stack frames to skip
	 *         bit 8 - collect user stack instead of kernel
	 *         bit 9 - compare stacks by hash only
	 *         bit 10 - if two different stacks hash into the same stackid
	 *                  discard old
	 *         other bits - reserved
	 * Return: >= 0 stackid on success or negative error
	 */
	BPF_FUNC_get_stackid,

	/**
	 * bpf_csum_diff(from, from_size, to, to_size, seed) - calculate csum diff
	 * @from: raw from buffer
	 * @from_size: length of from buffer
	 * @to: raw to buffer
	 * @to_size: length of to buffer
	 * @seed: optional seed
	 * Return: csum result
	 */
	BPF_FUNC_csum_diff,
	/**
	 * bpf_skb_change_proto(skb, proto, flags)
	 * Change protocol of the skb. Currently supported is
	 * v4 -> v6, v6 -> v4 transitions. The helper will also
	 * resize the skb. eBPF program is expected to fill the
	 * new headers via skb_store_bytes and lX_csum_replace.
	 * @skb: pointer to skb
	 * @proto: new skb->protocol type
	 * @flags: reserved
	 * Return: 0 on success or negative error
	 */
	BPF_FUNC_skb_change_proto=31,

	/**
	 * bpf_skb_change_type(skb, type)
	 * Change packet type of skb.
	 * @skb: pointer to skb
	 * @type: new skb->pkt_type type
	 * Return: 0 on success or negative error
	 */
	BPF_FUNC_skb_change_type,

	/**
	 * bpf_skb_under_cgroup(skb, map, index) - Check cgroup2 membership of skb
	 * @skb: pointer to skb
	 * @map: pointer to bpf_map in BPF_MAP_TYPE_CGROUP_ARRAY type
	 * @index: index of the cgroup in the bpf_map
	 * Return:
	 *   == 0 skb failed the cgroup2 descendant test
	 *   == 1 skb succeeded the cgroup2 descendant test
	 *    < 0 error
	 */
	BPF_FUNC_skb_under_cgroup,

	/**
	 * bpf_get_hash_recalc(skb)
	 * Retrieve and possibly recalculate skb->hash.
	 * @skb: pointer to skb
	 * Return: hash
	 */
	BPF_FUNC_get_hash_recalc,

	/**
	 * u64 bpf_get_current_task(void)
	 * Returns current task_struct
	 * Return: current
	 */
	BPF_FUNC_get_current_task,

	/**
	 * bpf_probe_write_user(void *dst, void *src, int len)
	 * safely attempt to write to a location
	 * @dst: destination address in userspace
	 * @src: source address on stack
	 * @len: number of bytes to copy
	 * Return: 0 on success or negative error
	 */
	BPF_FUNC_probe_write_user,

	/**
	 * bpf_skb_change_tail(skb, len, flags)
	 * The helper will resize the skb to the given new size,
	 * to be used f.e. with control messages.
	 * @skb: pointer to skb
	 * @len: new skb length
	 * @flags: reserved
	 * Return: 0 on success or negative error
	 */
	BPF_FUNC_skb_change_tail=38,

	/**
	 * bpf_skb_pull_data(skb, len)
	 * The helper will pull in non-linear data in case the
	 * skb is non-linear and not all of len are part of the
	 * linear section. Only needed for read/write with direct
	 * packet access.
	 * @skb: pointer to skb
	 * @len: len to make read/writeable
	 * Return: 0 on success or negative error
	 */
	BPF_FUNC_skb_pull_data,

	/**
	 * bpf_csum_update(skb, csum)
	 * Adds csum into skb->csum in case of CHECKSUM_COMPLETE.
	 * @skb: pointer to skb
	 * @csum: csum to add
	 * Return: csum on success or negative error
	 */
	BPF_FUNC_csum_update,

	/**
	 * bpf_set_hash_invalid(skb)
	 * Invalidate current skb>hash.
	 * @skb: pointer to skb
	 */
	BPF_FUNC_set_hash_invalid,

	/**
	 * int bpf_get_numa_node_id()
	 *     Return: Id of current NUMA node.
	 */
	BPF_FUNC_get_numa_node_id,

	/**
	 * int bpf_skb_change_head()
	 *     Grows headroom of skb and adjusts MAC header offset accordingly.
	 *     Will extends/reallocae as required automatically.
	 *     May change skb data pointer and will thus invalidate any check
	 *     performed for direct packet access.
	 *     @skb: pointer to skb
	 *     @len: length of header to be pushed in front
	 *     @flags: Flags (unused for now)
	 *     Return: 0 on success or negative error
	 */
	BPF_FUNC_skb_change_head,

	/**
	 * int bpf_xdp_adjust_head(xdp_md, delta)
	 *     Adjust the xdp_md.data by delta
	 *     @xdp_md: pointer to xdp_md
	 *     @delta: An positive/negative integer to be added to xdp_md.data
	 *     Return: 0 on success or negative on error
	 */
	BPF_FUNC_xdp_adjust_head,

	/**
	 * int bpf_probe_read_str(void *dst, int size, const void *unsafe_ptr)
	 *     Copy a NUL terminated string from unsafe address. In case the string
	 *     length is smaller than size, the target is not padded with further NUL
	 *     bytes. In case the string length is larger than size, just count-1
	 *     bytes are copied and the last byte is set to NUL.
	 *     @dst: destination address
	 *     @size: maximum number of bytes to copy, including the trailing NUL
	 *     @unsafe_ptr: unsafe address
	 *     Return:
	 *       > 0 length of the string including the trailing NUL on success
	 *       < 0 error
	 */
	BPF_FUNC_probe_read_str,

	/**
	 * u64 bpf_bpf_get_socket_cookie(skb)
	 *     Get the cookie for the socket stored inside sk_buff.
	 *     @skb: pointer to skb
	 *     Return: 8 Bytes non-decreasing number on success or 0 if the socket
	 *     field is missing inside sk_buff
	 */
	BPF_FUNC_get_socket_cookie,

	/**
	 * u32 bpf_get_socket_uid(skb)
	 *     Get the owner uid of the socket stored inside sk_buff.
	 *     @skb: pointer to skb
	 *     Return: uid of the socket owner on success or 0 if the socket pointer
	 *     inside sk_buff is NULL
	 */
	BPF_FUNC_get_socket_uid,

	__BPF_FUNC_MAX_ID,
};

/* All flags used by eBPF helper functions, placed here. */

/* BPF_FUNC_skb_store_bytes flags. */
#define BPF_F_RECOMPUTE_CSUM		(1ULL << 0)
#define BPF_F_INVALIDATE_HASH		(1ULL << 1)

/* BPF_FUNC_l3_csum_replace and BPF_FUNC_l4_csum_replace flags.
 * First 4 bits are for passing the header field size.
 */
#define BPF_F_HDR_FIELD_MASK		0xfULL

/* BPF_FUNC_l4_csum_replace flags. */
#define BPF_F_PSEUDO_HDR		(1ULL << 4)
#define BPF_F_MARK_MANGLED_0		(1ULL << 5)

/* BPF_FUNC_clone_redirect and BPF_FUNC_redirect flags. */
#define BPF_F_INGRESS			(1ULL << 0)

/* BPF_FUNC_skb_set_tunnel_key and BPF_FUNC_skb_get_tunnel_key flags. */
#define BPF_F_TUNINFO_IPV6		(1ULL << 0)

/* BPF_FUNC_get_stackid flags. */
#define BPF_F_SKIP_FIELD_MASK		0xffULL
#define BPF_F_USER_STACK		(1ULL << 8)
#define BPF_F_FAST_STACK_CMP		(1ULL << 9)
#define BPF_F_REUSE_STACKID		(1ULL << 10)

/* BPF_FUNC_perf_event_output flags and BPF_FUNC_perf_event_read. */
#define BPF_F_INDEX_MASK		0xffffffffULL
#define BPF_F_CURRENT_CPU		BPF_F_INDEX_MASK
/* BPF_FUNC_perf_event_output for sk_buff input context. */
#define BPF_F_CTXLEN_MASK		(0xfffffULL << 32)

/* user accessible mirror of in-kernel sk_buff.
 * new fields can only be added to the end of this structure
 */
struct __sk_buff {
	__u32 len;
	__u32 pkt_type;
	__u32 mark;
	__u32 queue_mapping;
	__u32 protocol;
	__u32 vlan_present;
	__u32 vlan_tci;
	__u32 vlan_proto;
	__u32 priority;
	__u32 ingress_ifindex;
	__u32 ifindex;
	__u32 tc_index;
	__u32 cb[5];
	__u32 hash;
	__u32 tc_classid;
	__u32 data;
	__u32 data_end;
};

/* User return codes for XDP prog type.
 * A valid XDP program must return one of these defined values. All other
 * return codes are reserved for future use. Unknown return codes will result
 * in packet drop.
 */
enum xdp_action {
	XDP_ABORTED = 0,
	XDP_DROP,
	XDP_PASS,
};

/* user accessible metadata for XDP packet hook
 * new fields must be added to the end of this structure
 */
struct xdp_md {
	__u32 data;
	__u32 data_end;
};

#endif /* _UAPI__LINUX_BPF_H__ */
