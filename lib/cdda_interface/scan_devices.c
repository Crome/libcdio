/*
  $Id: scan_devices.c,v 1.33 2008/06/16 19:45:44 flameeyes Exp $

  Copyright (C) 2004, 2005, 2007, 2008 Rocky Bernstein <rocky@gnu.org>
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

/******************************************************************
 * 
 * Autoscan for or verify presence of a CD-ROM device
 * 
 ******************************************************************/

#include "common_interface.h"
#include "low_interface.h"
#include "utils.h"
#include "cdio/mmc.h"
#include <limits.h>
#include <ctype.h>

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#define MAX_DEV_LEN 20 /* Safe because strings only come from below */
/* must be absolute paths! */
static const char scsi_cdrom_prefixes[][16]={
  "/dev/scd",
  "/dev/sr",
  ""};
static const char scsi_generic_prefixes[][16]={
  "/dev/sg",
  ""};

static const char devfs_scsi_test[]="/dev/scsi/";
static const char devfs_scsi_cd[]="cd";
static const char devfs_scsi_generic[]="generic";

static const char cdrom_devices[][32]={
  "/dev/cdrom",
  "/dev/cdroms/cdrom?",
  "/dev/hd?",
  "/dev/sg?",
  "/dev/cdu31a",
  "/dev/cdu535",
  "/dev/sbpcd",
  "/dev/sbpcd?",
  "/dev/sonycd",
  "/dev/mcd",
  "/dev/sjcd",
  /* "/dev/aztcd", timeout is too long */
  "/dev/cm206cd",
  "/dev/gscd",
  "/dev/optcd",
  ""};

static cdrom_drive_t *
cdda_identify_device_cdio(CdIo_t *p_cdio, const char *psz_device, 
			  int messagedest, char **ppsz_messages);

/* Functions here look for a cdrom drive; full init of a drive type
   happens in interface.c */

cdrom_drive_t *
cdio_cddap_find_a_cdrom(int messagedest, char **ppsz_messages){
  /* Brute force... */
  
  int i=0;
  cdrom_drive_t *d;

  while(*cdrom_devices[i]!='\0'){

    /* is it a name or a pattern? */
    char *pos;
    if((pos=strchr(cdrom_devices[i],'?'))){
      int j;
      /* try first eight of each device */
      for(j=0;j<4;j++){
	char *buffer=strdup(cdrom_devices[i]);

	/* number, then letter */
	
	buffer[pos-(cdrom_devices[i])]=j+48;
	if((d=cdda_identify(buffer, messagedest, ppsz_messages)))
	  return(d);
	idmessage(messagedest, ppsz_messages, "", NULL);
	buffer[pos-(cdrom_devices[i])]=j+97;
	if((d=cdda_identify(buffer, messagedest, ppsz_messages)))
	  return(d);
	idmessage(messagedest, ppsz_messages, "", NULL);
      }
    }else{
      /* Name.  Go for it. */
      if((d=cdda_identify(cdrom_devices[i], messagedest, ppsz_messages)))
	return(d);
      
      idmessage(messagedest, ppsz_messages, "", NULL);
    }
    i++;
  }
  {
#if defined(HAVE_GETPWUID) && defined(HAVE_GETEUID)
    struct passwd *temp;
    temp=getpwuid(geteuid());
    idmessage(messagedest, ppsz_messages,
	      "\n\nNo cdrom drives accessible to %s found.\n",
	      temp->pw_name);
#else
    idmessage(messagedest, ppsz_messages,
	      "\n\nNo cdrom drives accessible found.\n", NULL);
#endif
  }
  return(NULL);
}

#ifdef DEVICE_IN_FILESYSTEM
static char *
test_resolve_symlink(const char *file, int messagedest, char **ppsz_messages)
{
  char resolved[PATH_MAX];
  struct stat st;
  if (lstat(file,&st)){
    idperror(messagedest, ppsz_messages, "\t\tCould not stat %s",file);
    return(NULL);
  }

  if (realpath(file,resolved))
    return(strdup(resolved));

  idperror(messagedest, ppsz_messages, "\t\tCould not resolve symlink %s",
	   file);
  return(NULL);
}
#endif

/** Returns a paranoia CD-ROM drive object with a CD-DA in it or NULL
    if there was an error.
    @see cdio_cddap_identify_cdio
 */
cdrom_drive_t *
cdio_cddap_identify(const char *psz_dev, int messagedest,
char **ppsz_messages)
{
  CdIo_t *p_cdio = NULL;

  if (psz_dev) 
    idmessage(messagedest, ppsz_messages, "Checking %s for cdrom...", 
	      psz_dev);
  else 
    idmessage(messagedest, ppsz_messages, "Checking for cdrom...", NULL);

#ifdef DEVICE_IN_FILESYSTEM
  if (psz_dev) {
    char *psz_device = test_resolve_symlink(psz_dev, messagedest, 
					    ppsz_messages);
    if ( psz_device ) {
      cdrom_drive_t *d=NULL;
      p_cdio = cdio_open(psz_device, DRIVER_UNKNOWN);
      d = cdda_identify_device_cdio(p_cdio, psz_device, messagedest, 
				    ppsz_messages);
      free(psz_device);
      return d;
    }
  }
#endif

  p_cdio = cdio_open(psz_dev, DRIVER_UNKNOWN);
  if (p_cdio) {
    if (!psz_dev) {
      psz_dev = cdio_get_arg(p_cdio, "source");
    }
    return cdda_identify_device_cdio(p_cdio, psz_dev, messagedest, 
				     ppsz_messages);
  }
  return NULL;
}

