/*
  $Id: tracks.c,v 1.7 2008/03/24 15:30:56 karl Exp $

  Copyright (C) 2003, 2004, 2005, 2006, 2008 Rocky Bernstein <rocky@gnu.org>
  
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

/* Simple program to list track numbers and logical sector numbers of
   a Compact Disc using libcdio. */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <cdio/cdio.h>
int
main(int argc, const char *argv[])
{
  CdIo_t *p_cdio = cdio_open (NULL, DRIVER_UNKNOWN);
  track_t i_first_track;
  track_t i_tracks;
  int j, i;


  if (NULL == p_cdio) {
    printf("Couldn't find a driver.. leaving.\n");
    return 1;
  }
  
  printf("Disc last LSN: %d\n", cdio_get_disc_last_lsn(p_cdio));

  i_tracks      = cdio_get_num_tracks(p_cdio);
  i_first_track = i = cdio_get_first_track_num(p_cdio);

  printf("CD-ROM Track List (%i - %i)\n", i_first_track, 
	 i_first_track+i_tracks-1);

  printf("  #:  LSN\n");
  
  for (j = 0; j < i_tracks; i++, j++) {
    lsn_t lsn = cdio_get_track_lsn(p_cdio, i);
    if (CDIO_INVALID_LSN != lsn)
	printf("%3d: %06lu\n", (int) i, (long unsigned int) lsn);
  }
  printf("%3X: %06lu  leadout\n", CDIO_CDROM_LEADOUT_TRACK, 
       (long unsigned int) cdio_get_track_lsn(p_cdio, 
					      CDIO_CDROM_LEADOUT_TRACK));
  cdio_destroy(p_cdio);
  return 0;
}
