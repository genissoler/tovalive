#include <stdio.h>
#include <string>
#include <sstream>
#include "cv.h"
#include "highgui.h"
#include "audiere.h"

using namespace std;
using namespace audiere;

/*Valors per defecte: color gris fosc i tolerància 40*/
int red = 20;
int blue = 25;
int green = 20;
int tolerance = 50;

int const MIDAPUNT = 3;
/*Número de veins d'un píxel a considerar */
int llindar = 10;

int temps = 30;

/*Array amb la posició x de cada tecla*/
int teclesX[10] = {50, 105, 160, 215, 270, 325, 380, 435, 490, 545};
int teclaY = 100;
/*Array que indica el temps que cada tecla ha estat presionada*/
int teclesDown[10] = {0,0,0,0,0,0,0,0,0,0};
/*Array amb un audio per cada tecla*/
OutputStreamPtr audio[10] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

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

    /*Es carrega la imatge corresponent a la tecla*/
    IplImage* tecla = cvLoadImage("C:\\EUPMT\\Projects\\HandGesture\\piano.jpg");

    /*Path dels arxius .mp3*/
    string filename = "C:\\EUPMT\\Projects\\HandGesture\\tecla";
    AudioDevicePtr device(OpenDevice());


    /*Es carreguen tots els audios*/
    for(int i=0; i<10 ; i++)
    {
        std::string s;
        std::stringstream out;
        out << i+1;
        string aux = filename +out.str() + ".mp3";
        OutputStreamPtr sound(OpenSound(device, aux.c_str(), false));
        audio[i] = sound;
    }

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
        if(xminY > 0 && num > 0)
        {
            /*Es dibuixa el centroid i el punter*/
            cvRectangle(frame, cvPoint(avgX-MIDAPUNT,avgY-MIDAPUNT), cvPoint(avgX+MIDAPUNT,avgY+MIDAPUNT), cvScalar(0,0,255), -1);
            cvRectangle(frame, cvPoint(xminY-MIDAPUNT,minY-MIDAPUNT), cvPoint(xminY+MIDAPUNT,minY+MIDAPUNT), cvScalar(0,0,255), -1);
            cvLine(frame, cvPoint(avgX, avgY), cvPoint(xminY, minY), CV_RGB(255,255,255), 1, 4, 0);

            int numPunter = 0;
            int avgXPunter = 0;
            int avgYPunter = 0;
            for( i = minY-llindar; i < minY+llindar; i++)
                for( j = xminY-llindar; j < xminY+llindar; j++)
                    if(imgDataResult[i*result->widthStep+j*result->nChannels] == 255)
                    {
                        avgXPunter+=j;
                        avgYPunter+=i;
                        numPunter++;
                    }

            if(numPunter>0)avgXPunter/=numPunter;
            if(numPunter>0)avgYPunter/=numPunter;
            cvRectangle(frame, cvPoint(avgXPunter-MIDAPUNT*2,avgYPunter-MIDAPUNT*2), cvPoint(avgXPunter+MIDAPUNT*2,avgYPunter+MIDAPUNT*2), cvScalar(0,255,255), -1);

            for(i=0; i<10 ; i++)
            {
                if(avgXPunter > teclesX[i] && avgXPunter < teclesX[i]+tecla->width &&
                        avgYPunter > teclaY && avgYPunter < teclaY+tecla->height)
                {
                    if(teclesDown[i] == 0) /*Si no ha estat presionada recentment, reproduir so*/
                    {
                        teclesDown[i]=temps;
                        audio[i]->play();
                    }
                    else teclesDown[i]--; /*En cas contrari, reduir el temps*/

                }
                else teclesDown[i]=0;


            }

        }

        /*Es mostren totes les tecles*/
        float alpha = 0.6;
        float beta = 0.3;
        for(i=0; i<10; i++)
        {
            /*Es crea un Regió de Interès*/
            cvSetImageROI(frame, cvRect(teclesX[i],teclaY, tecla->width, tecla->height));
            if(teclesDown[i] ==  temps) /*Si està presionada la tecla, opacitat 1*/
            {
                cvAddWeighted(tecla, 1, frame, 0, 0, frame);
            }
            else /*En cas contrari, semitransparent*/
            {
                cvAddWeighted(tecla, alpha, frame, beta, 0, frame);
            }

        }
        /*Alliberar la ROI*/
        cvResetImageROI(frame);

        cvShowImage("Frame", frame);
        cvShowImage("Binary", result);
    }
    cvDestroyAllWindows();
    return 0;
}
