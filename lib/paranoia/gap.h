/*
  $Id: gap.h,v 1.2 2008/04/17 17:39:48 karl Exp $

  Copyright (C) 2004, 2008 Rocky Bernstein <rocky@gnu.org>
  Copyright (C) 1998 Monty xiphmont@mit.edu
  
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _GAP_H_
#define _GAP_H_

extern long i_paranoia_overlap_r(int16_t *buffA,int16_t *buffB,
				 long offsetA, long offsetB);
extern long i_paranoia_overlap_f(int16_t *buffA,int16_t *buffB,
				 long offsetA, long offsetB,
				 long sizeA,long sizeB);
extern int i_stutter_or_gap(int16_t *A, int16_t *B,long offA, long offB,
			    long gap);
extern void i_analyze_rift_f(int16_t *A,int16_t *B,
			     long sizeA, long sizeB,
			     long aoffset, long boffset, 
			     long *matchA,long *matchB,long *matchC);
extern void i_analyze_rift_r(int16_t *A,int16_t *B,
			     long sizeA, long sizeB,
			     long aoffset, long boffset, 
			     long *matchA,long *matchB,long *matchC);

extern void analyze_rift_silence_f(int16_t *A,int16_t *B,long sizeA,long sizeB,
				   long aoffset, long boffset,
				   long *matchA, long *matchB);
#endif /*_GAP_H*/
