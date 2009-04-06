/*
    $Id: cdtext.h,v 1.14 2008/03/25 15:59:08 karl Exp $

    Copyright (C) 2004, 2005, 2008 Rocky Bernstein <rocky@gnu.org>
    adapted from cuetools
    Copyright (C) 2003 Svend Sanjay Sorensen <ssorensen@fastmail.fm>

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
/*!
 * \file cdtext.h 
 *
 * \brief The top-level header for CD-Text information. Applications
 *  include this for CD-Text access.
*/


#ifndef __CDIO_CDTEXT_H__
#define __CDIO_CDTEXT_H__

#include <cdio/cdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MAX_CDTEXT_FIELDS 13
#define MIN_CDTEXT_FIELD  0
  
  /*! \brief structure for holding CD-Text information

  @see cdtext_init, cdtext_destroy, cdtext_get, and cdtext_set.
  */
  struct cdtext {
    char *field[MAX_CDTEXT_FIELDS];
  };
  
  /*! \brief A list of all of the CD-Text fields. Because
    the interval has no gaps, we can use ++ to iterate over fields.
   */
  typedef enum {
    CDTEXT_ARRANGER   =  0,   /**< name(s) of the arranger(s) */
    CDTEXT_COMPOSER   =  1,   /**< name(s) of the composer(s) */
    CDTEXT_DISCID     =  2,   /**< disc identification information */
    CDTEXT_GENRE      =  3,   /**< genre identification and genre information */
    CDTEXT_MESSAGE    =  4,   /**< ISRC code of each track */
    CDTEXT_ISRC       =  5,   /**< message(s) from the content provider or artist */
    CDTEXT_PERFORMER  =  6,   /**< name(s) of the performer(s) */
    CDTEXT_SIZE_INFO  =  7,   /**< size information of the block */
    CDTEXT_SONGWRITER =  8,   /**< name(s) of the songwriter(s) */
    CDTEXT_TITLE      =  9,   /**< title of album name or track titles */
    CDTEXT_TOC_INFO   = 10,   /**< table of contents information */
    CDTEXT_TOC_INFO2  = 11,   /**< second table of contents information */
    CDTEXT_UPC_EAN    = 12,
    CDTEXT_INVALID    = MAX_CDTEXT_FIELDS
  } cdtext_field_t;

  /*! Return string representation of the enum values above */
  const char *cdtext_field2str (cdtext_field_t i);
  
  /*! Initialize a new cdtext structure.
    When the structure is no longer needed, release the 
    resources using cdtext_delete.
  */
  void cdtext_init (cdtext_t *cdtext);
  
  /*! Free memory assocated with cdtext*/
  void cdtext_destroy (cdtext_t *cdtext);
  
  /*! returns an allocated string associated with the given field.  NULL is
    returned if key is CDTEXT_INVALID or the field is not set.

    The user needs to free the string when done with it.

    @see cdio_get_const to retrieve a constant string that doesn't
    have to be freed.
  */
  char *cdtext_get (cdtext_field_t key, const cdtext_t *cdtext);

  /*! returns a const string associated with the given field.  NULL is
    returned if key is CDTEXT_INVALID or the field is not set.

    Don't use the string when the cdtext object (i.e. the CdIo_t object
    you got it from) is no longer valid.

    @see cdio_get to retrieve an allocated string that persists past
    the cdtext object.
  */
  const char *cdtext_get_const (cdtext_field_t key, const cdtext_t *cdtext);
  
  /*!
    returns enum of keyword if key is a CD-Text keyword, 
    returns MAX_CDTEXT_FIELDS non-zero otherwise.
  */
  cdtext_field_t cdtext_is_keyword (const char *key);
  
  /*! 
    sets cdtext's keyword entry to field 
  */
  void cdtext_set (cdtext_field_t key, const char *value, cdtext_t *cdtext);
  
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CDIO_CDTEXT_H__ */

/* 
 * Local variables:
 *  c-file-style: "gnu"
 *  tab-width: 8
 *  indent-tabs-mode: nil
 * End:
 */
