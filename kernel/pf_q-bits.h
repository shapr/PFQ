/***************************************************************
 *                                                
 * (C) 2011-12 Nicola Bonelli <nicola.bonelli@cnit.it>   
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

#ifndef _PF_Q_BITS_H_
#define _PF_Q_BITS_H_ 
 

#define pfq_ctz(n) \
	 __builtin_choose_expr(__builtin_types_compatible_p(typeof(n),unsigned int), __builtin_ctz(n), \
         __builtin_choose_expr(__builtin_types_compatible_p(typeof(n),unsigned long), __builtin_ctzl(n), \
         __builtin_choose_expr(__builtin_types_compatible_p(typeof(n),unsigned long long), __builtin_ctzll(n), (void)0 )))

#define bitmask_for_each(mask, n) \
	for(; n = pfq_ctz(mask), mask ; mask^=(1L << n))


#endif /* _PF_Q_BITS_H_ */
