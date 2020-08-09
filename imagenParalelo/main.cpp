#include <iostream>
#include <cstdlib>
#include <opencv4/opencv2/opencv_modules.hpp>
#include <opencv4/opencv2/photo.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core/core.hpp>
#include <opencv4/opencv2/core.hpp>
#include <mpi.h>
#include <string>
#include <vector>
#include <ctime>
#include <cmath>

using namespace cv;
using namespace std;

// Variables globales necesarias
Mat imagen;
Mat nueva_imagen;

//Funciones utilizadas


string nombreImagenSalida(string opcion){
    string path_imagen_salida = "/media/sf_Compartido/proyectoparalelo/imagenParalelo/";
    time_t hoy = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&hoy);
    strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", &tstruct);
    string fechayhora(buf);
    return (path_imagen_salida + "programa_" + opcion + "_" + fechayhora + ".png");
}

float interpolacionLineal(float valor_pixel, float valor_pixel_adyacente, float distancia){
    return valor_pixel + (valor_pixel_adyacente - valor_pixel) * distancia;
}

float interpolacionBilineal(float pixel00, float pixel10, float pixel01, float pixel11, float x, float y){
    return interpolacionLineal(interpolacionLineal(pixel00, pixel10, x), interpolacionLineal(pixel01, pixel11, x), y);
}

void difuminarImagen(Mat imagen, Mat nueva_imagen, float desviacion){
    double mascara[3][3];
    for(int i = 0; i<3; i++){
        for(int j = 0; j<3; j++){
            float elevar = exp(((-pow(i-1,2)-pow(j-1,2))/(2*pow(desviacion,2))));
            mascara[i][j]=elevar/(2*3.1416*pow(desviacion,2));
        }
    }
    for(int x = 0; x < imagen.cols; x++){
        for(int y = 0; y < imagen.rows; y++){
            for(int c = 0; c < 3; c++){
                float suma_Gauss = 0;
                for(int posicion_mascara_x = -1; posicion_mascara_x < 2; posicion_mascara_x++){
                    for(int posicion_mascara_y = -1; posicion_mascara_y < 2; posicion_mascara_y++){
                        if(posicion_mascara_x + x >= 0 && posicion_mascara_x + x < imagen.cols){
                            if(posicion_mascara_y + y >= 0 && posicion_mascara_y + y < imagen.rows){
                                suma_Gauss = suma_Gauss + ((float)(imagen.at<Vec3b>(y + posicion_mascara_y,x + posicion_mascara_x)[c]) * mascara[posicion_mascara_y+1][posicion_mascara_x+1]);
                            }
                            else{
                                suma_Gauss = suma_Gauss + ((float)(imagen.at<Vec3b>(y,x + posicion_mascara_x)[c]) * mascara[posicion_mascara_x+1][posicion_mascara_x+1]);
                            }
                        }
                        else{
                            if(posicion_mascara_y+y >= 0 && posicion_mascara_y+y < imagen.rows){
                                suma_Gauss = suma_Gauss + ((float)(imagen.at<Vec3b>(y + posicion_mascara_y,x)[c]) * mascara[posicion_mascara_y+1][posicion_mascara_x+1]);
                            }
                            else{
                                suma_Gauss = suma_Gauss + ((float)(imagen.at<Vec3b>(y,x)[c]) * mascara[posicion_mascara_y+1][posicion_mascara_x+1]);
                            }
                        }
                    }
                }
                nueva_imagen.at<Vec3b>(y,x)[c] = suma_Gauss;
            }
        }
    }
}

void escalagrisImagen(Mat imagen, Mat nueva_imagen){
    for(int x=0; x<imagen.cols; x++){
        for(int y=0; y<imagen.rows; y++){
            float valores_promedio = 0;
            valores_promedio = (imagen.at<Vec3b>(y,x)[0] + imagen.at<Vec3b>(y,x)[1] + imagen.at<Vec3b>(y,x)[2])/3;
            nueva_imagen.at<Vec3b>(y,x)[0] = valores_promedio;
            nueva_imagen.at<Vec3b>(y,x)[1] = valores_promedio;
            nueva_imagen.at<Vec3b>(y,x)[2] = valores_promedio;
        }
    }
}


