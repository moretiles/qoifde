#include "qoiconv.h"

int main(){
    int err = 0;

    int IMAGE_WIDTH = 0;
    int IMAGE_HEIGHT = 0;
    char CHANNELS = 0;
    char COLORSPACE = 0;
    char *readFilename, *writeFilename;

    readFilename = "assets/dog1.rgb";
    writeFilename = "out/dog1.qoi";
    IMAGE_WIDTH = 1546;
    IMAGE_HEIGHT = 1213;
    CHANNELS = 3;
    COLORSPACE = 1;
    err = encodeQOI(readFilename, writeFilename, IMAGE_WIDTH, IMAGE_HEIGHT, CHANNELS, COLORSPACE);

    readFilename = "assets/dog2.rgb";
    writeFilename = "out/dog2.qoi";
    IMAGE_WIDTH = 1920;
    IMAGE_HEIGHT = 1177;
    CHANNELS = 3;
    COLORSPACE = 1;
    err = encodeQOI(readFilename, writeFilename, IMAGE_WIDTH, IMAGE_HEIGHT, CHANNELS, COLORSPACE);

    readFilename = "assets/dog3.rgb";
    writeFilename = "out/dog3.qoi";
    IMAGE_WIDTH = 1280;
    IMAGE_HEIGHT = 960;
    CHANNELS = 3;
    COLORSPACE = 1;
    err = encodeQOI(readFilename, writeFilename, IMAGE_WIDTH, IMAGE_HEIGHT, CHANNELS, COLORSPACE);

    readFilename = "assets/dog4.rgb";
    writeFilename = "out/dog4.qoi";
    IMAGE_WIDTH = 3198;
    IMAGE_HEIGHT = 2400;
    CHANNELS = 3;
    COLORSPACE = 1;
    err = encodeQOI(readFilename, writeFilename, IMAGE_WIDTH, IMAGE_HEIGHT, CHANNELS, COLORSPACE);

    readFilename = "assets/dog5.rgb";
    writeFilename = "out/dog5.qoi";
    IMAGE_WIDTH = 626;
    IMAGE_HEIGHT = 417;
    CHANNELS = 3;
    COLORSPACE = 1;
    err = encodeQOI(readFilename, writeFilename, IMAGE_WIDTH, IMAGE_HEIGHT, CHANNELS, COLORSPACE);

    readFilename = "assets/q1k3.rgb";
    writeFilename = "out/q1k3.qoi";
    IMAGE_WIDTH = 1464;
    IMAGE_HEIGHT = 823;
    CHANNELS = 3;
    COLORSPACE = 1;
    err = encodeQOI(readFilename, writeFilename, IMAGE_WIDTH, IMAGE_HEIGHT, CHANNELS, COLORSPACE);

    readFilename = "assets/test.rgb";
    writeFilename = "out/test.qoi";
    IMAGE_WIDTH = 366;
    IMAGE_HEIGHT = 206;
    CHANNELS = 3;
    COLORSPACE = 1;
    err = encodeQOI(readFilename, writeFilename, IMAGE_WIDTH, IMAGE_HEIGHT, CHANNELS, COLORSPACE);

    readFilename = "assets/wall.rgba";
    writeFilename = "out/wall.qoi";
    IMAGE_WIDTH = 5120;
    IMAGE_HEIGHT = 2880;
    CHANNELS = 4;
    COLORSPACE = 1;
    err = encodeQOI(readFilename, writeFilename, IMAGE_WIDTH, IMAGE_HEIGHT, CHANNELS, COLORSPACE);

    /*
    //366x206
    // test.jpg / test.rgb / test.rgba
    int IMAGE_WIDTH = 366;
    int IMAGE_HEIGHT = 206;
    char CHANNELS = 4;
    char COLORSPACE = 1;

    char *readFilename = "assets/test.rgba";
    char *writeFilename = "out/mine.qoi";

    //1464x823
    // q1k3.jpg / q1k3.rgb / q1k3.rgba
    int IMAGE_WIDTH = 1464;
    int IMAGE_HEIGHT = 823;
    char CHANNELS = 4;
    char COLORSPACE = 1;

    char *readFilename = "assets/q1k3.rgba";
    char *writeFilename = "out/mine.qoi";

    //5120x2880
    int IMAGE_WIDTH = 5120;
    int IMAGE_HEIGHT = 2880;
    char CHANNELS = 4;
    char COLORSPACE = 1;

    char *readFilename = "assets/wall.rgba";
    char *writeFilename = "out/mine.qoi";

    err = encodeQOI(readFilename, writeFilename, IMAGE_WIDTH, IMAGE_HEIGHT, CHANNELS, COLORSPACE);
    */

    /*
       err = decodeQOI("./assets/dog5.qoi", "./out/dog5.rgb");
       err = decodeQOI("./assets/test.qoi", "./out/test.rgb");
       err = decodeQOI("./assets/q1k3.qoi", "./out/q1k3.rgb");
       err = decodeQOI("./assets/dog3.qoi", "./out/dog3.rgb");
       err = decodeQOI("./assets/dog2.qoi", "./out/dog2.rgb");
       err = decodeQOI("./assets/dog1.qoi", "./out/dog1.rgb");
       err = decodeQOI("./assets/dog4.qoi", "./out/dog4.rgb");
       err = decodeQOI("./assets/wall.qoi", "./out/wall.rgba");
       */

    return err;
}
