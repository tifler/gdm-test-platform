#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "ion_api.h"
#include "v4l2_jpeg_api.h"
#include "mme_c_api.h"

static int jpeg_enc_v4l2(int argc, char* argv[]);

#ifdef WIN32
int main_jpeg_enc(int argc, char* argv[]) 
#else
int main(int argc, char* argv[]) 
#endif
{
    int iret; 
    
    iret = jpeg_enc_v4l2(argc, argv);

    return iret;
}


#define IN_FILE_BITMAP      1
#define IN_FILE_YUV420_FILE 2
static int jpeg_enc_v4l2(int argc, char* argv[]) 
{
    int iret = 0;
    char input_filename[256];
    char jpeg_filename[256];
    char szbuffer[128];

    int pic_width, pic_height;
    
    struct v4l2_ion_frame v4l2_ion_frame_src;
    struct v4l2_ion_buffer v4l2_ion_jpeg_dst;
    
    int jpegsize, jpeg_enc_quality = 100 /* range 0~100 */;
    unsigned int* p_int_tmp;
    unsigned int pix_fmt_src;
    int input_file_type = 0;

    FILE* fp;
    
    if(argc < 3) {
        printf("\n--------------------------------------------------------------------------------------\n\n");
        fprintf(stderr, "!!!!! ERROR: arg count must be 3   \n");
        fprintf(stderr, "  ex1) [bitmap file] jpeg_enc  [bitmap filename] [yuv444/yuv420/nv12/yuv422] \n");
        fprintf(stderr, "  ex2) [yuv file] jpeg_enc  [yuv420 filename(*.yuv)] [widht] [height] \n");
        printf("\n--------------------------------------------------------------------------------------\n");
        return -1;
    }

    
    /* init parameter */
    strcpy(input_filename, argv[1]);
    memset(&v4l2_ion_frame_src, 0x00, sizeof(v4l2_ion_frame_src));
    memset(&v4l2_ion_jpeg_dst, 0x00, sizeof(v4l2_ion_jpeg_dst));

    /* check file type */
    mme_util_split_file_ext(input_filename, szbuffer); /* e.g  IN:/mnt/abcde.dat   OUT: abcde */    
    mme_util_string_make_lower(szbuffer);
    if(strcmp(szbuffer, "bmp") == 0) {
        input_file_type = IN_FILE_BITMAP;
    }
    else if(strcmp(szbuffer, "yuv") == 0) {

        if(argc != 4) {
            printf("\n--------------------------------------------------------------------------------------\n\n");
            fprintf(stderr, "!!!!! ERROR: yuv file arg must be 4   \n");
            fprintf(stderr, "  ex) jpeg_enc  [yuv420 filename(*.yuv)] [widht] [height] \n");
            printf("\n--------------------------------------------------------------------------------------\n");
            iret = -1;
        }
        else {
            input_file_type = IN_FILE_YUV420_FILE;
        }
    }
    else {
        iret = -1;
    }
    

    if(input_file_type == IN_FILE_BITMAP) {

        /* check Bmp => YUV Format */        
        if(strcmp(argv[2], "yuv444") == 0) {
            pix_fmt_src = V4L2_PIX_FMT_YUV444_P1; /* YUV444 24bit 1P */
        }
        else if(strcmp(argv[2], "yuv420") == 0) {
            pix_fmt_src = V4L2_PIX_FMT_YUV420; /* YUV420 12bit 3P */
        }
        else if(strcmp(argv[2], "nv12") == 0) {
            pix_fmt_src = V4L2_PIX_FMT_NV12; /* Y/CbCr 4:2:0 12bit 2P */
        }
        else if(strcmp(argv[2], "yuv422") == 0) {
            pix_fmt_src = V4L2_PIX_FMT_NV16; /* 164bit Y/CbCr 4:2:2 Plane 2 */
        }
        else {
            printf("\n--------------------------------------------------------------------------------------\n\n");
            fprintf(stderr, "!!!!! ERROR: not supoort yuv foramt   \n");
            printf("\n--------------------------------------------------------------------------------------\n");
            iret = -1;
        }

        /* load bitmap / alloc ion frame buf / convert RGB =>YUV.. */
        if(iret == 0) {
            iret = mme_imagetool_bmp_get_info(input_filename, &pic_width, &pic_height, NULL, NULL);
            if(iret != 0) {
                fprintf(stderr, "!!!!! FAIL: mme_imagetool_bmp_get_info \n");
            }
            else {
                
                iret = v4l2_ion_alloc_yuv_frame(pic_width, pic_height, pix_fmt_src, &v4l2_ion_frame_src);
                if(iret != 0) {
                    fprintf(stderr, "!!!!! FAIL: alloc ion buffer for YUV Frame \n");
                }
                else {
                    
                    iret = mme_imagetool_bmp_load_file_for_v4l2(input_filename, &v4l2_ion_frame_src);
                    if(iret != 0) {
                        fprintf(stderr, "!!!!! FAIL: load bitmap and convert color \n");
                    }
                }
            }
        }
    }
    else if(input_file_type == IN_FILE_YUV420_FILE) {
        pic_width = atoi(argv[2]);
        pic_height = atoi(argv[3]);

        if( (pic_width < 16) || (pic_width > 5000)
            || (pic_height < 16) || (pic_height > 4000) ){
            fprintf(stderr, "!!!!! FAIL: width/height is out of range \n");
            iret = -1;
        }
        else {
            pix_fmt_src = V4L2_PIX_FMT_YUV420; /* YUV420 12bit 3P */
            iret = v4l2_ion_alloc_yuv_frame(pic_width, pic_height, pix_fmt_src, &v4l2_ion_frame_src);
            if(iret != 0) {
                fprintf(stderr, "!!!!! FAIL: alloc ion buffer for YUV Frame \n");
            }
            else {
                iret = mme_imagetool_yuv_load_file_for_v4l2(input_filename, &v4l2_ion_frame_src);
                if(iret != 0) {
                    fprintf(stderr, "!!!!! FAIL: load bitmap and convert color \n");
                }
            }
        }
    }
    else {
        fprintf(stderr, "!!!!! FAIL: not support INPUT FILE %s\n", input_filename);
        iret = -1;
    }

    /* load stream buf */
    if(iret == 0) {
        iret = v4l2_ion_alloc_stream(pic_width, pic_height, &v4l2_ion_jpeg_dst);
        if(iret != 0) {
            fprintf(stderr, "!!!!! FAIL: alloc jpeg ion stream buf \n");
        }

    }

    /* jpeg encoding */
    if(iret == 0) {
        iret = v4l2_jpeg_encode_ion(&v4l2_ion_frame_src, jpeg_enc_quality, &v4l2_ion_jpeg_dst, &jpegsize);
        if(iret != 0) {
            fprintf(stderr, "!!!!! FAIL: v4l2_jpeg_encode_ion \n");
        }
        else {
#ifdef WIN32
                strcpy(jpeg_filename, "D:\\work\\");
#else
                strcpy(jpeg_filename, "/tmp/");
#endif
                mme_util_split_file_name(input_filename, szbuffer); /* e.g  IN:/mnt/abcde.dat   OUT: abcde */    
                strcat(jpeg_filename, szbuffer);
                if(input_file_type = IN_FILE_BITMAP) {
                    strcat(jpeg_filename, ".bmp");
                }
                
                sprintf(szbuffer, ".%dx%d", pic_width, pic_height);
                strcat(jpeg_filename, szbuffer);
                switch(pix_fmt_src) {
                    case V4L2_PIX_FMT_YUV444_P1: /* YUV444 24bit 1P */
                        strcat(jpeg_filename, ".yuv444p1");
                        break;
                    case V4L2_PIX_FMT_YUV420: /* YUV420 12bit 3P */
                        strcat(jpeg_filename, ".yuv420p3");
                        break;
                    case V4L2_PIX_FMT_NV16: /* 16bit Y/CbCr 4:2:2 Plane 2, V4L2_PIX_FMT_NV16 */
                        strcat(jpeg_filename, ".yuv422p2");
                        break;
                    case V4L2_PIX_FMT_NV12: /* 12bit Y/CbCr 4:2:0 Plane 2, V4L2_PIX_FMT_NV12 */
                        strcat(jpeg_filename, ".yuv420p2");
                        break;
                }
                strcat(jpeg_filename, ".jpeg");

                fp = fopen(jpeg_filename, "wb");
                if(fp!=NULL) {
                    fwrite((void*)v4l2_ion_jpeg_dst.vir_addr, 1, jpegsize, fp);
                    fclose(fp);
                }

                p_int_tmp = (unsigned int*)v4l2_ion_jpeg_dst.vir_addr;
                printf("\n\n");
                printf("[jpeg_enc] SUCCESS: enc jpeg  result_file = %s (0x%08x 0x%08x) \n", jpeg_filename, p_int_tmp[0], p_int_tmp[1] );
                printf("\n\n");
        }
    }


    v4l2_ion_free_yuv_frame(&v4l2_ion_frame_src);
    v4l2_ion_free_stream(&v4l2_ion_jpeg_dst);
    
    return iret;
}



