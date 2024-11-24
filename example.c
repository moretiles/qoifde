#include "qoi.h"

int main(){
    int err = 0;

    /*
    //366x206
    // test.jpg / test.rgb / test.rgba
    int IMAGE_WIDTH = 366;
    int IMAGE_HEIGHT = 206;
    char CHANNELS = 4;
    char COLORSPACE = 1;
    char *readFilename = "assets/test.rgba";
    char *writeFilename = "out/mine.qoi";
    */

    /*
    //1464x823
    int IMAGE_WIDTH = 1464;
    int IMAGE_HEIGHT = 823;
    char CHANNELS = 4;
    char COLORSPACE = 1;
    char *readFilename = "assets/q1k3.rgba";
    char *writeFilename = "out/mine.qoi";
    */

    //5120x2880
    int IMAGE_WIDTH = 5120;
    int IMAGE_HEIGHT = 2880;
    char CHANNELS = 4;
    char COLORSPACE = 1;
    char *readFilename = "assets/wall.rgba";
    char *writeFilename = "out/mine.qoi";

    err = encodeQOI(readFilename, writeFilename, IMAGE_WIDTH, IMAGE_HEIGHT, CHANNELS, COLORSPACE);
    return err;
}