void escalarImagenX2(Mat imagen, Mat nueva_imagen, int minimo_x, int minimo_y,int maximo_x, int maximo_y){
    for(int x = minimo_x; x < maximo_x; x++){
        for(int y = minimo_y; y < maximo_y; y++){
            float escaladox = ((float)(x) / nueva_imagen.cols) * (imagen.cols - 1);
            float escaladoy = ((float)(y) / nueva_imagen.rows) * (imagen.rows - 1);
            int escaladoxi = (int) escaladox;
            int escaladoyi = (int) escaladoy;
            int rojo = interpolacionBilineal(imagen.at<Vec3b>(escaladoyi, escaladoxi)[0], imagen.at<Vec3b>(escaladoyi + 1, escaladoxi)[0], imagen.at<Vec3b>(escaladoyi, escaladoxi + 1)[0], imagen.at<Vec3b>(escaladoyi + 1, escaladoxi + 1)[0], escaladox - escaladoxi, escaladoy - escaladoyi);
            int verde = interpolacionBilineal(imagen.at<Vec3b>(escaladoyi, escaladoxi)[1], imagen.at<Vec3b>(escaladoyi + 1, escaladoxi)[1], imagen.at<Vec3b>(escaladoyi, escaladoxi + 1)[1], imagen.at<Vec3b>(escaladoyi + 1, escaladoxi + 1)[1], escaladox - escaladoxi, escaladoy - escaladoyi);
            int azul = interpolacionBilineal(imagen.at<Vec3b>(escaladoyi, escaladoxi)[2], imagen.at<Vec3b>(escaladoyi + 1, escaladoxi)[2], imagen.at<Vec3b>(escaladoyi, escaladoxi + 1)[2], imagen.at<Vec3b>(escaladoyi + 1, escaladoxi + 1)[2], escaladox - escaladoxi, escaladoy - escaladoyi);
            nueva_imagen.at<Vec3b>(y, x)[0] = rojo;
            nueva_imagen.at<Vec3b>(y, x)[1] = verde;
            nueva_imagen.at<Vec3b>(y, x)[2] = azul;
        }
    }
}

int main(int argc, char** argv){
    if (argc > 2){
        int rango, procesadores;
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rango);
        MPI_Comm_size(MPI_COMM_WORLD, &procesadores);
        if(*argv[1] == '1'){
            if(rango == 0){
                float desviacion = 0.95;
                imagen = imread(argv[2], -1);
                nueva_imagen.create(imagen.rows, imagen.cols, CV_8UC3);
                difuminarImagen(imagen, nueva_imagen, desviacion);
                string salida = nombreImagenSalida("1");
                imwrite(salida, nueva_imagen);
            }
            /*else{
                
            }*/
        }
        if(*argv[1] == '2'){
            if(rango == 0){
                imagen = imread(argv[2], -1);
                nueva_imagen.create(imagen.rows, imagen.cols, CV_8UC3);
                escalagrisImagen(imagen, nueva_imagen);
                string salida = nombreImagenSalida("2");
                imwrite(salida, nueva_imagen);
            }
            /*else{
                
            }*/
        }
        if(*argv[1] == '3'){
            if(rango == 0){
                imagen = imread(argv[2], -1);
                nueva_imagen.create(imagen.rows, imagen.cols, CV_8UC3);
                escalarImagenX2(imagen, nueva_imagen, 0, 0, imagen.rows*2.0, imagen.cols*2.0);
                string salida = nombreImagenSalida("3");
                imwrite(salida, nueva_imagen);
            }
            /*else{
                
            }*/
        }
        if(*argv[1] != '1' && *argv[1] != '2' && *argv[1] != '3'){
            cout<<"No has ingresado la opción 1, 2 o 3"<<endl;
        }
        MPI_Finalize();
    }
    else{
        cout<<"Cantidad de argumentos incorrecta.";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