#if 0
static int jpeg_enc_mme(int argc, char* argv[]) 
{
    int iret = 0, iret1, ion_fd, i;
    char bmp_filename[256];
    char jpeg_filename[256];
    char szbuffer[128];
    
    struct mme_image_buffer img_buf_bmp;
    struct mme_image_buffer img_buf_yuv;
    struct mme_ion_plane ion_plane_yuv;

    unsigned char* jpegdata = NULL;
    int jpegdata_max_size;
    int jpegsize, jpeg_enc_quality = 100 /* range 0~100 */;
    unsigned int* p_int_tmp;

    FILE* fp;
    
    if(argc < 2) {
        printf("\n--------------------------------------------------------------------------------------\n\n");
        fprintf(stderr, "!!!!! ERROR: arg count must be 2  ex) jpeg_enc  [bitmap filename] [swcodec] \n");
        printf("\n--------------------------------------------------------------------------------------\n");
        return -1;
    }

    /* init parameter */
    strcpy(bmp_filename, argv[1]);
    memset(&img_buf_bmp, 0x00, sizeof(img_buf_bmp));
    memset(&img_buf_yuv, 0x00, sizeof(img_buf_yuv));
    memset(&ion_plane_yuv, 0x00, sizeof(ion_plane_yuv));
    for(i = 0; i < MME_MAX_PLANE_COUNT; i++) {
        ion_plane_yuv.plane[i].shared_fd = -1;
    }
    
    /* load bitmap file info & data*/
    if(iret == 0) {

        img_buf_bmp.type = MME_IMG_BUF_USERPTR;
        iret = mme_imagetool_bmp_get_info(bmp_filename, &img_buf_bmp.width, &img_buf_bmp.height, &img_buf_bmp.fourcc, &img_buf_bmp.stride);
        if(iret != 0) {
            fprintf(stderr, "FAIL:  get bitmap file info \n");
        }
        else {
            /* alloc bitmap data */
            img_buf_bmp.m.data = malloc(img_buf_bmp.width*img_buf_bmp.height*4);
            if(img_buf_bmp.m.data == NULL) {
                fprintf(stderr, "FAIL: bmp_data = malloc(sz = %d) \n", img_buf_bmp.width*img_buf_bmp.height*4);
                iret = -1;
            }
            else {
                iret = mme_imagetool_bmp_load_file(bmp_filename, img_buf_bmp.m.data, img_buf_bmp.width*img_buf_bmp.height*4);
                if(iret != 0) {
                      fprintf(stderr, "FAIL:  load bitmap file  \n");
                }
            }
        }
    }

    /* alloc yuv data */
    if(iret == 0) {
        
        img_buf_yuv.type = MME_IMG_BUF_ION_PLANE;
        img_buf_yuv.width = img_buf_bmp.width;
        img_buf_yuv.height = img_buf_bmp.height;
        img_buf_yuv.fourcc = MME_FOURCC_IMAGE_YUV444_P1; 
        img_buf_yuv.stride = MME_BYTE_ALIGN(img_buf_yuv.width*3, 16);
        img_buf_yuv.m.ion_plane = &ion_plane_yuv;
        
        ion_plane_yuv.plane_count = 1;
        ion_plane_yuv.plane[0].buf_size = img_buf_yuv.stride*img_buf_yuv.height;
        ion_plane_yuv.plane[0].mem_offset = 0;
        
        ion_fd = ion_open();
        if(ion_fd >= 0) {
            for(i = 0; i < ion_plane_yuv.plane_count; i++) {
                iret1 = ion_alloc_fd(ion_fd, ion_plane_yuv.plane[i].buf_size, 0, ION_HEAP_CARVEOUT_MASK,  0, &ion_plane_yuv.plane[i].shared_fd);
                if(iret1 < 0) {
                    iret = -1;
                    fprintf(stderr, "FAIL:  ion_alloc_fd i=%d bufsz=%d \n", i, ion_plane_yuv.plane[i].buf_size);
                    break;
                }
                else {
                    ion_plane_yuv.plane[i].vir_addr = (unsigned int)MME_DRIVER_MMAP(NULL, ion_plane_yuv.plane[i].buf_size, (PROT_READ | PROT_WRITE), MAP_SHARED, ion_plane_yuv.plane[i].shared_fd, 0);
                    if(ion_plane_yuv.plane[i].vir_addr == 0) {
                        iret = -1;
                        fprintf(stderr, "FAIL:  ion mmap i=%d bufsz=%d \n", i, ion_plane_yuv.plane[i].buf_size);
                        break;
                    }
                }
            } /* end of for(i = 0; i < ion_plane_yuv.plane_count; i++) { */

            ion_close(ion_fd);
        } /* end of if(ion_fd >= 0) { */
    }

    /* image color convert RGB => YUV*/
    if(iret == 0) {
        
        iret = mme_imagetool_convert_color(&img_buf_bmp, &img_buf_yuv);
        if(iret != 0) {
            fprintf(stderr, "FAIL:  color convert \n");
        }
    }

    /* jpeg encoding */
    if(iret == 0) {

        jpegdata_max_size = img_buf_yuv.stride*img_buf_yuv.height;
        jpegsize = 0;
        jpegdata = (unsigned char*)malloc(jpegdata_max_size);
        if(jpegdata == NULL) {
            fprintf(stderr, "FAIL: malloc jpegdata sz=%d \n", jpegdata_max_size);
            iret = -1;
        }
        else {
            //iret = mme_jpeg_encode_libjpeg(&img_buf_yuv, jpeg_enc_quality, jpegdata, jpegdata_max_size, &jpegsize);
            //iret = mme_jpeg_encode_jpu(&img_buf_yuv, jpeg_enc_quality, jpegdata, jpegdata_max_size, &jpegsize);
            iret = mme_jpeg_encode_v4l2(&img_buf_yuv, jpeg_enc_quality, jpegdata, jpegdata_max_size, &jpegsize);
            if(iret != 0) {
                fprintf(stderr, "FAIL:  mme_jpeg_encode \n");
            }
            else {
#ifdef WIN32
                strcpy(jpeg_filename, "D:\\work\\");
#else
                strcpy(jpeg_filename, "/tmp/");
#endif
                mme_util_split_file_name(bmp_filename, szbuffer); /* e.g  IN:/mnt/abcde.dat   OUT: abcde */    
                strcat(jpeg_filename, szbuffer);
                strcat(jpeg_filename, ".bmp.jpeg");

                fp = fopen(jpeg_filename, "wb");
                if(fp!=NULL) {
                    fwrite(jpegdata, 1, jpegsize, fp);
                    fclose(fp);
                }

                p_int_tmp = (unsigned int*)jpegdata;
                printf("\n\n");
                printf("[jpeg_enc] SUCCESS: enc jpeg  result_file = %s (0x%08x 0x%08x) \n", jpeg_filename, p_int_tmp[0], p_int_tmp[1] );
                printf("\n\n");
            }
        }
    }


    /* free jpeg data*/
    if(jpegdata != NULL) free(jpegdata);

    /* free bmp data*/
    if(img_buf_bmp.m.data != NULL) free(img_buf_bmp.m.data);

    /* free ion */
    for(i = 0; i < ion_plane_yuv.plane_count; i++) {
        if(ion_plane_yuv.plane[i].shared_fd >= 0) {
        
            if(ion_plane_yuv.plane[i].vir_addr != 0) {
                MME_DRIVER_MUNMAP(ion_plane_yuv.plane[i].vir_addr, ion_plane_yuv.plane[i].buf_size);
                ion_plane_yuv.plane[i].vir_addr = 0;
            }
            MME_DRIVER_CLOSE(ion_plane_yuv.plane[i].shared_fd);
            ion_plane_yuv.plane[i].shared_fd = -1;
        }
    } /* end of for(i = 0; i < ion_plane_yuv.plane_count; i++) { */

    
    return iret;
}

#endif
