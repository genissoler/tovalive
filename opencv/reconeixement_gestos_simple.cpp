#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

void calibrarMa();
void calibrarVenda();

/*Valors dels píxels que pertanyen a la mà*/
int red[2] = {255,0};
int blue[2] = {255,0};
int green[2] = {255,0};

/*Valors dels píxels que pertanyen a la venda o polsera*/
int vred[2] = {255,0};
int vblue[2] = {255,0};
int vgreen[2] = {255,0};


int distOK = 120;

IplImage* frame;
IplImage* result;

int main(int argc, char* argv[]){
    cvNamedWindow("Color detection",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Binaria",CV_WINDOW_AUTOSIZE);
    CvCapture* capture=cvCaptureFromCAM(0);
    frame = cvQueryFrame(capture);
    result = cvCreateImage( cvGetSize(frame), 8, 3 );
    IplImage* destination = cvCreateImage( cvGetSize(frame), 8, 3 );
    CvFont font;
    cvInitFont(&font, CV_FONT_VECTOR0, 2.0, 2.0, 0, 3, CV_AA);


    boolean calibratMa = false;
    boolean calibratVenda = false;

    int input;
    while( (input = cvWaitKey(50))!= 118 ) {
        frame = cvQueryFrame(capture);

        if(!calibratMa){
            if(input == 32){
                calibrarMa();
                calibratMa = true;
            }
            else cvRectangle( frame, cvPoint(200, 100), cvPoint(300, 200), CV_RGB(255,0,0), 2 );
            /*Es mostra un rectangle per posar a sobre la mà*/

        }
        else if(!calibratVenda){
            if(input == 32){
                calibrarVenda();
                calibratVenda = true;
            }
            else cvRectangle( frame, cvPoint(200, 250), cvPoint(250, 300), CV_RGB(0,255,255), 2 );
            /*Es mostra un rectangle per posar a sobre la mà*/
        }
        else{
            int step =frame->widthStep;
            int canals = frame->nChannels;
            uchar* imgData = (uchar*) frame->imageData;
            uchar* imgDataResult = (uchar*) result->imageData;

            int num_ma = 0;
            int num_venda = 0;
            int sum_ma[2] = {0,0};
            int sum_venda[2] = {0,0};


            int i = 0, j = 0;
            for( i = 0; i < frame->height; i++)
                for( j = 0; j < frame->width; j++)
                    if( ((imgData[i*step+j*canals]) >= blue[0] && (imgData[i*step+j*canals]) <= blue[1])
                    && ((imgData[i*step+j*canals+1]) >= green[0] && (imgData[i*step+j*canals+1]) <= green[1] )
                    && ((imgData[i*step+j*canals+2]) >= red[0] && (imgData[i*step+j*canals+2]) <= red[1] )){
                        /*Mà*/
                        imgDataResult[i*result->widthStep+j*result->nChannels]=255;
                        imgDataResult[i*result->widthStep+j*result->nChannels+1]=0;
                        imgDataResult[i*result->widthStep+j*result->nChannels+2]=0;
                        num_ma ++;
                        sum_ma[0] += j;
                        sum_ma[1] += i;
                    }

                    else if(((imgData[i*step+j*canals]) >= vblue[0] && (imgData[i*step+j*canals]) <= vblue[1])
                    && ((imgData[i*step+j*canals+1]) >= vgreen[0] && (imgData[i*step+j*canals+1]) <= vgreen[1] )
                    && ((imgData[i*step+j*canals+2]) >= vred[0] && (imgData[i*step+j*canals+2]) <= vred[1] )){
                        /*Venda*/
                        imgDataResult[i*result->widthStep+j*result->nChannels]=255;
                        imgDataResult[i*result->widthStep+j*result->nChannels+1]=255;
                        imgDataResult[i*result->widthStep+j*result->nChannels+2]=0;

                        num_venda++;
                        sum_venda[0] += j;
                        sum_venda[1] += i;
                    }
                    else{/*Background*/
                        imgDataResult[i*result->widthStep+j*result->nChannels]=0;
                        imgDataResult[i*result->widthStep+j*result->nChannels+1]=0;
                        imgDataResult[i*result->widthStep+j*result->nChannels+2]=0;
                    }

        cvSmooth(result, result, CV_MEDIAN, 3, 3 );

        if(num_ma > 0 && num_venda > 0){
            /*S'obté el punt central de la mà i la venda*/
            float mig[2] = {sum_ma[0]/num_ma,sum_ma[1]/num_ma};
            float vmig[2] = {sum_venda[0]/num_venda,sum_venda[1]/num_venda};
            cvRectangle( frame, cvPoint(mig[0], mig[1]), cvPoint(mig[0]+10, mig[1]+10), CV_RGB(0,0,0), -1 );
            cvRectangle( frame, cvPoint(vmig[0], vmig[1]), cvPoint(vmig[0]+10, vmig[1]+10), CV_RGB(0,0,0), -1 );

            /*Es calcula la distància entre els dos punts*/
            double dist = sqrt(pow(mig[0]-vmig[0],2) + pow(mig[1]-vmig[1], 2));

            /*Es normalitza la imatge, s'adapta la seva mida*/
            destination = cvCreateImage(cvSize((int)(frame->width*distOK/dist) , (int)(frame->height*distOK/dist) ),frame->depth, frame->nChannels );
            cvResize(result, destination, CV_INTER_LINEAR);

            int step =destination->widthStep;
            int canals = destination->nChannels;
            uchar* imgData = (uchar*) destination->imageData;

            /*S'obté el nombre de píxels que pertanyen a la mà*/
            int num_ma = 0;
            for( i = 0; i < destination->height; i++)
                for( j = 0; j < destination->width; j++)
                    if((imgData[i*step+j*canals]) == 255
                    && (imgData[i*step+j*canals+1]) == 0
                    && (imgData[i*step+j*canals+2]) == 0 ){
                        num_ma++;
                    }


            /*Comparació del gest actual amb la resta de possibles segons la seva àreea*/
            char num[1];
            strcpy(num, "0");
            if(num_ma > 14000) { /*5 dits*/
                strcpy(num, "5");
            }
            else if(num_ma > 10000) { /*4 dits*/
                strcpy(num, "4");
            }
            else if(num_ma > 7500) { /*3 dits*/
                strcpy(num, "3");
            }
            else if(num_ma > 6000) { /*2 dits*/
                strcpy(num, "2");
            }
            else if(num_ma > 4500) { /*1 dit*/
                strcpy(num, "1");
            }
            else { /*Cap dit*/
                strcpy(num, "0");
            }
            cvPutText(frame, num, cvPoint(10, 50), &font, cvScalar(255, 255, 255, 0));
        }

        }

        cvShowImage("Color detection", frame);
        cvShowImage("Binaria", result);
        }
    cvDestroyAllWindows();
    return 0;
}

/*Obtenció dels valors dels píxels que pertanyen a la mà*/
void calibrarMa(){
    int step =frame->widthStep;
    int canals = frame->nChannels;
    uchar* imgData = (uchar*) frame->imageData;
    uchar* imgDataResult = (uchar*) result->imageData;
        for( int i = 100; i < 200; i++)
            for(int  j = 200; j < 300; j++){
                if((imgData[i*step+j*canals]) < blue[0]) blue[0] = imgData[i*step+j*canals];
                else if((imgData[i*step+j*canals]) > blue[1]) blue[1] = imgData[i*step+j*canals];
                if((imgData[i*step+j*canals+1]) < green[0]) green[0] = imgData[i*step+j*canals+1];
                else if((imgData[i*step+j*canals]+1) > green[1]) green[1] = imgData[i*step+j*canals+1];
                if((imgData[i*step+j*canals+2]) < red[0]) red[0] = imgData[i*step+j*canals+2];
                else if((imgData[i*step+j*canals]+2) > red[1]) red[1] = imgData[i*step+j*canals+2];
            }
}

/*Obtenció dels valors dels píxels que pertanyen a la venda o polsera*/
void calibrarVenda(){
    int step =frame->widthStep;
    int canals = frame->nChannels;
    uchar* imgData = (uchar*) frame->imageData;
    uchar* imgDataResult = (uchar*) result->imageData;
        for( int i = 250; i < 300; i++)
            for(int  j = 200; j < 250; j++){
                if((imgData[i*step+j*canals]) < vblue[0]) vblue[0] = imgData[i*step+j*canals];
                else if((imgData[i*step+j*canals]) > vblue[1]) vblue[1] = imgData[i*step+j*canals];
                if((imgData[i*step+j*canals+1]) < vgreen[0]) vgreen[0] = imgData[i*step+j*canals+1];
                else if((imgData[i*step+j*canals]+1) > vgreen[1]) vgreen[1] = imgData[i*step+j*canals+1];
                if((imgData[i*step+j*canals+2]) < vred[0]) vred[0] = imgData[i*step+j*canals+2];
                else if((imgData[i*step+j*canals]+2) > vred[1]) vred[1] = imgData[i*step+j*canals+2];
            }
}

