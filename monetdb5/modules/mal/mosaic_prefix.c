/*
 * The contents of this file are subject to the MonetDB Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.monetdb.org/Legal/MonetDBLicense
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the MonetDB Database System.
 *
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
 *                * Copyright August 2008-2014 MonetDB B.V.
 * All Rights Reserved.
 */

/*
 * (c)2014 author Martin Kersten
 * Blocked bit_ prefix compression
 */

#include "monetdb_config.h"
#include "mosaic.h"
#include "mosaic_prefix.h"

/* Beware, the dump routines use the compressed part of the task */
void
MOSdump_prefix(Client cntxt, MOStask task)
{
	MosaicBlk blk= task->blk;
	void *val = (void*)(((char*) blk) + MosaicBlkSize);

	mnstr_printf(cntxt->fdout,"#rle "BUNFMT" ", MOSgetCnt(blk));
	switch(task->type){
	case TYPE_bte:
		mnstr_printf(cntxt->fdout,"bte %hhd", *(bte*) val); break;
	case TYPE_sht:
		mnstr_printf(cntxt->fdout,"sht %hd", *(sht*) val); break;
	case TYPE_int:
		mnstr_printf(cntxt->fdout,"int %d", *(int*) val); break;
	case  TYPE_oid:
		mnstr_printf(cntxt->fdout,"oid "OIDFMT, *(oid*) val); break;
	case  TYPE_lng:
		mnstr_printf(cntxt->fdout,"lng "LLFMT, *(lng*) val); break;
#ifdef HAVE_HGE
	case  TYPE_hge:
		mnstr_printf(cntxt->fdout,"hge %.40g", (dbl) *(hge*) val); break;
#endif
	case  TYPE_wrd:
		mnstr_printf(cntxt->fdout,"wrd "SZFMT, *(wrd*) val); break;
	case TYPE_flt:
		mnstr_printf(cntxt->fdout,"flt  %f", *(flt*) val); break;
	case TYPE_dbl:
		mnstr_printf(cntxt->fdout,"flt  %f", *(dbl*) val); break;
	case TYPE_str:
		mnstr_printf(cntxt->fdout,"str TBD"); break;
	default:
		if( task->type == TYPE_date)
			mnstr_printf(cntxt->fdout,"date %d ", *(int*) val); 
		if( task->type == TYPE_daytime)
			mnstr_printf(cntxt->fdout,"daytime %d ", *(int*) val);
		if( task->type == TYPE_timestamp)
			mnstr_printf(cntxt->fdout,"int "LLFMT, *(lng*) val); 
	}
	mnstr_printf(cntxt->fdout,"\n");
}

void
MOSadvance_prefix(Client cntxt, MOStask task)
{
	int bits, bytes;
	(void) cntxt;

	task->start += MOSgetCnt(task->blk);
	switch(ATOMsize(task->type)){
	case 4:
		{ int *dst = (int*)  (((char*) task->blk) + MosaicBlkSize);
		  int mask = *dst++;
		  int val = *dst++;
			bits = val & (~mask);
			bytes = (MOSgetCnt(task->blk) * bits)/8 + ((MOSgetCnt(task->blk) %8) != 0);
			task->blk = (MosaicBlk) (((char*) dst)  + wordaligned(bytes,int)); 
			mnstr_printf(cntxt->fdout,"advance mask width %d bytes %d %d \n",bits,bytes,(int)wordaligned(bytes,int));
		}
			break;
	}
}

void
MOSskip_prefix(Client cntxt, MOStask task)
{
	MOSadvance_prefix(cntxt, task);
	if ( MOSgetTag(task->blk) == MOSAIC_EOL)
		task->blk = 0; // ENDOFLIST
}

// later turn into logaritmic search
#define Prefix(Bits,Mask,X,Y,N) \
{ int k, m = 1; \
  for(k=0;k<N;k++, X>>=1, Y>>=1){\
	if( X == Y) break;\
	m= (m<<1)|1; \
  }\
  Bits = N-k;\
  Mask = ~(m >>1);\
} 


#define Estimate(TYPE)\
{	TYPE *v = (TYPE*) task->src, *w= v+1, val= *v, val2= *w,  mask;\
	Prefix(bits, mask, val, val2, 8 * sizeof(TYPE));\
	val = *v & mask;\
	for(i = 1; i < task->elm; w++, i++){\
		if ( val != (*w & mask) )\
			break;\
	}\
	if ( i > MOSlimit() ) i = MOSlimit();\
	bits = i * (8* sizeof(TYPE) -bits);\
	store = bits/8 + ((bits % 8) >0);\
	store = MosaicBlkSize + 2 * sizeof(int) + wordaligned( store,TYPE);\
	factor = ( (flt)i * sizeof(TYPE))/ store;\
}

