#include "cv.h"
#include "highgui.h"

void dibuixarHistograma(float);
int llindar_plite(IplImage*, CvHistogram*, float );

IplImage* imgHistogram;
IplImage* imatge;
CvHistogram* hist;
int bins = 256;


int main(int argc, char* argv[])
{
    cvNamedWindow("Original",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Binaria",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Histograma",CV_WINDOW_AUTOSIZE);
    /*Es carrega la imatge*/
    imatge = cvLoadImage("C:\\EUPMT\\Projects\\OpenCV_Contours\\ex_plite.png", 0);
    IplImage* binaria = cvCreateImage(cvGetSize(imatge), 8, 1);
    imgHistogram = 0;


    int hsize[] = {bins};
    float max_value = 0, min_value = 0;
    float xranges[] = { 0, 256 };
    float* ranges[] = { xranges };

    /*Es calcula l'histograma*/
    hist = cvCreateHist( 1, hsize, CV_HIST_ARRAY, ranges,1);
    cvCalcHist( &imatge, hist, 0, NULL);
    cvGetMinMaxHistValue( hist, &min_value, &max_value);
    dibuixarHistograma(max_value);


    /*S'obté el llindar amb el mètode P-lite*/
    int llindar = llindar_plite(imatge, hist, 0.15);

    /*Es segmenta la imatge a partir del llindar calculat*/
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

int llindar_plite(IplImage* img, CvHistogram* histo, float p )
{
    /*Percentatge de píxels pertanyents a l'objecte*/
    int i = 0, count = 0, n = img->width*img->height*p;

    while ((i < bins) && (count < n))
    {
        count += cvQueryHistValue_1D( hist, i);
        i++;
    }
    return i-1;
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

