#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
This program can perform 4 transformations on a ppm file: Negative coloring, grayscale, rotation, and scaling down to halfsize.
I originally thought this program would take me 3 days at least to really write, but I managed to code it all in under 4 hours when I really sat down and worked.
*/

typedef struct {//Structure to hold all the ppm file data
    int x, y;
    float* rpixNorm;
    float* gpixNorm;
    float* bpixNorm;
    unsigned char* rpix;
    unsigned char* gpix;
    unsigned char* bpix;
    char fname[15];
} PPMImage;

static PPMImage* readPPM(const char* filename)//Reads in ppm file
{
    char buff[16];
    PPMImage* img;
    FILE* fp;
    int c, rgb_comp_color;
    //open PPM file for reading
    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //read image format
    if (!fgets(buff, sizeof(buff), fp)) {
        perror(filename);
        exit(1);
    }

    //check the image format
    if (buff[0] != 'P' || buff[1] != '6') {
        fprintf(stderr, "Invalid image format (must be 'P6')\n");
        exit(1);
    }

    //alloc memory form image
    img = (PPMImage*)malloc(sizeof(PPMImage));
    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }
    strcpy(img->fname, filename);

    //check for comments
    c = getc(fp);
    while (c == '#') {
        while (getc(fp) != '\n');
        c = getc(fp);
    }

    ungetc(c, fp);
    //read image size information
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
        fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
        exit(1);
    }

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
        fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
        exit(1);
    }

    //check rgb component depth
    if (rgb_comp_color != 255) {
        fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
        exit(1);
    }

    while (fgetc(fp) != '\n');
    //memory allocation for pixel data
    img->rpix = (unsigned char*)malloc(img->x * img->y * sizeof(unsigned char));
    img->gpix = (unsigned char*)malloc(img->x * img->y * sizeof(unsigned char));
    img->bpix = (unsigned char*)malloc(img->x * img->y * sizeof(unsigned char));
    img->rpixNorm = (float*)malloc(img->x * img->y * sizeof(float));
    img->gpixNorm = (float*)malloc(img->x * img->y * sizeof(float));
    img->bpixNorm = (float*)malloc(img->x * img->y * sizeof(float));

    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    //read pixel data from file
    int counter = 0;
    while (counter < (img->x * img->y))
    {
        fread(img->rpix+counter, 1, 1, fp);
        fread(img->gpix+counter, 1, 1, fp);
        fread(img->bpix+counter, 1, 1, fp);
        float* rvalNorm = img->rpixNorm + counter;
        float* gvalNorm = img->gpixNorm + counter;
        float* bvalNorm = img->bpixNorm + counter;
        unsigned char* rval = img->rpix + counter;
        unsigned char* gval = img->gpix + counter;
        unsigned char* bval = img->bpix + counter;
        //Normalize unsigned ints for the normalized float pointers
        *rvalNorm = (float)((int)*rval / 255.0);
        *gvalNorm = (float)((int)*gval / 255.0);
        *bvalNorm = (float)((int)*bval / 255.0);
        counter++;  
    }    

    fclose(fp);
    return img;
}
void writePPM(const char* filename, PPMImage* img)
{
    FILE* fp;
    //open file for output
    fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //write the header file
    //image format
    fprintf(fp, "P6\n");

    //image size
    fprintf(fp, "%d %d\n", img->x, img->y);

    // rgb component depth
    fprintf(fp, "%d\n", 255);

    // pixel data
    int counter = 0;
    while (counter < (img->x * img->y))
    {
        fwrite(img->rpix + counter, 1, 1, fp);
        fwrite(img->gpix + counter, 1, 1, fp);
        fwrite(img->bpix + counter, 1, 1, fp);
        counter++;
    }
    fclose(fp);
}

void invert(PPMImage* img)//Invert image colors
{
    int counter = 0;
    while (counter < img->x * img->y) {
        *(img->rpixNorm + counter) = 1 - *(img->rpixNorm + counter);
        *(img->gpixNorm + counter) = 1 - *(img->gpixNorm + counter);
        *(img->bpixNorm + counter) = 1 - *(img->bpixNorm + counter);
        *(img->rpix + counter) = (unsigned char)(int)(*(img->rpixNorm + counter) * 255);
        *(img->gpix + counter) = (unsigned char)(int)(*(img->gpixNorm + counter) * 255);
        *(img->bpix + counter) = (unsigned char)(int)(*(img->bpixNorm + counter) * 255);
        counter++;
    }
}

void rotate(PPMImage* img)//Rotate image 90 degrees clockwise. Done by essentially calling a variation of the write function, so no write function is further needed after this call
{
    FILE* fp;
    char newName[128];
    //open file for output
    strncpy(newName, img->fname, (strlen(img->fname) - 4));
    strcat(newName, "_rotate.ppm");
    fp = fopen(newName, "wb");
    int temp = img->x;
    img->x = img->y;
    img->y = temp;
    int rowcount = 0;
    int colcount = 0;
    int counter = 0;
    int offset = img->y;
    int offset2 = img->x - 1;
    fprintf(fp, "P6\n");

    //image size
    fprintf(fp, "%d %d\n", img->x, img->y);

    // rgb component depth
    fprintf(fp, "%d\n", 255);
    while (colcount < img->y) {
        rowcount = 0;
        offset2 = img->x - 1;
        while (rowcount < img->x) {//Math for rotation
            unsigned char* rPrev = img->rpix + (offset * offset2);
            unsigned char* gPrev = img->gpix + (offset * offset2);
            unsigned char* bPrev = img->bpix + (offset * offset2);
            rPrev += counter;
            gPrev += counter;
            bPrev += counter;
            fwrite(rPrev, 1, 1, fp);
            fwrite(gPrev, 1, 1, fp);
            fwrite(bPrev, 1, 1, fp);
            offset2--;
            rowcount++;
        }
        counter++;
        colcount++;
    }
    fclose(fp);
}