// calculate the expected reduction 
flt
MOSestimate_prefix(Client cntxt, MOStask task)
{	BUN i = -1;
	flt factor = 1.0;
	int bits;
	lng store;
	(void) cntxt;
	(void) bits;

	if( task->elm >= 2)
	switch(ATOMsize(task->type)){
	case 4:
		{	int *v = (int*) task->src, *w= v+1, val= *v,val2= *w, mask;
			Prefix(bits, mask, val, val2, 32);
			val = *v & mask;
			for(i = 0; i < task->elm; w++, i++){
				if ( val != (*w & mask) )
					break;
			}
			if ( i > MOSlimit() ) i = MOSlimit();
			bits = i * (32 -bits);
			store = bits/8 + ((bits % 8) >0);
			store = MosaicBlkSize + 2 * sizeof(int) + wordaligned( store,int);
			factor = ( (flt)i * sizeof(int))/ store;
		}
	}
#ifdef _DEBUG_MOSAIC_
	mnstr_printf(cntxt->fdout,"#estimate rle "BUNFMT" elm %4.3f factor\n",i,factor);
#endif
	return factor;
}

// insert a series of values into the compressor block using rle.
#define PREFIXcompress(TYPE)\

// assume filling an already zeroed vector
#define SetBits(B, I, Val, Mask,Bits) \

void
MOScompress_prefix(Client cntxt, MOStask task)
{
	BUN i ;
	unsigned int *base;
	unsigned int bits, rbits;
	MosaicBlk blk = task->blk;

	(void) cntxt;
	MOSsetTag(blk, MOSAIC_PREFIX);

	if( task->elm >=2 )
	switch(ATOMsize(task->type)){
	case 4:
		{	unsigned int *v = (unsigned int*) task->src, *w= v+1, val = *v, val2 = *w, mask, m;
			unsigned int *dst = (unsigned int*)  (((char*) blk) + MosaicBlkSize);
			BUN limit = task->elm > MOSlimit()? MOSlimit(): task->elm;

			Prefix(bits, mask, val, val2, 32);
			rbits = 32-bits;
			*dst++ = mask;
			*dst = *v & mask;
			*dst = *dst | rbits; // bits outside mask
			dst++;
			base  = (unsigned int*) dst; // start of bit vector
			*base = 0;
			
			val = *v & mask;	//reference value
mnstr_printf(cntxt->fdout,"compress %o %o val %d bits %d, %d mask %o\n",*v,*w,val,bits, rbits,mask);
			for(w = v, i = 0; i < limit; w++, i++){
				if ( val  != (*w & mask) )
					break;
				m = *w & (~mask); // residu
mnstr_printf(cntxt->fdout,"compress %d residu %d %o\n",*w,m,m);
{	unsigned int cell = (i * rbits)/32;
	unsigned int lshift= 32 -( (i * rbits) % 32) ;
	if ( lshift > rbits){
		base[cell]= base[cell] | (m << (lshift-rbits));
		mnstr_printf(cntxt->fdout,"[%d] shift %d rbits %d cell %o\n",cell, lshift, rbits, base[cell]);
	}else{ 
		base[cell]= base[cell] | (m >> (rbits-lshift));
		base[cell+1]= 0 |(((m  & ((~mask) >> ((rbits-lshift)))) << (32 -(rbits-lshift))));
		mnstr_printf(cntxt->fdout,"[%d] shift %d %d %d cell %o %o val %o %o\n", cell, lshift, rbits,
			(32-(rbits-lshift)),base[cell],base[cell+1], (m >> (rbits-lshift)),  (m  & ((~mask) >> ((rbits-lshift)))));
	}
}
			}
			MOSincCnt(blk,i);
			task->src += i * sizeof(int);
		}
	}
#ifdef _DEBUG_MOSAIC_
	MOSdump_prefix(cntxt, task);
#endif
}

// the inverse operator, extend the src
#define PREFIXdecompress(TYPE)\
{	TYPE val = *(TYPE*) task->dst;\
	BUN lim = MOSgetCnt(blk);\
	for(i = 0; i < lim; i++)\
		((TYPE*)task->src)[i] = val;\
	task->src += i * sizeof(TYPE);\
}

