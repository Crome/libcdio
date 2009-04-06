/*
  $Id: _cdio_stream.h,v 1.5 2008/04/22 15:29:11 karl Exp $

  Copyright (C) 2003, 2004, 2005, 2006, 2008 Rocky Bernstein <rocky@gnu.org>
  Copyright (C) 2000 Herbert Valerio Riedel <hvr@gnu.org>

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


#ifndef __CDIO_STREAM_H__
#define __CDIO_STREAM_H__

#include <cdio/types.h>
#include "cdio_private.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  /* typedef'ed IO functions prototypes */
  
  typedef int(*cdio_data_open_t)(void *user_data);
  
  typedef long(*cdio_data_read_t)(void *user_data, void *buf, long count);
  
  typedef driver_return_code_t(*cdio_data_seek_t)(void *user_data, long offset,
                                                  int whence);
  
  typedef long(*cdio_data_stat_t)(void *user_data);
  
  typedef int(*cdio_data_close_t)(void *user_data);
  
  typedef void(*cdio_data_free_t)(void *user_data);
  
  
  /* abstract data source */
  
  typedef struct {
    cdio_data_open_t open;
    cdio_data_seek_t seek; 
    cdio_data_stat_t stat; 
    cdio_data_read_t read;
    cdio_data_close_t close;
    cdio_data_free_t free;
  } cdio_stream_io_functions;
  
  /**
     Like 3 fgetpos.
     
     This function gets the current file position indicator for the stream
     pointed to by stream.  
     
     @return unpon successful completion, return value is positive, else,
     the global variable errno is set to indicate the error.
  */
  ssize_t cdio_stream_getpos(CdioDataSource_t* p_obj, 
                             /*out*/ ssize_t *i_offset);
  
  CdioDataSource_t *
  cdio_stream_new(void *user_data, const cdio_stream_io_functions *funcs);

  /**
     Like fread(3) and in fact may be the same.

     DESCRIPTION:
     The function fread reads nmemb elements of data, each size bytes long,
     from the stream pointed to by stream, storing them at the location
     given by ptr.

     RETURN VALUE:
     return the number of items successfully read or written (i.e.,
     not the number of characters).  If an error occurs, or the
     end-of-file is reached, the return value is a short item count
     (or zero).

     We do not distinguish between end-of-file and error, and callers
     must use feof(3) and ferror(3) to determine which occurred.
  */
  ssize_t cdio_stream_read(CdioDataSource_t* p_obj, void *ptr, long i_size, 
                           long nmemb);
  
  /** 
    Like fseek(3) and in fact may be the same.

    This  function sets the file position indicator for the stream
    pointed to by stream.  The new position, measured in bytes, is obtained
    by  adding offset bytes to the position specified by whence.  If whence
    is set to SEEK_SET, SEEK_CUR, or SEEK_END, the offset  is  relative  to
    the  start of the file, the current position indicator, or end-of-file,
    respectively.  A successful call to the fseek function clears the end-
    of-file indicator for the stream and undoes any effects of the
    ungetc(3) function on the same stream.
    
    @return upon successful completion, DRIVER_OP_SUCCESS, else,
    DRIVER_OP_ERROR is returned and the global variable errno is set to
    indicate the error.
   */
  ssize_t cdio_stream_seek(CdioDataSource_t *p_obj, ssize_t i_offset, 
                           int whence);
  
  /**
    Return whatever size of stream reports, I guess unit size is bytes. 
    On error return -1;
  */
  ssize_t cdio_stream_stat(CdioDataSource_t *p_obj);
  
  /**
    Deallocate resources associated with p_obj. After this p_obj is unusable.
  */
  void cdio_stream_destroy(CdioDataSource_t *p_obj);
  
  void cdio_stream_close(CdioDataSource_t *p_obj);
  
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CDIO_STREAM_H__ */


/* 
 * Local variables:
 *  c-file-style: "gnu"
 *  tab-width: 8
 *  indent-tabs-mode: nil
 * End:
 */
