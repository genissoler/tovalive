#include "cv.h"
#include "highgui.h"

/*Valors per defecte: color groc i tolerància 50*/
int red = 225;
int blue = 65;
int green = 225;
int tolerance = 50;

IplImage* frame; /*Imatge original obtinguda de la webcam*/
IplImage* result; /*Imatge binaria: segmentació per color*/

int main(int argc, char* argv[]){
    /*Es creen les finestres actives que s'utilitzaran per mostrar les imatges*/
    cvNamedWindow("Color detection",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Binaria",CV_WINDOW_AUTOSIZE);
    /*Obtenim la font de vídeo de la webcam*/
    CvCapture* capture=cvCaptureFromCAM(0);
    frame = cvQueryFrame(capture);
    /*Inicialitzem l'altra imatge creant la seva estrucutura: indicant-li que tingui la
    mateixa mida que 'frame' i que només tingui 1 canal ( Blanc i negre )*/
    result = cvCreateImage( cvGetSize(frame), 8, 1 );


    int input;
    /*Capturem cada frame del senyal de vídeo*/
    while( (input = cvWaitKey(50))!= 27 ) {
        /*L'aplicació finalitza al prémer la tecla 'ESC'*/
        frame = cvQueryFrame(capture);

        /*Variables de l'estructura de les imatges per recórrer el seu contingut*/
        int step =frame->widthStep;
        int canals = frame->nChannels;
        uchar* imgData = (uchar*) frame->imageData;
        uchar* imgDataResult = (uchar*) result->imageData;

        int i = 0, j = 0; /*Índexs del recorregut*/
        /*Fem un recorregut de la imatge original, pixel a pixel. Si cada canal està dins del interval
        [valor-tolerància, valor+tolerància] pintem aquell pixel de la imatge binària resultant de blanc.
        En cas contrari, de negre.*/
        for( i = 0; i < frame->height; i++)
            for( j = 0; j < frame->width; j++) /*Si els 3 canals compleixen la condició: blanc*/
                if( ((imgData[i*step+j*canals]) <= blue + tolerance && (imgData[i*step+j*canals]) >= blue - tolerance )
                        && ((imgData[i*step+j*canals+1]) <= green + tolerance && (imgData[i*step+j*canals+1]) >= green - tolerance )
                        && ((imgData[i*step+j*canals+2]) <= red + tolerance && (imgData[i*step+j*canals+2]) >= red - tolerance ))
                            imgDataResult[i*result->widthStep+j*result->nChannels]=255;
                else imgDataResult[i*result->widthStep+j*result->nChannels]=0; /*Si algun canal no ho compleix: negre*/

        /*En la imatge binaria, apliquem un efecte de suavitzat*/
        cvSmooth(result, result, CV_MEDIAN, 9, 9 );


        /*Recorrem la imatge binaria per trobar la posició del objecte a seguir*/
        int minX = frame->width;
        int minY = frame->height;
        int maxX = 0;
        int maxY = 0;
        for( i = 0; i < result->height; i++)
            for( j = 0; j < result->width; j++)
                if(imgDataResult[i*result->widthStep+j*result->nChannels] == 255) {
                    /*Obtenim les posicions*/
                    if(j < minX) minX = j;
                    if(j > maxX) maxX = j;
                    if(i < minY) minY = i;
                    if(i > maxY) maxY = i;
                }


         /*Emmarquem l'objecte identificat amb dins d'un rectangle*/
         if(minX != frame->width){
            int diffX = maxX-minX;
            int diffY = maxY-minY;
            int diff = (diffX < diffY)? diffX*0.3: diffY*0.3;
            if(diff>5) diff=5;
            /*Rectangle*/
            cvRectangle( frame, cvPoint(minX, minY), cvPoint(maxX, maxY), CV_RGB(255,0,0), 2 );
            /*Diagonals*/
            cvLine(frame, cvPoint(minX, minY), cvPoint(maxX, maxY), CV_RGB(255,255,255), 1, 4, 0);
            cvLine(frame, cvPoint(maxX, minY), cvPoint(minX, maxY), CV_RGB(255,255,255), 1, 4, 0);
            /*Creu*/
            cvLine(frame, cvPoint(maxX-(diffX/2)-diff, maxY-(diffY/2)), cvPoint(maxX-(diffX/2)+diff, maxY-(diffY/2)), CV_RGB(255,0,0), 1, 8, 0);
            cvLine(frame, cvPoint(maxX-(diffX/2), maxY-(diffY/2)-diff), cvPoint(maxX-(diffX/2), maxY-(diffY/2)+diff), CV_RGB(255,0,0), 1, 8, 0);
        }

        /*Mostrem les imatges a les finestres corresponent*/
        cvShowImage("Color detection", frame);
        cvShowImage("Binaria", result);
        }
    /*Tanquem totes les finestres*/
    cvDestroyAllWindows();
    return 0;
}