/*
static int bitmasks[33]= {
0,   01,   03,   07,  017,  037,   077,  0177,
0377, 0777, 01777, 03777, 07777, 017777, 037777, 077777,
0177777, 0377777, 0777777, 01777777, 03777777, 0777777, 017777777, 037777777,
077777777, 0177777777, 0377777777, 0777777777, 01777777777, 03777777777, 07777777777, 017777777777,
037777777777 };
*/

void
MOSdecompress_prefix(Client cntxt, MOStask task)
{
	MosaicBlk blk =  ((MosaicBlk) task->blk);
	BUN i;
	unsigned int rbits;
	unsigned int *base;
	(void) cntxt;

	switch(ATOMsize(task->type)){
	case 4:
		{	unsigned int *dst =  (unsigned int*)  (((char*) blk) + MosaicBlkSize);
			unsigned int mask = *dst++, val  =  *dst++, v;
			unsigned int m,m1,m2;
			BUN lim= MOSgetCnt(blk);
			rbits = val & (~mask);
			for(m=1, i=1; i < (BUN) rbits; i++)
				m= (m << 1) | 1;
			base = (unsigned int*) dst;
			val = val & mask;
			mnstr_printf(cntxt->fdout,"decompress rbits %d mask %o val %d\n",rbits,m,val);
			for(i = 0; i < lim; i++){
{ unsigned int cell = (i*rbits)/32;
	unsigned int lshift= 32 -(((i+1) * rbits) % 32);
	unsigned int l = base[cell];
	if ( lshift > rbits){
		m1 = (l & (m <<(lshift-rbits)))>> (lshift-rbits);
		mnstr_printf(cntxt->fdout,"[%d]cell %o lshift %d m %d\n", cell,  base[cell],lshift,m1);
		v = val  | m1;
	  }else{ 
		m1 =(l & (m >> (rbits-lshift)));
		v = val | (l & (m1 << lshift));
		l = base[cell+1];
		m2 = ( l >>(32-(rbits-lshift)));
		v= v | (m1 <<(rbits-lshift)) | m2;
		mnstr_printf(cntxt->fdout,"[%d] shift %d %d cell %o %o val %o %o\n", cell, lshift, (32-(rbits-lshift)),base[cell],base[cell+1], m1,  m2);
	  }
}
				((int*)task->src)[i] = v;
			}
			task->src += i * sizeof(int);
		}
		break;
	}
}

// perform relational algebra operators over non-compressed chunks
// They are bound by an oid range and possibly a candidate list

#define  subselect_prefix(TPE) \
{ 	TPE *val= (TPE*) (((char*) task->blk) + MosaicBlkSize);\
\
	if( !*anti){\
		if( *(TPE*) low == TPE##_nil && *(TPE*) hgh == TPE##_nil){\
			for( ; first < last; first++){\
				MOSskipit();\
				*o++ = (oid) first;\
			}\
		} else\
		if( *(TPE*) low == TPE##_nil ){\
			cmp  =  ((*hi && *(TPE*)val <= * (TPE*)hgh ) || (!*hi && *(TPE*)val < *(TPE*)hgh ));\
			if (cmp )\
			for( ; first < last; first++){\
				MOSskipit();\
				*o++ = (oid) first;\
			}\
		} else\
		if( *(TPE*) hgh == TPE##_nil ){\
			cmp  =  ((*li && *(TPE*)val >= * (TPE*)low ) || (!*li && *(TPE*)val > *(TPE*)low ));\
			if (cmp )\
			for( ; first < last; first++){\
				MOSskipit();\
				*o++ = (oid) first;\
			}\
		} else{\
			cmp  =  ((*hi && *(TPE*)val <= * (TPE*)hgh ) || (!*hi && *(TPE*)val < *(TPE*)hgh )) &&\
					((*li && *(TPE*)val >= * (TPE*)low ) || (!*li && *(TPE*)val > *(TPE*)low ));\
			if (cmp )\
			for( ; first < last; first++){\
				MOSskipit();\
				*o++ = (oid) first;\
			}\
		}\
	} else {\
		if( *(TPE*) low == TPE##_nil && *(TPE*) hgh == TPE##_nil){\
			/* nothing is matching */\
		} else\
		if( *(TPE*) low == TPE##_nil ){\
			cmp  =  ((*hi && *(TPE*)val <= * (TPE*)hgh ) || (!*hi && *(TPE*)val < *(TPE*)hgh ));\
			if ( !cmp )\
			for( ; first < last; first++){\
				MOSskipit();\
				*o++ = (oid) first;\
			}\
		} else\
		if( *(TPE*) hgh == TPE##_nil ){\
			cmp  =  ((*li && *(TPE*)val >= * (TPE*)low ) || (!*li && *(TPE*)val > *(TPE*)low ));\
			if ( !cmp )\
			for( ; first < last; first++, val++){\
				MOSskipit();\
				*o++ = (oid) first;\
			}\
		} else{\
			cmp  =  ((*hi && *(TPE*)val <= * (TPE*)hgh ) || (!*hi && *(TPE*)val < *(TPE*)hgh )) &&\
					((*li && *(TPE*)val >= * (TPE*)low ) || (!*li && *(TPE*)val > *(TPE*)low ));\
			if (!cmp)\
			for( ; first < last; first++, val++){\
				MOSskipit();\
				*o++ = (oid) first;\
			}\
		}\
	}\
}

