#include "cv.h"
#include "highgui.h"
#include <math.h>

void dibuixarHistograma(float);
void dibuixarHistogramaSuavizat(float, int*);
int llindar_gaussian(IplImage*, CvHistogram*);

IplImage* imgHistogram;
IplImage* imatge;
CvHistogram* hist;
int bins = 256;


int main(int argc, char* argv[])
{
    cvNamedWindow("Original",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Binaria",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Histograma",CV_WINDOW_AUTOSIZE);
    imatge = cvLoadImage("C:\\EUPMT\\Projects\\OpenCV_Contours\\exemple03.jpg", 0);
    IplImage* binaria = cvCreateImage(cvGetSize(imatge), 8, 1);
    imgHistogram = 0;


    int hsize[] = {bins};
    float max_value = 0, min_value = 0;
    float xranges[] = { 0, 256 };
    float* ranges[] = { xranges };

    hist = cvCreateHist( 1, hsize, CV_HIST_ARRAY, ranges,1);
    cvCalcHist( &imatge, hist, 0, NULL);
    cvGetMinMaxHistValue( hist, &min_value, &max_value);

    dibuixarHistograma(max_value);


    int llindar = llindar_gaussian(imatge, hist);

    int step =imatge->widthStep;
    int canals = imatge->nChannels;
    uchar* imgData = (uchar*) imatge->imageData;
    uchar* imgDataBinaria = (uchar*) binaria->imageData;

    for( int i = 0; i < imatge->height; i++)
        for( int j = 0; j < imatge->width; j++)
        {
            if((imgData[i*step+j*canals]) > llindar)
            {
                imgDataBinaria[i*binaria->widthStep+j*binaria->nChannels]=0;
            }
            else
            {
                imgDataBinaria[i*binaria->widthStep+j*binaria->nChannels]=255;
            }
        }


    cvShowImage("Original", imatge);
    cvShowImage("Histograma", imgHistogram);
    cvShowImage("Binaria", binaria);
    cvWaitKey(0);
    cvDestroyAllWindows();
    return 0;
}

int llindar_gaussian(IplImage* img, CvHistogram* histo)
{
    int i = 0;
    int aux = 0;
    int maxPrimer = 0;
    int idxMaxPrimer = 0;
    int maxSegon = 0;
    int idxMaxSegon = 0;

    /*Obtenció del màxim absolut*/
    while (i < bins)
    {
        aux = cvQueryHistValue_1D( hist, i);
        if( aux > maxPrimer) {
            maxPrimer = aux;
            idxMaxPrimer = i;
        }
        i++;
    }
    /*Obtenció del segon màxim absolut*/
    i=0;
    while (i < bins)
    {
        aux = cvQueryHistValue_1D( hist, i);
        if((pow((i-idxMaxPrimer),2) * aux) > maxSegon){
            maxSegon = pow((i-idxMaxPrimer),2)*aux;
            idxMaxSegon = i;
        }
        i++;
    }
    /*Suavització del histograma*/
    i=0;
    int* histValors = new int[256];
    int suavizat = 3;

    while (i < bins)
    {
        int suma = 0;
        for(int k = i-(suavizat-1/2); k <= i+(suavizat-1/2); k++)
        {
            if(k>=0 && k < bins)
            {
                suma+= cvQueryHistValue_1D( hist, k);
            }
            histValors[i] = suma/suavizat;
        }
        i++;
    }

    dibuixarHistogramaSuavizat(maxPrimer, histValors);

    /*En aquest cas, l'objecte és més fosc que el fons*/
    /*Càlcul del llindar: es busca una pendent positiva superior a 200*/
    int j;
    int finestra = 10;
    boolean trobat = false;
    for(j=idxMaxSegon+finestra; j<idxMaxPrimer && !trobat;j++){
        int aux = histValors[j] - histValors[j-finestra];
        if(aux > 200) trobat = true;
    }
    return j - finestra/2;
}

void dibuixarHistograma(float max_value)
{
    float value;
    int normalized;
    imgHistogram = cvCreateImage(cvSize(bins*2, 125),8,1);
    cvRectangle(imgHistogram, cvPoint(0,0), cvPoint(bins*2,125), CV_RGB(255,255,255),-1);
    for(int i=0; i < bins; i++)
    {
        value = cvQueryHistValue_1D( hist, i);
        normalized = cvRound(value*100/max_value);
        cvLine(imgHistogram,cvPoint(i*2,100), cvPoint(i*2,100-normalized), CV_RGB(0,0,0));
    }
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.4, 0, 0.1, CV_AA);
    cvPutText(imgHistogram, "0", cvPoint(5, 120), &font, cvScalar(0, 0, 0, 0));
    cvPutText(imgHistogram, "50", cvPoint(100-10, 120), &font, cvScalar(0, 0, 0, 0));
    cvPutText(imgHistogram, "100", cvPoint(200-15, 120), &font, cvScalar(0, 0, 0, 0));
    cvPutText(imgHistogram, "150", cvPoint(300-15, 120), &font, cvScalar(0, 0, 0, 0));
    cvPutText(imgHistogram, "200", cvPoint(400-15, 120), &font, cvScalar(0, 0, 0, 0));
    cvPutText(imgHistogram, "256", cvPoint(250*2-15, 120), &font, cvScalar(0, 0, 0, 0));
}

void dibuixarHistogramaSuavizat(float maxValor, int* histValors)
{
    float value;
    int normalized;
    IplImage* hist2 = cvCreateImage(cvSize(bins*2, 125),8,1);
    cvRectangle(hist2, cvPoint(0,0), cvPoint(bins*2,125), CV_RGB(255,255,255),-1);
    for(int i=0; i < bins; i++)
    {
        value = histValors[i];
        normalized = cvRound(value*100/maxValor);
        cvLine(hist2,cvPoint(i*2,100), cvPoint(i*2,100-normalized), CV_RGB(0,0,0));
    }
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.4, 0, 0.1, CV_AA);
    cvPutText(hist2, "0", cvPoint(5, 120), &font, cvScalar(0, 0, 0, 0));
    cvPutText(hist2, "50", cvPoint(100-10, 120), &font, cvScalar(0, 0, 0, 0));
    cvPutText(hist2, "100", cvPoint(200-15, 120), &font, cvScalar(0, 0, 0, 0));
    cvPutText(hist2, "150", cvPoint(300-15, 120), &font, cvScalar(0, 0, 0, 0));
    cvPutText(hist2, "200", cvPoint(400-15, 120), &font, cvScalar(0, 0, 0, 0));
    cvPutText(hist2, "256", cvPoint(250*2-15, 120), &font, cvScalar(0, 0, 0, 0));
    cvShowImage("Histograma suavizat",hist2);
}


