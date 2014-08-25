#ifndef __GISP_H__
#define __GISP_H__

/*****************************************************************************/

enum GISP_INPUT_TYPE {
    GISP_INPUT_FRONT,
    GISP_INPUT_BACK,
    GISP_INPUT_VIRTUAL,
    GISP_INPUT_COUNT,
};

enum GISP_OUTPUT_TYPE {
    GISP_OUTPUT_CAPTURE,
    GISP_OUTPUT_VIDEO,
    GISP_OUTPUT_DISPLAY,
    GISP_OUTPUT_FACEDETECT,
    GISP_OUTPUT_COUNT,
};

/*****************************************************************************/

struct GISPDevice;

struct GISPDevice *GISP_Open(void);
void GISP_Close(struct GISPDevice *gisp);

int GISP_SetInput(struct GISPDevice *gisp, enum GISP_INPUT_TYPE input);
int GISP_GetInput(struct GISPDevice *gisp);

int GISP_SetInputFormat(struct GISPDevice *gisp, const struct GISPFormat *fmt);
int GISP_GetInputFormat(struct GISPDevice *gisp, struct GISPFormat *fmt);

int GISP_SetOutputFormat(struct GISPDevice *gisp,
        enum GISP_OUTPUT_TYPE output, const struct GISPFormat *fmt);
int GISP_GetOutputFormat(struct GISPDevice *gisp,
        enum GISP_OUTPUT_TYPE output, struct GISPFormat *fmt);

int GISP_SetOutputBuffer(
        struct GISPDevice *gisp, enum GISP_OUTPUT_TYPE output,
        unsigned int count, struct GISPBuffer **buffer);

#endif  /*__GISP_H__*/
