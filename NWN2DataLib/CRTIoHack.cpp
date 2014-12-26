/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	CRTIoHack.cpp

Abstract:

	This module houses CRT no-lock I/O routines for use with msvcrt.dll.

--*/

#include "Precomp.h"


size_t __cdecl crtfread_nolock(
    void *buffer,
    size_t elementSize,
    size_t count,
    FILE *stream);

size_t __cdecl crtfwrite_nolock (
        const void *buffer,
        size_t size,
        size_t num,
        FILE *stream);

long __cdecl crtftell_nolock (


        FILE *str
        );

int __cdecl crtfseek_nolock (


        FILE *str,
        long offset,
        int whence
        );




/* Additional _iobuf[]._flag values
 *
 *  _IOSETVBUF - Indicates file was buffered via a setvbuf (or setbuf call).
 *               Currently used ONLY in _filbuf.c, _getbuf.c, fseek.c and
 *               setvbuf.c, to disable buffer resizing on "random access"
 *               files if the buffer was user-installed.
 *
 *  _IOLOCKED  - Indicate that the stream is locked.
 *
 */

#define _IOYOURBUF      0x0100
#define _IOSETVBUF      0x0400
#define _IOFEOF         0x0800
#define _IOFLRTN        0x1000
#define _IOCTRLZ        0x2000
#define _IOCOMMIT       0x4000
#define _IOLOCKED       0x8000

/* General use macros */

#define inuse(s)        ((s)->_flag & (_IOREAD|_IOWRT|_IORW))
#define mbuf(s)         ((s)->_flag & _IOMYBUF)
#define nbuf(s)         ((s)->_flag & _IONBF)
#define ybuf(s)         ((s)->_flag & _IOYOURBUF)
#define bigbuf(s)       ((s)->_flag & (_IOMYBUF|_IOYOURBUF))
#define anybuf(s)       ((s)->_flag & (_IOMYBUF|_IONBF|_IOYOURBUF))
#define str_locked(s)   ((s)->_flag & (_IOLOCKED))


extern __declspec(dllimport) ioinfo * __pioinfo[];

#define _pioinfo(i) ( __pioinfo[(i) >> IOINFO_L2E] + ((i) & (IOINFO_ARRAY_ELTS - \
                              1)) )


#define _osfile(i)  ( _pioinfo(i)->osfile )

#define _BUFFER_FILL_PATTERN 0

/*
 * Real default size for stdio buffers
 */
#define _INTERNAL_BUFSIZ    4096
#define _SMALL_BUFSIZ       512