str
MOSsubselect_prefix(Client cntxt,  MOStask task, void *low, void *hgh, bit *li, bit *hi, bit *anti){
	oid *o;
	int cmp;
	BUN first,last;
	(void) cntxt;

	// set the oid range covered
	first = task->start;
	last = first + MOSgetCnt(task->blk);

	if (task->cl && *task->cl > last){
		MOSadvance_prefix(cntxt,task);
		return MAL_SUCCEED;
	}
	o = task->lb;

	switch(ATOMstorage(task->type)){
	case TYPE_bit: subselect_prefix(bit); break;
	case TYPE_bte: subselect_prefix(bte); break;
	case TYPE_sht: subselect_prefix(sht); break;
	case TYPE_lng: subselect_prefix(lng); break;
	case TYPE_oid: subselect_prefix(oid); break;
	case TYPE_wrd: subselect_prefix(wrd); break;
	case TYPE_flt: subselect_prefix(flt); break;
	case TYPE_dbl: subselect_prefix(dbl); break;
#ifdef HAVE_HGE
	case TYPE_hge: subselect_prefix(hge); break;
#endif
	case TYPE_int:
	// Expanded MOSselect_prefix for debugging
	{ 	int *val= (int*) (((char*) task->blk) + MosaicBlkSize);

		if( !*anti){
			if( *(int*) low == int_nil && *(int*) hgh == int_nil){
				for( ; first < last; first++){
					MOSskipit();
					*o++ = (oid) first;
				}
			} else
			if( *(int*) low == int_nil ){
				cmp  =  ((*hi && *(int*)val <= * (int*)hgh ) || (!*hi && *(int*)val < *(int*)hgh ));
				if (cmp )
				for( ; first < last; first++){
					MOSskipit();
					*o++ = (oid) first;
				}
			} else
			if( *(int*) hgh == int_nil ){
				cmp  =  ((*li && *(int*)val >= * (int*)low ) || (!*li && *(int*)val > *(int*)low ));
				if (cmp )
				for( ; first < last; first++){
					MOSskipit();
					*o++ = (oid) first;
				}
			} else{
				cmp  =  ((*hi && *(int*)val <= * (int*)hgh ) || (!*hi && *(int*)val < *(int*)hgh )) &&
						((*li && *(int*)val >= * (int*)low ) || (!*li && *(int*)val > *(int*)low ));
				if (cmp )
				for( ; first < last; first++){
					MOSskipit();
					*o++ = (oid) first;
				}
			}
		} else {
			if( *(int*) low == int_nil && *(int*) hgh == int_nil){
				/* nothing is matching */
			} else
			if( *(int*) low == int_nil ){
				cmp  =  ((*hi && *(int*)val <= * (int*)hgh ) || (!*hi && *(int*)val < *(int*)hgh ));
				if ( !cmp )
				for( ; first < last; first++){
					MOSskipit();
					*o++ = (oid) first;
				}
			} else
			if( *(int*) hgh == int_nil ){
				cmp  =  ((*li && *(int*)val >= * (int*)low ) || (!*li && *(int*)val > *(int*)low ));
				if ( !cmp )
				for( ; first < last; first++){
					MOSskipit();
					*o++ = (oid) first;
				}
			} else{
				cmp  =  ((*hi && *(int*)val <= * (int*)hgh ) || (!*hi && *(int*)val < *(int*)hgh )) &&
						((*li && *(int*)val >= * (int*)low ) || (!*li && *(int*)val > *(int*)low ));
				if (!cmp)
				for( ; first < last; first++){
					MOSskipit();
					*o++ = (oid) first;
				}
			}
		}
	}
	break;
	case  TYPE_str:
		// we only have to look at the index width, not the values
		switch(task->b->T->width){
		case 1: break;
		case 2: break;
		case 4: break;
		case 8: break;
		}
		break;
	default:
		if( task->type == TYPE_date)
			subselect_prefix(date); 
		if( task->type == TYPE_daytime)
			subselect_prefix(daytime); 
		if( task->type == TYPE_timestamp)
			subselect_prefix(lng); 
	}
	MOSadvance_prefix(cntxt,task);
	task->lb = o;
	return MAL_SUCCEED;
}

