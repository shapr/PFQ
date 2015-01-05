/***************************************************************
 *
 * (C) 2011-14 Nicola Bonelli <nicola@pfq.io>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 ****************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/crc16.h>

#include <pf_q-module.h>
#include <pf_q-sparse.h>

#include "headers.h"
#include "misc.h"


static Action_SkBuff
dummy(arguments_t args, SkBuff b)
{
        const int data = get_arg(int,args);

	SkBuff new;

	printk(KERN_INFO "[PFQ/lang] dummy = %d\n", data);

        new = pfq_copy_buff(b);

	if (new.skb == NULL) {
                printk(KERN_INFO "[PFQ/lang] clone error!!!\n");
                return Drop(b);
	}

        printk(KERN_INFO "[PFQ/lang] packet cloned: %p -> %p\n", new.skb, b.skb);

        return Pass(new);
}


static Action_SkBuff
vdummy(arguments_t args, SkBuff b)
{
        const int *data = get_array(int,args);
	size_t len = get_array_len(args);
        int n;

	printk(KERN_INFO "[PFQ/lang] vdummy len: %zu...\n", len);

	for(n = 0; n < len; n++)
	{
		printk(KERN_INFO "[PFQ/lang]  data[%d] = %d\n", n, data[n]);
	}

        return Pass(b);
}

static int
dummy_init(arguments_t args)
{
	printk(KERN_INFO "[PFQ/lang] %s :)\n", __PRETTY_FUNCTION__);
	return 0;
}

static int
dummy_fini(arguments_t args)
{
	printk(KERN_INFO "[PFQ/lang] %s :(\n", __PRETTY_FUNCTION__);
	return 0;
}


static Action_SkBuff
inc_counter(arguments_t args, SkBuff b)
{
        const int idx = get_arg(int,args);

        sparse_counter_t * ctr;

        ctr = get_counter(b, idx);
        if (ctr)  {
                sparse_inc(ctr);
        }
        else {
                if (printk_ratelimit())
                        printk(KERN_INFO "[PFQ/lang] counter[%d]: bad index!\n", idx);
        }

        return Pass(b);
}


static Action_SkBuff
dec_counter(arguments_t args, SkBuff b)
{
        const int idx = get_arg(int,args);

        sparse_counter_t * ctr;

        ctr = get_counter(b, idx);
        if (ctr)  {
                sparse_dec(ctr);
        }
        else {
                if (printk_ratelimit())
                        printk(KERN_INFO "[PFQ/lang] counter[%d]: bad index!\n", idx);
        }

        return Pass(b);
}


static Action_SkBuff
crc16_sum(arguments_t args, SkBuff b)
{
	u16 crc = crc16(0, (u8 const *)eth_hdr(b.skb), b.skb->len);
	set_state(b, crc);

        return Pass(b);
}


static Action_SkBuff
log_msg(arguments_t args, SkBuff b)
{
	const char *msg = get_arg(const char *, args);

	if (printk_ratelimit())
		printk(KERN_INFO "[PFQ/lang] log_msg: %s\n", msg);

	return Pass(b);
}


static Action_SkBuff
log_buff(arguments_t args, SkBuff b)
{
	if (!printk_ratelimit())
		return Pass(b);

	printk(KERN_INFO "[PFQ/lang] [%p] len=%u head=%u tail=%u\n", b.skb,
								b.skb->len,
								skb_headroom(b.skb),
								skb_tailroom(b.skb));

	printk(KERN_INFO "      [%p] %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x...\n",
				b.skb->data,
				b.skb->data[0],
				b.skb->data[1],
				b.skb->data[2],
				b.skb->data[3],
				b.skb->data[4],
				b.skb->data[5],
				b.skb->data[6],
				b.skb->data[7],
				b.skb->data[8],
				b.skb->data[9],
				b.skb->data[10],
				b.skb->data[11],
				b.skb->data[12],
				b.skb->data[13]);

	return Pass(b);
}


static Action_SkBuff
log_packet(arguments_t args, SkBuff b)
{
	if (!printk_ratelimit())
		return Pass(b);

	if (eth_hdr(b.skb)->h_proto == __constant_htons(ETH_P_IP))
	{
		struct iphdr _iph;
		const struct iphdr *ip;

		ip = skb_header_pointer(b.skb, b.skb->mac_len, sizeof(_iph), &_iph);
		if (ip == NULL)
			return Pass(b);

		switch(ip->protocol)
		{
		case IPPROTO_UDP: {
			struct udphdr _udph; const struct udphdr *udp;
			udp = skb_header_pointer(b.skb, b.skb->mac_len + (ip->ihl<<2), sizeof(struct udphdr), &_udph);
			if (udp == NULL)
				return Pass(b);

			printk(KERN_INFO "[PFQ/lang] IP %pI4.%d > %pI4.%d: UDP\n", &ip->saddr, ntohs(udp->source),
						         		      &ip->daddr, ntohs(udp->dest));
			return Pass(b);
		}
		case IPPROTO_TCP: {
			struct tcphdr _tcph; const struct tcphdr *tcp;
			tcp = skb_header_pointer(b.skb, b.skb->mac_len + (ip->ihl<<2), sizeof(struct tcphdr), &_tcph);
			if (tcp == NULL)
				return Pass(b);

			printk(KERN_INFO "[PFQ/lang] IP %pI4.%d > %pI4.%d: TCP\n", &ip->saddr, ntohs(tcp->source),
									      &ip->daddr, ntohs(tcp->dest));
			return Pass(b);
		}
		case IPPROTO_ICMP: {

			printk(KERN_INFO "[PFQ/lang] IP %pI4 > %pI4: ICMP\n", &ip->saddr, &ip->daddr);
			return Pass(b);
		}
		default: {

			printk(KERN_INFO "[PFQ/lang] IP %pI4 > %pI4: proto %x\n", &ip->saddr, &ip->daddr,
									     ip->protocol);
			return Pass(b);
		}

		}

	} else
		printk(KERN_INFO "[PFQ/lang] ETH proto %x\n", ntohs(eth_hdr(b.skb)->h_proto));

        return Pass(b);
}


static Action_SkBuff
inv(arguments_t args, SkBuff b)
{
	function_t expr = get_arg(function_t, args);
	SkBuff nb = EVAL_FUNCTION(expr, b).value;

	if (!nb.skb || is_drop(PFQ_CB(nb.skb)->monad->fanout))
		return Copy(nb);

	return Drop(nb);
}


static Action_SkBuff
par(arguments_t args, SkBuff b)
{
	function_t f = get_arg_0(function_t, args);
	function_t g = get_arg_1(function_t, args);

	fanout_t fout = PFQ_CB(b.skb)->monad->fanout;

	Action_SkBuff a = EVAL_FUNCTION(f, b);

	if (!a.value.skb || is_drop(PFQ_CB(a.value.skb)->monad->fanout)) {

		/* restore the original fanout.. */

		PFQ_CB(b.skb)->monad->fanout = fout;

		return EVAL_FUNCTION(g, b);
	}

	return a;
}


struct pfq_function_descr misc_functions[] = {

        { "dummy",      "Int   -> SkBuff -> Action SkBuff",  		dummy, dummy_init,  dummy_fini },
        { "vdummy",     "[Int] -> SkBuff -> Action SkBuff",     	vdummy, dummy_init,  dummy_fini },

        { "inc", 	"Int -> SkBuff -> Action SkBuff",     		inc_counter 	},
        { "dec", 	"Int -> SkBuff -> Action SkBuff",    		dec_counter 	},
 	{ "mark", 	"CULong -> SkBuff -> Action SkBuff",  		mark		},
        { "crc16", 	"SkBuff -> Action SkBuff", 			crc16_sum	},
        { "log_msg",  	"String -> SkBuff -> Action SkBuff", 		log_msg 	},
        { "log_buff",   "SkBuff -> Action SkBuff", 			log_buff 	},
        { "log_packet", "SkBuff -> Action SkBuff", 			log_packet	},

        { "inv", 	"(SkBuff -> Action SkBuff) -> SkBuff -> Action SkBuff",     				inv },
        { "par", 	"(SkBuff -> Action SkBuff) -> (SkBuff -> Action SkBuff) -> SkBuff -> Action SkBuff",    par },
        { NULL }};