#define _CALL_INVALID_PARAMETER_FUNC(funcname, expr) funcname(NULL, NULL, NULL, 0, 0)
#define _INVALID_PARAMETER(expr) _CALL_INVALID_PARAMETER_FUNC(_invalid_parameter, expr)
#define _VALIDATE_RETURN_VOID( expr, errorcode )                               \
    {                                                                          \
        int _Expr_val=!!(expr);                                                \
        _ASSERT_EXPR( ( _Expr_val ), _CRT_WIDE(#expr) );                       \
        if ( !( _Expr_val ) )                                                  \
        {                                                                      \
            errno = errorcode;                                                 \
            _INVALID_PARAMETER(_CRT_WIDE(#expr));                              \
            return;                                                            \
        }                                                                      \
    }

#ifndef _VALIDATE_RETURN
#define _VALIDATE_RETURN( expr, errorcode, retexpr )                           \
    {                                                                          \
        int _Expr_val=!!(expr);                                                \
        _ASSERT_EXPR( ( _Expr_val ), _CRT_WIDE(#expr) );                       \
        if ( !( _Expr_val ) )                                                  \
        {                                                                      \
            errno = errorcode;                                                 \
            _INVALID_PARAMETER(_CRT_WIDE(#expr) );                             \
            return ( retexpr );                                                \
        }                                                                      \
    }
#endif  /* _VALIDATE_RETURN */



/* define the normal version */
size_t __cdecl crtfread_nolock_s(
    void *buffer,
    size_t bufferSize,
    size_t elementSize,
    size_t num,
    FILE *stream
)
{
    char *data;                     /* point inside the destination buffer to where we need to copy the read chars */
    size_t dataSize;                /* space left in the destionation buffer (in bytes) */
    size_t total;                   /* total bytes to read */
    size_t count;                   /* num bytes left to read */
    unsigned streambufsize;         /* size of stream buffer */
    unsigned nbytes;                /* how much to read now */
    unsigned nread;                 /* how much we did read */
    int c;                          /* a temp char */

    /* initialize local vars */
    data = (char*) buffer;
    dataSize = bufferSize;

    if (elementSize == 0 || num == 0)
    {
        return 0;
    }

    /* validation */
    _VALIDATE_RETURN((buffer != NULL), EINVAL, 0);
    if (stream == NULL || num > (SIZE_MAX / elementSize))
    {
        if (bufferSize != SIZE_MAX)
        {
            memset(buffer, _BUFFER_FILL_PATTERN, bufferSize);
        }

        _VALIDATE_RETURN((stream != NULL), EINVAL, 0);
        _VALIDATE_RETURN(num <= (SIZE_MAX / elementSize), EINVAL, 0);
    }


    count = total = elementSize * num;

    if (anybuf(stream))
    {
        /* already has buffer, use its size */
        streambufsize = stream->_bufsiz;
    }
    else
    {
        /* assume will get _INTERNAL_BUFSIZ buffer */
        streambufsize = _INTERNAL_BUFSIZ;
    }

    /* here is the main loop -- we go through here until we're done */
    while (count != 0) {
        /* if the buffer exists and has characters, copy them to user
            buffer */
        if (anybuf(stream) && stream->_cnt != 0)
        {
            if(stream->_cnt < 0)
            {
                _ASSERTE(("Inconsistent Stream Count. Flush between consecutive read and write", stream->_cnt >= 0));
                stream->_flag |= _IOERR;
                return (total - count) / elementSize;
            }

            /* how much do we want? */
            nbytes = (count < (size_t)stream->_cnt) ? (unsigned)count : stream->_cnt;
            if (nbytes > dataSize)
            {
                if (bufferSize != SIZE_MAX)
                {
                    memset(buffer, _BUFFER_FILL_PATTERN, bufferSize);
                }
                _VALIDATE_RETURN(("buffer too small", 0), ERANGE, 0)
            }
            memcpy_s(data, dataSize, stream->_ptr, nbytes);

            /* update stream and amt of data read */
            count -= nbytes;
            stream->_cnt -= nbytes;
            stream->_ptr += nbytes;
            data += nbytes;
            dataSize -= nbytes;
        }
        else if (count >= streambufsize)
        {
            /* If we have more than streambufsize chars to read, get data
                by calling read with an integral number of bufsiz
                blocks.  Note that if the stream is text mode, read
                will return less chars than we ordered. */

            if (streambufsize)
            {
                /* In 64bit apps size_t is bigger than unsigned
                 * (which is 32bit even in 64 bit machines), so
                 * we need to split the read into INT_MAX chunks
                 * since _read() only support up to _signed_ int
                 * (even though the in parameter is unsigned).
                 */
                if (count > INT_MAX)
                {
                    /* calc chars to read -- the largest multiple of streambufsize
                     * smaller then INT_MAX
                     */
                    nbytes = (unsigned)(INT_MAX - INT_MAX % streambufsize);
                }
                else
                {
                    /* calc chars to read -- (count/streambufsize) * streambufsize */
                    nbytes = (unsigned)(count - count % streambufsize);
                }
            }
            else
            {
                nbytes = (count > INT_MAX)?(unsigned)INT_MAX: (unsigned)count;
            }

            if (nbytes > dataSize)
            {
                if (bufferSize != SIZE_MAX)
                {
                    memset(buffer, _BUFFER_FILL_PATTERN, bufferSize);
                }
                _VALIDATE_RETURN(("buffer too small", 0), ERANGE, 0)
            }

            nread = _read(_fileno(stream), data, nbytes);
            if (nread == 0) {
                    /* end of file -- out of here */
                    stream->_flag |= _IOEOF;
                    return (total - count) / elementSize;
            }
            else if (nread == (unsigned)-1) {
                    /* error -- out of here */
                    stream->_flag |= _IOERR;
                    return (total - count) / elementSize;
            }

            /* update count and data to reflect read */
            count -= nread;
            data += nread;
            dataSize -= nread;
        }
        else
        {
            /* less than streambufsize chars to read, so call _filbuf to
                fill buffer */
            if ((c = _filbuf(stream)) == EOF) {
                    /* error or eof, stream flags set by _filbuf */
                    return (total - count) / elementSize;
            }

            /* _filbuf returned a char -- store it */
            if (dataSize == 0)
            {
                if (bufferSize != SIZE_MAX)
                {
                    memset(buffer, _BUFFER_FILL_PATTERN, bufferSize);
                }
                _VALIDATE_RETURN(("buffer too small", 0), ERANGE, 0)
            }
            *data++ = (char) c;
            --count;
            --dataSize;

            /* update buffer size */
            streambufsize = stream->_bufsiz;
        }
    }

    /* we finished successfully, so just return num */
    return num;
}



#ifdef _M_IX86
/*
 * 386/486
 */
#define REG1    register
#define REG2    register
#define REG3    register
#define REG4
#define REG5
#define REG6
#define REG7
#define REG8
#define REG9

#elif defined (_M_IA64) || defined (_M_AMD64)
/*
 * IA64
 */
#define REG1    register
#define REG2    register
#define REG3    register
#define REG4    register
#define REG5    register
#define REG6    register
#define REG7    register
#define REG8    register
#define REG9    register

#else  /* defined (_M_IA64) || defined (_M_AMD64) */

#pragma message ("Machine register set not defined")

/*
 * Unknown machine
 */

#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#define REG7
#define REG8
#define REG9

#endif  /* defined (_M_IA64) || defined (_M_AMD64) */



/* __osfile flag values for DOS file handles */

#define FOPEN           0x01    /* file handle open */
#define FEOFLAG         0x02    /* end of file has been encountered */
#define FCRLF           0x04    /* CR-LF across read buffer (in text mode) */
#define FPIPE           0x08    /* file handle refers to a pipe */
#define FNOINHERIT      0x10    /* file handle opened _O_NOINHERIT */
#define FAPPEND         0x20    /* file handle opened O_APPEND */
#define FDEV            0x40    /* file handle refers to device */
#define FTEXT           0x80    /* file handle is in text mode */



/***
*int _flush(stream) - flush the buffer on a single stream
*
*Purpose:
*       If file open for writing and buffered, flush the buffer.  If
*       problems flushing the buffer, set the stream flag to error.
*       Multi-thread version assumes stream lock is held by caller.
*
*Entry:
*       FILE* stream - stream to flush
*
*Exit:
*       Returns 0 if flushed successfully, or if no buffer to flush.,
*       Returns EOF and sets file error flag if fails.
*       File struct entries affected: _ptr, _cnt, _flag.
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _flush (
        FILE *str
        )
{
        REG1 FILE *stream;
        REG2 int rc = 0; /* assume good return */
        REG3 int nchar;

        /* Init pointer to stream */
        stream = str;


        if ((stream->_flag & (_IOREAD | _IOWRT)) == _IOWRT && bigbuf(stream)
                && (nchar = (int)(stream->_ptr - stream->_base)) > 0)
        {
                if ( _write(_fileno(stream), stream->_base, nchar) == nchar ) {
                        /* if this is a read/write file, clear _IOWRT so that
                         * next operation can be a read
                         */
                        if ( _IORW & stream->_flag )
                                stream->_flag &= ~_IOWRT;
                }
                else {
                        stream->_flag |= _IOERR;
                        rc = EOF;
                }
        }

        stream->_ptr = stream->_base;
        stream->_cnt = 0;

        return(rc);
}


size_t __cdecl crtfread_nolock(
    void *buffer,
    size_t elementSize,
    size_t count,
    FILE *stream
)
{
    /* assumes there is enough space in the destination buffer */
    return crtfread_nolock_s(buffer, SIZE_MAX, elementSize, count, stream);
}



/* define the normal version */
size_t __cdecl crtfwrite_nolock (
        const void *buffer,
        size_t size,
        size_t num,
        FILE *stream
        )
{
        const char *data;               /* point to where data comes from next */
        size_t total;                   /* total bytes to write */
        size_t count;                   /* num bytes left to write */
        unsigned bufsize;               /* size of stream buffer */
        unsigned nbytes;                /* number of bytes to write now */
        unsigned nactuallywritten;      /* number of bytes actually written by write() call*/
        unsigned nwritten;              /* number of bytes we consider written */
        int c;                          /* a temp char */

        /* initialize local vars */
        if (size ==0 || num ==0)
            return 0;

        _VALIDATE_RETURN( (stream != NULL) ,EINVAL, 0);
        _VALIDATE_RETURN( (buffer != NULL) ,EINVAL, 0);
        _VALIDATE_RETURN(num <= (SIZE_MAX / size), EINVAL, 0);

        data = (const char *) buffer;
        count = total = size * num;

        if (anybuf(stream))
                /* already has buffer, use its size */
                bufsize = stream->_bufsiz;
        else
                /* assume will get _INTERNAL_BUFSIZ buffer */
                bufsize = _INTERNAL_BUFSIZ;

        /* here is the main loop -- we go through here until we're done */
        while (count != 0) {
                /* if the buffer is big and has room, copy data to buffer */
                if (bigbuf(stream) && stream->_cnt != 0) {
                         if(stream->_cnt < 0) {
                            _ASSERTE(("Inconsistent Stream Count. Flush between consecutive read and write", stream->_cnt >= 0));
                            stream->_flag |= _IOERR;
                            return (total - count) / size;
                        }

                        /* how much do we want? */
                        nbytes = (count < (unsigned)stream->_cnt) ? (unsigned)count : stream->_cnt;
                        memcpy(stream->_ptr, data, nbytes);

                        /* update stream and amt of data written */
                        count -= nbytes;
                        stream->_cnt -= nbytes;
                        stream->_ptr += nbytes;
                        data += nbytes;
                }
                else if (count >= bufsize) {
                        /* If we have more than bufsize chars to write, write
                           data by calling write with an integral number of
                           bufsiz blocks.  If we reach here and we have a big
                           buffer, it must be full so _flush it. */

                        if (bigbuf(stream)) {
                                if (_flush(stream)) {
                                        /* error, stream flags set -- we're out
                                           of here */
                                        return (total - count) / size;
                                }
                        }

                        /* calc chars to read -- (count/bufsize) * bufsize */
                        nbytes = ( bufsize ? (unsigned)(count - count % bufsize) :
                                   (unsigned)count );

                        nactuallywritten = _write(_fileno(stream), data, nbytes);
                        if (nactuallywritten == (unsigned)EOF) {
                                /* error -- out of here */
                                stream->_flag |= _IOERR;
                                return (total - count) / size;
                        }

                        /* VSWhidbey#326224 - _write can return more bytes than we requested due unicode
                           involved conversions in text files.
                           But we do not care about how many bytes were written as long as the number is
                           at least as big as we wanted.
                        */
                        nwritten = (nactuallywritten > nbytes ? nbytes : nactuallywritten);

                        /* update count and data to reflect write */

                        count -= nwritten;
                        data += nwritten;

                        if (nactuallywritten < nbytes) {
                                /* error -- out of here */
                                stream->_flag |= _IOERR;
                                return (total - count) / size;
                        }
                }
                else {
                        /* buffer full and not enough chars to do direct write,
                           so do a _flsbuf. */
                        c = *data;  /* _flsbuf write one char, this is it */
                        if (_flsbuf(c, stream) == EOF) {
                                /* error or eof, stream flags set by _flsbuf */
                                return (total - count) / size;
                        }

                        /* _flsbuf wrote a char -- update count */
                        ++data;
                        --count;

                        /* update buffer size */
                        bufsize = stream->_bufsiz > 0 ? stream->_bufsiz : 1;
                }
        }

        /* we finished successfully, so just return num */
        return num;
}



/***
*_ftell_nolock() - Ftell() core routine (assumes stream is locked).
*
*Purpose:
*       Core ftell() routine; assumes caller has aquired stream lock).
*
*       [See ftell() above for more info.]
*
*Entry: [See ftell()]
*
*Exit:  [See ftell()]
*
*Exceptions:
*
*******************************************************************************/

long __cdecl crtftell_nolock (


        FILE *str
        )
{
        REG1 FILE *stream;
        unsigned int offset;
        long filepos;
        REG2 char *p;
        char *max;
        int fd;
        unsigned int rdcnt;

        _VALIDATE_RETURN( (str != NULL), EINVAL, (-1L) );

        /* Init stream pointer and file descriptor */
        stream = str;
        fd = _fileno(stream);

        if (stream->_cnt < 0)
            stream->_cnt = 0;

        if ((filepos = _lseek(fd, 0L, SEEK_CUR)) < 0L)
            return(-1L);

        if (!bigbuf(stream))            /* _IONBF or no buffering designated */
            return(filepos - stream->_cnt);

        offset = (unsigned)(stream->_ptr - stream->_base);

        if (stream->_flag & (_IOWRT|_IOREAD)) {
            if (_osfile(fd) & FTEXT)
                for (p = stream->_base; p < stream->_ptr; p++)
                    if (*p == '\n')  /* adjust for '\r' */
                        offset++;
        }
        else if (!(stream->_flag & _IORW)) {
            errno=EINVAL;
            return(-1L);
        }

        if (filepos == 0L)
            return((long)offset);

        if (stream->_flag & _IOREAD)    /* go to preceding sector */

            if (stream->_cnt == 0)  /* filepos holds correct location */
                offset = 0;

            else {

                /* Subtract out the number of unread bytes left in the buffer.
                   [We can't simply use _iob[]._bufsiz because the last read
                   may have hit EOF and, thus, the buffer was not completely
                   filled.] */

                rdcnt = stream->_cnt + (unsigned)(stream->_ptr - stream->_base);

                /* If text mode, adjust for the cr/lf substitution. If binary
                   mode, we're outta here. */
                if (_osfile(fd) & FTEXT) {
                    /* (1) If we're not at eof, simply copy _bufsiz onto rdcnt
                       to get the # of untranslated chars read. (2) If we're at
                       eof, we must look through the buffer expanding the '\n'
                       chars one at a time. */

                    /* [NOTE: Performance issue -- it is faster to do the two
                       _lseek() calls than to blindly go through and expand the
                       '\n' chars regardless of whether we're at eof or not.] */

                    if (_lseek(fd, 0L, SEEK_END) == filepos) {

                        max = stream->_base + rdcnt;
                        for (p = stream->_base; p < max; p++)
                            if (*p == '\n')
                                /* adjust for '\r' */
                                rdcnt++;

                        /* If last byte was ^Z, the lowio read didn't tell us
                           about it. Check flag and bump count, if necessary. */

                        if (stream->_flag & _IOCTRLZ)
                            ++rdcnt;
                    }

                    else {

                        if (_lseek(fd, filepos, SEEK_SET) < 0)
                            return (-1);

                        /* We want to set rdcnt to the number of bytes
                           originally read into the stream buffer (before
                           crlf->lf translation). In most cases, this will
                           just be _bufsiz. However, the buffer size may have
                           been changed, due to fseek optimization, at the
                           END of the last _filbuf call. */

                        if ( (rdcnt <= _SMALL_BUFSIZ) &&
                             (stream->_flag & _IOMYBUF) &&
                             !(stream->_flag & _IOSETVBUF) )
                        {
                            /* The translated contents of the buffer is small
                               and we are not at eof. The buffer size must have
                               been set to _SMALL_BUFSIZ during the last
                               _filbuf call. */

                            rdcnt = _SMALL_BUFSIZ;
                        }
                        else
                            rdcnt = stream->_bufsiz;

                        /* If first byte in untranslated buffer was a '\n',
                           assume it was preceeded by a '\r' which was
                           discarded by the previous read operation and count
                           the '\n'. */
                        if  (_osfile(fd) & FCRLF)
                            ++rdcnt;
                    }

                } /* end if FTEXT */

                filepos -= (long)rdcnt;

            } /* end else stream->_cnt != 0 */

        return(filepos + (long)offset);
}


/***
*_fseek_nolock() - Core fseek() routine (stream is locked)
*
*Purpose:
*       Core fseek() routine; assumes that caller has the stream locked.
*
*       [See fseek() for more info.]
*
*Entry: [See fseek()]
*
*Exit:  [See fseek()]
*
*Exceptions:
*
*******************************************************************************/

int __cdecl crtfseek_nolock (


        FILE *str,
        long offset,
        int whence
        )
{


        REG1 FILE *stream;


        /* Init stream pointer */
        stream = str;

        if ( !inuse(stream)) {
                errno=EINVAL;
                return(-1);
        }

        /* Clear EOF flag */

        stream->_flag &= ~_IOEOF;

        /* If seeking relative to current location, then convert to
           a seek relative to beginning of file.  This accounts for
           buffering, etc. by letting fseek() tell us where we are. */

        if (whence == SEEK_CUR) {
                offset += _ftell_nolock(stream);
                whence = SEEK_SET;
        }

        /* Flush buffer as necessary */

        _flush(stream);

        /* If file opened for read/write, clear flags since we don't know
           what the user is going to do next. If the file was opened for
           read access only, decrease _bufsiz so that the next _filbuf
           won't cost quite so much */

        if (stream->_flag & _IORW)
                stream->_flag &= ~(_IOWRT|_IOREAD);
        else if ( (stream->_flag & _IOREAD) && (stream->_flag & _IOMYBUF) &&
                  !(stream->_flag & _IOSETVBUF) )
                stream->_bufsiz = _SMALL_BUFSIZ;

        /* Seek to the desired locale and return. */

        return(_lseek(_fileno(stream), offset, whence) == -1L ? -1 : 0);
}

