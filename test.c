#include <sys/time.h>
#include <stdio.h>   
#include <stdlib.h> 
#include <string.h>
#include "tjpgd.h"

#include <time.h>
#include <stdint.h>

typedef int64_t tmillis_t;


tmillis_t GetTimeInMillis(){
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

void SleepMillis(tmillis_t milli_seconds){
	if (milli_seconds <= 0) return;
	struct timespec ts;
	ts.tv_sec = milli_seconds / 1000;
	ts.tv_nsec = (milli_seconds % 1000) * 1000000;
	nanosleep(&ts, NULL);
}


/* Bytes per pixel of image output */
#define N_BPP (3 - JD_FORMAT)


/* Session identifier for input/output functions (name, members and usage are as user defined) */
typedef struct {
    FILE *fp;               /* Input stream */
    uint8_t *fbuf;          /* Output frame buffer */
    unsigned int wfbuf;     /* Width of the frame buffer [pix] */
} IODEV;


/*------------------------------*/
/* User defined input funciton  */
/*------------------------------*/

size_t in_func (    /* Returns number of bytes read (zero on error) */
    JDEC* jd,       /* Decompression object */
    uint8_t* buff,  /* Pointer to the read buffer (null to remove data) */
    size_t nbyte    /* Number of bytes to read/remove */
)
{
    IODEV *dev = (IODEV*)jd->device;   /* Session identifier (5th argument of jd_prepare function) */


    if (buff) { /* Raad data from imput stream */
        return fread(buff, 1, nbyte, dev->fp);
    } else {    /* Remove data from input stream */
        return fseek(dev->fp, nbyte, SEEK_CUR) ? 0 : nbyte;
    }
}


/*------------------------------*/
/* User defined output funciton */
/*------------------------------*/

int out_func (      /* Returns 1 to continue, 0 to abort */
    JDEC* jd,       /* Decompression object */
    void* bitmap,   /* Bitmap data to be output */
    JRECT* rect     /* Rectangular region of output image */
)
{
    IODEV *dev = (IODEV*)jd->device;   /* Session identifier (5th argument of jd_prepare function) */
    uint8_t *src, *dst;
    uint16_t y, bws;
    unsigned int bwd;


    /* Progress indicator */
    if (rect->left == 0) {
        printf("\r%lu%%", (rect->top << jd->scale) * 100UL / jd->height);
    }

    /* Copy the output image rectangle to the frame buffer */
    src = (uint8_t*)bitmap;                           /* Output bitmap */
    dst = dev->fbuf + N_BPP * (rect->top * dev->wfbuf + rect->left);  /* Left-top of rectangle in the frame buffer */
    bws = N_BPP * (rect->right - rect->left + 1);     /* Width of the rectangle [byte] */
    bwd = N_BPP * dev->wfbuf;                         /* Width of the frame buffer [byte] */
    for (y = rect->top; y <= rect->bottom; y++) {
        memcpy(dst, src, bws);   /* Copy a line */
        src += bws; dst += bwd;  /* Next line */
    }

    return 1;    /* Continue to decompress */
}

/*------------------------------*/
/* Program Main                 */
/*------------------------------*/

int main (int argc, char* argv[])
{
    JRESULT res;      /* Result code of TJpgDec API */
    JDEC jdec;        /* Decompression object */
    void *work;       /* Pointer to the work area */
    size_t sz_work = 10240; /* Size of work area */
    IODEV devid;      /* Session identifier */


    /* Initialize input stream */
    if (argc < 2) return -1;
    devid.fp = fopen(argv[1], "rb");
    if (!devid.fp) return -1;

    /* Prepare to decompress */
    work = (void*)malloc(sz_work);
    res = jd_prepare(&jdec, in_func, work, sz_work, &devid);
    if (res == JDR_OK) {
        /* It is ready to dcompress and image info is available here */
        printf("Image size is %u x %u.\n%lu bytes of work ares is used.\n", jdec.width, jdec.height, sz_work - jdec.sz_pool);

        /* Initialize output device */
        devid.fbuf = (uint8_t*)malloc(N_BPP * jdec.width * jdec.height); /* Create frame buffer for output image */
        devid.wfbuf = jdec.width;

        tmillis_t last = GetTimeInMillis();
        res = jd_decomp(&jdec, out_func, 0);   /* Start to decompress with 1/1 scaling */
        if (res == JDR_OK) {
            /* Decompression succeeded. You have the decompressed image in the frame buffer here. */
            printf("\rDecompression succeeded.Cost %ldms\n", GetTimeInMillis()-last);

        } else {
            printf("jd_decomp() failed (rc=%d)\n", res);
        }

        free(devid.fbuf);    /* Discard frame buffer */

    } else {
        printf("jd_prepare() failed (rc=%d)\n", res);
    }

    free(work);             /* Discard work area */

    fclose(devid.fp);       /* Close the JPEG file */

    return res;
}