#define thetasubselect_prefix(TPE)\
{ 	TPE low,hgh,*v;\
	low= hgh = TPE##_nil;\
	if ( strcmp(oper,"<") == 0){\
		hgh= *(TPE*) val;\
		hgh = PREVVALUE##TPE(hgh);\
	} else\
	if ( strcmp(oper,"<=") == 0){\
		hgh= *(TPE*) val;\
	} else\
	if ( strcmp(oper,">") == 0){\
		low = *(TPE*) val;\
		low = NEXTVALUE##TPE(low);\
	} else\
	if ( strcmp(oper,">=") == 0){\
		low = *(TPE*) val;\
	} else\
	if ( strcmp(oper,"!=") == 0){\
		low = hgh = *(TPE*) val;\
		anti++;\
	} else\
	if ( strcmp(oper,"==") == 0){\
		hgh= low= *(TPE*) val;\
	} \
	v = (TPE*) (((char*)task->blk) + MosaicBlkSize);\
	if( (low == TPE##_nil || * v >= low) && (* v <= hgh || hgh == TPE##_nil) ){\
			if ( !anti) {\
				for( ; first < last; first++){\
					MOSskipit();\
					*o++ = (oid) first;\
				}\
			}\
	} else\
	if( anti)\
		for( ; first < last; first++){\
			MOSskipit();\
			*o++ = (oid) first;\
		}\
}

str
MOSthetasubselect_prefix(Client cntxt,  MOStask task, void *val, str oper)
{
	oid *o;
	int anti=0;
	BUN first,last;
	(void) cntxt;
	
	// set the oid range covered and advance scan range
	first = task->start;
	last = first + MOSgetCnt(task->blk);

	if (task->cl && *task->cl > last){
		MOSskip_prefix(cntxt,task);
		return MAL_SUCCEED;
	}
	o = task->lb;

	switch(ATOMstorage(task->type)){
	case TYPE_bit: thetasubselect_prefix(bit); break;
	case TYPE_bte: thetasubselect_prefix(bte); break;
	case TYPE_sht: thetasubselect_prefix(sht); break;
	case TYPE_lng: thetasubselect_prefix(lng); break;
	case TYPE_oid: thetasubselect_prefix(oid); break;
	case TYPE_wrd: thetasubselect_prefix(wrd); break;
	case TYPE_flt: thetasubselect_prefix(flt); break;
	case TYPE_dbl: thetasubselect_prefix(dbl); break;
#ifdef HAVE_HGE
	case TYPE_hge: thetasubselect_prefix(hge); break;
#endif
	case TYPE_int:
		{ 	int low,hgh,*v;
			low= hgh = int_nil;
			if ( strcmp(oper,"<") == 0){
				hgh= *(int*) val;
				hgh = PREVVALUEint(hgh);
			} else
			if ( strcmp(oper,"<=") == 0){
				hgh= *(int*) val;
			} else
			if ( strcmp(oper,">") == 0){
				low = *(int*) val;
				low = NEXTVALUEint(low);
			} else
			if ( strcmp(oper,">=") == 0){
				low = *(int*) val;
			} else
			if ( strcmp(oper,"!=") == 0){
				low = hgh = *(int*) val;
				anti++;
			} else
			if ( strcmp(oper,"==") == 0){
				hgh= low= *(int*) val;
			} 
			v = (int*) (((char*)task->blk) + MosaicBlkSize);
			if( (low == int_nil || * v >= low) && (* v <= hgh || hgh == int_nil) ){
					if ( !anti) {
						for( ; first < last; first++){
							MOSskipit();
							*o++ = (oid) first;
						}
					}
			} else
			if( anti)
				for( ; first < last; first++){
					MOSskipit();
					*o++ = (oid) first;
				}
		}
		break;
	case  TYPE_str:
		// we only have to look at the index width, not the values
		switch(task->b->T->width){
		case 1: break;
		case 2: break;
		case 4: break;
		case 8: break;
		}
		break;
	}
	MOSskip_prefix(cntxt,task);
	task->lb =o;
	return MAL_SUCCEED;
}

#define leftfetchjoin_prefix(TPE)\
{	TPE val, *v;\
	v= (TPE*) task->src;\
	val = *(TPE*) (((char*) task->blk) + MosaicBlkSize);\
	for(; first < last; first++){\
		MOSskipit();\
		*v++ = val;\
		task->n--;\
	}\
	task->src = (char*) v;\
}

str
MOSleftfetchjoin_prefix(Client cntxt,  MOStask task)
{
	BUN first,last;
	(void) cntxt;

	// set the oid range covered and advance scan range
	first = task->start;
	last = first + MOSgetCnt(task->blk);

	switch(ATOMstorage(task->type)){
		case TYPE_bit: leftfetchjoin_prefix(bit); break;
		case TYPE_bte: leftfetchjoin_prefix(bte); break;
		case TYPE_sht: leftfetchjoin_prefix(sht); break;
		case TYPE_lng: leftfetchjoin_prefix(lng); break;
		case TYPE_oid: leftfetchjoin_prefix(oid); break;
		case TYPE_wrd: leftfetchjoin_prefix(wrd); break;
		case TYPE_flt: leftfetchjoin_prefix(flt); break;
		case TYPE_dbl: leftfetchjoin_prefix(dbl); break;
#ifdef HAVE_HGE
		case TYPE_hge: leftfetchjoin_prefix(hge); break;
#endif
		case TYPE_int:
		{	int val, *v;
			v= (int*) task->src;
			val = *(int*) (((char*) task->blk) + MosaicBlkSize);
			for(; first < last; first++){
				MOSskipit();
				*v++ = val;
				task->n--;
			}
			task->src = (char*) v;
		}
		break;
	case  TYPE_str:
		// we only have to look at the index width, not the values
		switch(task->b->T->width){
		case 1: leftfetchjoin_prefix(bte); break;
		case 2: leftfetchjoin_prefix(sht); break;
		case 4: leftfetchjoin_prefix(int); break;
		case 8: leftfetchjoin_prefix(lng); break;
		}
		break;
	}
	MOSskip_prefix(cntxt,task);
	return MAL_SUCCEED;
}

#define join_prefix(TPE)\
{	TPE *v, *w;\
	v = (TPE*) (((char*) task->blk) + MosaicBlkSize);\
	w = (TPE*) task->src;\
	for(n = task->elm, o = 0; n -- > 0; w++,o++)\
	if ( *w == *v)\
		for(oo= (oid) first; oo < (oid) last; v++, oo++){\
			BUNappend(task->lbat, &oo, FALSE);\
			BUNappend(task->rbat, &o, FALSE);\
		}\
}

str
MOSjoin_prefix(Client cntxt,  MOStask task)
{
	BUN n,first,last;
	oid o, oo;
	(void) cntxt;

	// set the oid range covered and advance scan range
	first = task->start;
	last = first + MOSgetCnt(task->blk);

	switch(ATOMstorage(task->type)){
		case TYPE_bit: join_prefix(bit); break;
		case TYPE_bte: join_prefix(bte); break;
		case TYPE_sht: join_prefix(sht); break;
		case TYPE_lng: join_prefix(lng); break;
		case TYPE_oid: join_prefix(oid); break;
		case TYPE_wrd: join_prefix(wrd); break;
		case TYPE_flt: join_prefix(flt); break;
		case TYPE_dbl: join_prefix(dbl); break;
#ifdef HAVE_HGE
		case TYPE_hge: join_prefix(hge); break;
#endif
		case TYPE_int:
			{	int *v, *w;
				v = (int*) (((char*) task->blk) + MosaicBlkSize);
				w = (int*) task->src;
				for(n = task->elm, o = 0; n -- > 0; w++,o++)
				if ( *w == *v)
					for(oo= (oid) first; oo < (oid) last; v++, oo++){
						BUNappend(task->lbat, &oo, FALSE);
						BUNappend(task->rbat, &o, FALSE);
					}
			}
			break;
		case  TYPE_str:
		// we only have to look at the index width, not the values
		switch(task->b->T->width){
		case 1: break;
		case 2: break;
		case 4: break;
		case 8: break;
		}
			break;
	}
	MOSskip_prefix(cntxt,task);
	return MAL_SUCCEED;
}