/** Returns a paranoia CD-ROM drive object with a CD-DA in it or NULL
    if there was an error.  In contrast to cdio_cddap_identify, we
    start out with an initialized p_cdio object. For example you may
    have used that for other purposes such as to get CDDB/CD-Text
    information.  @see cdio_cddap_identify
 */
cdrom_drive_t *
cdio_cddap_identify_cdio(CdIo_t *p_cdio, int messagedest, char **ppsz_messages)
{
  if (!p_cdio) return NULL;
  { 
    const char *psz_device = cdio_get_arg(p_cdio, "source");
    idmessage(messagedest, ppsz_messages, "Checking %s for cdrom...", 
	      psz_device);
    return cdda_identify_device_cdio(p_cdio, psz_device, messagedest, 
				     ppsz_messages);
  }

}

static cdrom_drive_t *
cdda_identify_device_cdio(CdIo_t *p_cdio, const char *psz_device, 
			  int messagedest, char **ppsz_messages)
{
  cdrom_drive_t *d=NULL;
  int drive_type = 0;
  char *description=NULL;
#ifdef HAVE_LINUX_MAJOR_H
  struct stat st;
#endif

  if (!p_cdio) {
    idperror(messagedest, ppsz_messages, "\t\tUnable to open %s", psz_device);
    return NULL;
  }
  
#ifdef HAVE_LINUX_MAJOR_H
  if ( 0 == stat(psz_device, &st) ) {
    if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode)) {
      drive_type=(int)(st.st_rdev>>8);
      switch (drive_type) {
      case IDE0_MAJOR:
      case IDE1_MAJOR:
      case IDE2_MAJOR:
      case IDE3_MAJOR:
	/* Yay, ATAPI... */
	description=strdup("ATAPI compatible ");
	break;
      case CDU31A_CDROM_MAJOR:
	/* major indicates this is a cdrom; no ping necessary. */
	description=strdup("Sony CDU31A or compatible");
	break;
      case CDU535_CDROM_MAJOR:
	/* major indicates this is a cdrom; no ping necessary. */
	description=strdup("Sony CDU535 or compatible");
	break;
	
      case MATSUSHITA_CDROM_MAJOR:
      case MATSUSHITA_CDROM2_MAJOR:
      case MATSUSHITA_CDROM3_MAJOR:
      case MATSUSHITA_CDROM4_MAJOR:
	/* major indicates this is a cdrom; no ping necessary. */
	description=strdup("non-ATAPI IDE-style Matsushita/Panasonic CR-5xx or compatible");
	break;
      case SANYO_CDROM_MAJOR:
	description=strdup("Sanyo proprietary or compatible: NOT CDDA CAPABLE");
	break;
      case MITSUMI_CDROM_MAJOR:
      case MITSUMI_X_CDROM_MAJOR:
	description=strdup("Mitsumi proprietary or compatible: NOT CDDA CAPABLE");
	break;
      case OPTICS_CDROM_MAJOR:
	description=strdup("Optics Dolphin or compatible: NOT CDDA CAPABLE");
	break;
      case AZTECH_CDROM_MAJOR:
	description=strdup("Aztech proprietary or compatible: NOT CDDA CAPABLE");
	break;
      case GOLDSTAR_CDROM_MAJOR:
	description=strdup("Goldstar proprietary: NOT CDDA CAPABLE");
	break;
      case CM206_CDROM_MAJOR:
	description=strdup("Philips/LMS CM206 proprietary: NOT CDDA CAPABLE");
	break;
	
      case SCSI_CDROM_MAJOR:   
      case SCSI_GENERIC_MAJOR: 
	/* Nope nope nope */
	description=strdup("SCSI CD-ROM");
	break;
      default:
	/* What the hell is this? */
	idmessage(messagedest, ppsz_messages,
		  "\t\t%s is not a cooked ioctl CDROM.",
		  psz_device);
	return(NULL);
      }
    }
  }
#endif /*HAVE_LINUX_MAJOR_H*/

  /* Minimum init */
  
  d=calloc(1, sizeof(cdrom_drive_t));
  d->p_cdio           = p_cdio;
  d->cdda_device_name = strdup(psz_device);
  d->drive_type       = drive_type;
  d->bigendianp       = -1; /* We don't know yet... */
  d->nsectors         = -1; /* We don't know yet... */
  d->messagedest      = messagedest;
  d->b_swap_bytes     = true;
  
  {
    cdio_hwinfo_t hw_info = {
      "UNKNOWN", "Unknown model", "????"
    };

    if ( mmc_get_hwinfo( p_cdio, &hw_info ) ) {
      unsigned int i_len = strlen(hw_info.psz_vendor) 
	+ strlen(hw_info.psz_model) 
	+ strlen(hw_info.psz_revision) + 5;
      
      if (description) {
	i_len += strlen(description);
	d->drive_model=malloc( i_len );
	snprintf( d->drive_model, i_len, "%s %s %s %s", 
		  hw_info.psz_vendor, hw_info.psz_model, hw_info.psz_revision,
		  description );
	free(description);
      } else {
	d->drive_model=malloc( i_len );
	snprintf( d->drive_model, i_len, "%s %s %s", 
		  hw_info.psz_vendor, hw_info.psz_model, hw_info.psz_revision 
		  );
      }
      idmessage(messagedest, ppsz_messages, "\t\tCDROM sensed: %s\n", 
		d->drive_model);
    }
  }
  
  return(d);
}
