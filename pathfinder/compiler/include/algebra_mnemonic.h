/**
 * @file
 *
 * Mnemonic abbreviations for algebra constructors.
 * (Generic algebra constructors)
 *
 * The contents of this file are subject to the Pathfinder Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://monetdb.cwi.nl/Legal/PathfinderLicense-1.1.html
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the Pathfinder system.
 *
 * The Initial Developer of the Original Code is the Database &
 * Information Systems Group at the University of Konstanz, Germany.
 * Portions created by the University of Konstanz are Copyright (C)
 * 2000-2006 University of Konstanz.  All Rights Reserved.
 *
 * $Id$
 */

/** atomic natural number construction */
#define lit_nat(n)      PFalg_lit_nat (n)

/** atomic integer construction */
#define lit_int(i)      PFalg_lit_int (i)

/** atomic string construction */
#define lit_str(s)      PFalg_lit_str (s)

/** atomic float construction */
#define lit_dec(f)      PFalg_lit_dec (f)

/** atomic double construction */
#define lit_dbl(d)      PFalg_lit_dbl (d)

/** atomic boolean construction */
#define lit_bln(b)      PFalg_lit_bln (b)

/** atomic QName construction */
#define lit_qname(b)    PFalg_lit_qname (b)

/** tuple construction */
#define tuple(...)      PFalg_tuple (__VA_ARGS__)

/** (schema) attribute construction */
#define att(n,t)        PFalg_att ((n),(t))

/** attribute list construction */
#define attlist(...)    PFalg_attlist (__VA_ARGS__)

/** item in the projection list */
#define proj(a,b)       PFalg_proj ((a),(b))

/** a sort specification list is just another attribute list */
#define sortby(...)     PFalg_attlist (__VA_ARGS__)

/* vim:set shiftwidth=4 expandtab: */
