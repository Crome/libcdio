/*
  $Id: testparanoia.c,v 1.14 2008/06/19 15:44:31 flameeyes Exp $

  Copyright (C) 2005, 2006, 2008 Rocky Bernstein <rocky@gnu.org>

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

/* Simple program to show using libcdio's version of cdparanoia. */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cdio/paranoia.h>
#include <cdio/cd_types.h>
#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef WORDS_BIGENDIAN
#define BIGENDIAN 1
#else
#define BIGENDIAN 0
#endif
 
#define SKIP_TEST_RC 77

#define MAX_SECTORS 50
static uint8_t audio_buf[MAX_SECTORS][CDIO_CD_FRAMESIZE_RAW] = { {0}, };
static paranoia_cb_mode_t audio_status[MAX_SECTORS];
static unsigned int i = 0;

static void 
callback(long int inpos, paranoia_cb_mode_t function)
{
  audio_status[i] = function;
}

int
main(int argc, const char *argv[])
{
  cdrom_drive_t *d = NULL; /* Place to store handle given by cd-parapnioa. */
  driver_id_t driver_id;
  char **ppsz_cd_drives;  /* List of all drives with a loaded CDDA in it. */
  int i_rc=0;

  /* See if we can find a device with a loaded CD-DA in it. If successful
     drive_id will be set.  */
  ppsz_cd_drives = cdio_get_devices_with_cap_ret(NULL, CDIO_FS_AUDIO, false,
						 &driver_id);

  if (ppsz_cd_drives && *ppsz_cd_drives) {
    /* Found such a CD-ROM with a CD-DA loaded. Use the first drive in
       the list. */
    d=cdda_identify(*ppsz_cd_drives, 1, NULL);
  } else {
    printf("Unable find or access a CD-ROM drive with an audio CD in it.\n");
    exit(SKIP_TEST_RC);
  }

  /** We had a bug in is_device when driver_id == DRIVER_UNKNOWN or
     DRIVER_DEVICE. Let's make sure we've fixed that problem. **/
  if (!cdio_is_device(*ppsz_cd_drives, DRIVER_UNKNOWN) || 
      !cdio_is_device(*ppsz_cd_drives, DRIVER_DEVICE))
    exit(99);
  
  /* Don't need a list of CD's with CD-DA's any more. */
  cdio_free_device_list(ppsz_cd_drives);

  /* We'll set for verbose paranoia messages. */
  cdda_verbose_set(d, CDDA_MESSAGE_PRINTIT, CDDA_MESSAGE_PRINTIT);

  if ( 0 != cdio_cddap_open(d) ) {
    printf("Unable to open disc.\n");
    exit(SKIP_TEST_RC);
  }

  /* Okay now set up to read up to the first 300 frames of the first
     audio track of the Audio CD. */
  { 
    cdrom_paranoia_t *p = paranoia_init(d);
    lsn_t i_first_lsn = cdda_disc_firstsector(d);

    if ( -1 == i_first_lsn ) {
      printf("Trouble getting starting LSN\n");
    } else {
      lsn_t   i_lsn; /* Current LSN to read */
      lsn_t   i_last_lsn = cdda_disc_lastsector(d);
      unsigned int i_sectors = i_last_lsn - i_first_lsn + 1;
      unsigned int j;
      unsigned int i_good = 0;
      unsigned int i_bad  = 0;
      
      /* Set reading mode for full paranoia, but allow skipping sectors. */
      paranoia_modeset(p, PARANOIA_MODE_FULL^PARANOIA_MODE_NEVERSKIP);

      for ( j=0; j<10; j++ ) {
	
	/* Pick a place to start reading. */
	i_lsn = i_first_lsn + (rand() % i_sectors);
	paranoia_seek(p, i_lsn, SEEK_SET);

	printf("Testing %d sectors starting at %ld\n",
	       MAX_SECTORS, (long int) i_lsn);
	for ( i = 0; 
	      i < MAX_SECTORS && i_lsn <= i_last_lsn; 
	      i++, i_lsn++ ) {
	  /* read a sector */
	  int16_t *p_readbuf = paranoia_read(p, callback);
	  char *psz_err=cdio_cddap_errors(d);
	  char *psz_mes=cdio_cddap_messages(d);

	  memcpy(audio_buf[i], p_readbuf, CDIO_CD_FRAMESIZE_RAW);
	  
	  if (psz_mes || psz_err)
	    printf("%s%s\n", psz_mes ? psz_mes: "", psz_err ? psz_err: "");
	  
	  if (psz_err) free(psz_err);
	  if (psz_mes) free(psz_mes);
	  if( !p_readbuf ) {
	    printf("paranoia read error. Stopping.\n");
	    goto out;
	  }
	}

	/* Compare with the sectors from paranoia. */
	i_lsn -= MAX_SECTORS;
	for ( i = 0; i < MAX_SECTORS; i++, i_lsn++ ) {
	  uint8_t readbuf[CDIO_CD_FRAMESIZE_RAW] = {0,};
	  if ( PARANOIA_CB_READ == audio_status[i] || 
	       PARANOIA_CB_VERIFY == audio_status[i] ) {
	    /* We read the block via paranoia without an error. */

	    if ( 0 == cdio_read_audio_sector(d->p_cdio, readbuf, i_lsn) ) {
	      if ( BIGENDIAN != d->bigendianp ) {
		/* We will compare in the slow, pedantic way*/
		int j;
		for (j=0; j < CDIO_CD_FRAMESIZE_RAW ; j +=2) {
		  if (audio_buf[i][j]   != readbuf[j+1] && 
		      audio_buf[i][j+1] != readbuf[j] ) {
		    printf("LSN %ld doesn't match\n", (long int) i_lsn);
		    i_bad++;
		  } else {
		    i_good++;
		  }
		}
	      } else {
		if ( 0 != memcmp(audio_buf[i], readbuf, 
				 CDIO_CD_FRAMESIZE_RAW) ) {
		  printf("LSN %ld doesn't match\n", (long int) i_lsn);
		  i_bad++;
		} else {
		  i_good++;
		}
	      }
	    }
	  } else {
	    printf("Skipping LSN %ld because of status: %s\n", 
		   (long int) i_lsn, paranoia_cb_mode2str[audio_status[i]]);
	  }
	}
      }
      printf("%u sectors compared okay %u sectors were different\n",
	     i_good, i_bad);
      if (i_bad > i_good) i_rc = 1;
    }
  out: paranoia_free(p);
  }
  
  cdio_cddap_close(d);

  exit(i_rc);
}

