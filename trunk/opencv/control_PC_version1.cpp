#include <stdio.h>
#include "cv.h"
#include "highgui.h"
#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <windowsx.h>


/*Valors per defecte: color gris fosc i tolerància 40*/
int red = 20;
int blue = 25;
int green = 20;
int tolerance = 40;

int const MIDAPUNT = 3;
/*[0] = Posició x, [1] Posició y, [2] = temps */
int item[3] = {-1, -1, 0};
/*Número de veins d'un píxel a considerar */
int llindar = 10;
/*Indica si el botó està pres o no*/
boolean drag = false;

IplImage* frame;
IplImage* result;

int main(int argc, char* argv[])
{
    cvNamedWindow("Frame",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Binary",CV_WINDOW_AUTOSIZE);
    CvCapture* capture=cvCaptureFromCAM(0);
    frame = cvQueryFrame(capture);
    result = cvCreateImage( cvGetSize(frame), 8, 1 );
    cvCreateTrackbar( "Tolerancia", "Frame", &tolerance, 150, NULL);

    int input;
    while( (input = cvWaitKey(50))!= 32 )
    {
        frame = cvQueryFrame(capture);
        int step =frame->widthStep;
        int channels = frame->nChannels;
        uchar* imgData = (uchar*) frame->imageData;
        uchar* imgDataResult = (uchar*) result->imageData;

        /*Binarització de la imatge*/
        int i, j;
        for( i = 0; i < frame->height; i++)
            for( j = 0; j < frame->width; j++)
                if( ((imgData[i*step+j*channels]) <= blue + tolerance && (imgData[i*step+j*channels]) >= blue - tolerance )
                        && ((imgData[i*step+j*channels+1]) <= green + tolerance && (imgData[i*step+j*channels+1]) >= green - tolerance )
                        && ((imgData[i*step+j*channels+2]) <= red + tolerance && (imgData[i*step+j*channels+2]) >= red - tolerance ))
                {
                    imgDataResult[i*result->widthStep+j*result->nChannels]=255;
                }
                else imgDataResult[i*result->widthStep+j*result->nChannels]=0;


        /*Suavitzat de la imatge per reduir el soroll*/
        cvSmooth(result, result, CV_MEDIAN, 3, 3 );

        /*S'obté informació sobre la imatge binària*/
        int minY = frame->height;
        int xminY = 0;
        int num = 0;
        int avgX = 0;
        int avgY = 0;
        for( i = 0; i < result->height; i++)
            for( j = 0; j < result->width; j++)
                if(imgDataResult[i*result->widthStep+j*result->nChannels] == 255)
                {
                    if(i < minY)
                    {
                        minY = i;
                        xminY = j;
                    }
                    avgX+=j;
                    avgY+=i;
                    num++;
                }
        /*Es calcular el centre (centroid) de la mà*/
        if(num>0)avgX/=num;
        if(num>0)avgY/=num;

        /*Indica el color pel que es binaritza la imatge*/
        cvRectangle(frame, cvPoint(frame->width - 60,10), cvPoint(frame->width - 10,60), cvScalar(blue,green,red), -1);

        /*Si s'ha trobat la mà en el frame actual*/
        if(xminY > 0 && num > 0)
        {
            /*Es dibuixa el centroid i el punter*/
            cvRectangle(frame, cvPoint(avgX-MIDAPUNT,avgY-MIDAPUNT), cvPoint(avgX+MIDAPUNT,avgY+MIDAPUNT), cvScalar(0,0,255), -1);
            cvRectangle(frame, cvPoint(xminY-MIDAPUNT,minY-MIDAPUNT), cvPoint(xminY+MIDAPUNT,minY+MIDAPUNT), cvScalar(0,0,255), -1);
            cvLine(frame, cvPoint(avgX, avgY), cvPoint(xminY, minY), CV_RGB(255,255,255), 1, 4, 0);
            /*Crida per situar el punter a la posició desitjada*/
            SetCursorPos(xminY*1366/640 ,minY*768/480);

            /*Si el punter no s'ha mogut ( dins d'un interval )*/
            if(item[0] - llindar < xminY && item[0] + llindar > xminY)
            {
                if(item[1] - llindar < minY && item[1] + llindar > minY)
                {
                    /*Incrementar el temps en que el punter es manté a la mateixa posició*/
                    item[2]++;
                }
            }
            /*Si el punter ha estat durant 20 frames a la mateixa posició*/
            if(item[2] == 20)
            {
                /*S'obtenen les coordenades del ratolí*/
                item[2] = 0;
                POINT cursorPos;
                GetCursorPos(&cursorPos);
                float x = cursorPos.x;
                float y = cursorPos.y;
                if(drag)
                {
                    /*Si el botó estava premut, s'aixeca*/
                    mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP, x, y,0,0);
                    drag = false;
                }
                else
                {
                    /*Si el botó estava aixecat, es prem*/
                    mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN, x, y,0,0);
                    drag = true;
                }
            }
        }

        item[0] = xminY;
        item[1] = minY;

        cvShowImage("Frame", frame);
        cvShowImage("Binary", result);
    }
    cvDestroyAllWindows();
    return 0;
}