void grayscale(PPMImage* img)//Convert image to greyscale by averaging and equating all r g and b values
{
    int counter = 0;
    while (counter < img->x * img->y) {
        float total = *(img->rpixNorm + counter) + *(img->gpixNorm + counter) + *(img->bpixNorm + counter);
        total = total / 3.0;
        *(img->rpixNorm + counter) = total;
        *(img->gpixNorm + counter) = total;
        *(img->bpixNorm + counter) = total;
        *(img->rpix + counter) = (unsigned char)(int)(total*255);
        *(img->gpix + counter) = (unsigned char)(int)(total*255);
        *(img->bpix + counter) = (unsigned char)(int)(total*255);
        counter++;
    }
}

void halfsize(PPMImage* img)//Scale the image to half its original dimensions taking the average r g and b values from nearby pixels
{
    int newX;
    int newY;
    int rowOffset = img->x*2;
    if (img->x % 2 == 0) {
        if (img->y % 2 == 0) {//Even width and height
            newX = img->x/2;
            newY = img->y/2;
        }
        else {//Even width odd height
            newX = img->x/2;
            newY = (img->y-1)/2;
        }
    }
    else {
        if (img->y % 2 == 0) {//Odd width even height
            newX = (img->x-1)/2;
            newY = img->y;
        }
        else {//Odd width and height
            newX = (img->x-1)/2;
            newY = (img->y-1)/2;
        }
    }

    float* newR = (float*)(malloc(sizeof(float) * newX * newY));
    float* newG = (float*)(malloc(sizeof(float) * newX * newY));
    float* newB = (float*)(malloc(sizeof(float) * newX * newY));
    int counter = 0;
    int mastercounter = 0;
    int rowcount = 0;
    while (rowcount < newY) {
        counter = 0;
        while (counter < newX) {
            float averageR = (*(img->rpixNorm + (rowOffset * rowcount) + (2 * counter)) + *(img->rpixNorm + (rowOffset * rowcount) + (2 * counter + 1)) + *(img->rpixNorm + (rowOffset * rowcount) + (2 * counter + (img->x))) + *(img->rpixNorm + (rowOffset * rowcount) + (2 * counter + (img->x)) + 1)) / 4.0;
            float averageG = (*(img->gpixNorm + (rowOffset * rowcount) + (2 * counter)) + *(img->gpixNorm + (rowOffset * rowcount) + (2 * counter + 1)) + *(img->gpixNorm + (rowOffset * rowcount) + (2 * counter + (img->x))) + *(img->gpixNorm + (rowOffset * rowcount) + (2 * counter + (img->x)) + 1)) / 4.0;
            float averageB = (*(img->bpixNorm + (rowOffset * rowcount) + (2 * counter)) + *(img->bpixNorm + (rowOffset * rowcount) + (2 * counter + 1)) + *(img->bpixNorm + (rowOffset * rowcount) + (2 * counter + (img->x))) + *(img->bpixNorm + (rowOffset * rowcount) + (2 * counter + (img->x)) + 1)) / 4.0;
            *(img->rpixNorm + mastercounter) = averageR;
            *(img->gpixNorm + mastercounter) = averageG;
            *(img->bpixNorm + mastercounter) = averageB;
            counter++;
            mastercounter++;
        }
        rowcount++;
    }
    for (int i = 0; i < newX * newY; i++) {
        //Send new values to img pixel values
        *(img->rpix + i) = *(img->rpixNorm + i) * 255;
        *(img->gpix + i) = *(img->gpixNorm + i) * 255;
        *(img->bpix + i) = *(img->bpixNorm + i) * 255;
    }
    img->x = newX;
    img->y = newY;
    //Free dynamically allocated memory
    free(newR);
    free(newG);
    free(newB);
}

int main(int argc, char*argv[]) {
    PPMImage* image;
    image = readPPM(argv[1]);
    char newName[128];
    //Name extensions to write new filename
    char* rExt = "_rotate.ppm";
    char* gExt = "_grayscale.ppm";
    char* sExt = "_halfsize.ppm";
    char* nExt = "_negative.ppm";
    int wVal = 0;
    strncpy(newName, argv[1], (strlen(argv[1])-4));//Generate buffer to hold new filename
    if ((strcmp(argv[2], "g") == 0)|| (strcmp(argv[2], "G") == 0))//Grayscale
    {
        printf("Grayscale");
        strcat(newName, gExt);
        grayscale(image);

    }
    if ((strcmp(argv[2], "n") == 0) || (strcmp(argv[2], "N") == 0))//Negative
    {
        printf("Negative");
        strcat(newName, nExt);
        invert(image);
    }
    if ((strcmp(argv[2], "r") == 0) || (strcmp(argv[2], "R") == 0))//Rotate (No longer need to call writePPM)
    {
        printf("Rotate");
        rotate(image);
        wVal = 1;
    }
    if ((strcmp(argv[2], "s") == 0) || (strcmp(argv[2], "S") == 0))//Scale down to halfsize
    {
        printf("Scale");
        halfsize(image);
        strcat(newName, sExt);
    }
    if (wVal == 0) {
        writePPM(newName, image);
    }

    //Free dynamically allocated memory
    free(image->rpix);
    free(image->gpix);
    free(image->bpix);
    free(image->rpixNorm);
    free(image->gpixNorm);
    free(image->bpixNorm);
    free(image);

    getchar();
}
