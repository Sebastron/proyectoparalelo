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
#include <opencv4/opencv2/core/mat.hpp>

using namespace cv;
using namespace std;

// Variables globales necesarias
Mat imagen; // Imagen de entrada
Mat nueva_imagen; // Imagen de salida
float escala = 2.0; // Valor para escalar una imagen, en este caso, escala de x2 
float desviacion = 0.99; // Se utiliza para difuminar una imagen

//Funciones utilizadas


void enviarImagen(Mat imagen_enviar, int destino){
    int tamanos[3];
    tamanos[2] = imagen_enviar.elemSize();
    Size tamano = imagen_enviar.size();
    tamanos[0] = tamano.height;
    tamanos[1] = tamano.width;
    MPI_Send(tamanos, 3, MPI_INT, destino, 0, MPI_COMM_WORLD);
    MPI_Send(imagen_enviar.data, tamanos[0]*tamanos[1]*4, MPI_CHAR, destino, 1, MPI_COMM_WORLD);
}

void recibirImagen(Mat &imagen_recibir, int origen){
    MPI_Status estado;
    int tamanos[3];
    MPI_Recv(tamanos, 3, MPI_INT, origen, 0, MPI_COMM_WORLD, &estado);
    imagen_recibir.create(tamanos[0], tamanos[1], CV_8UC3);
    MPI_Recv(imagen_recibir.data, tamanos[0] * tamanos[1] * 4, MPI_CHAR, origen, 1, MPI_COMM_WORLD, &estado);
}


string nombreImagenSalida(string opcion){
    string path_imagen_salida = "./imagenes/";
    time_t hoy = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&hoy);
    strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", &tstruct);
    string fechayhora(buf);
    return (path_imagen_salida + "programa_" + opcion + "_" + fechayhora + ".png");
}

void difuminarImagen(Mat imagen, Mat nueva_imagen, float desviacion){
    double mascara[3][3];
    for(int i = 0; i<3; i++){
        for(int j = 0; j<3; j++){
            float elevar = exp(((-pow(i-1,2)-pow(j-1,2))/(2*pow(desviacion,2))));
            mascara[i][j] = elevar/(2*3.1416*pow(desviacion,2));
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

void escalarImagen(Mat imagen, Mat nueva_imagen, float escala){
    resize(imagen, nueva_imagen, Size(imagen.cols*escala, imagen.rows*escala), escala - 1, escala - 1); // Funcion nativa de opencv
}


// Función Main y de MPI
int main(int argc, char** argv){
    if (argc > 2){
        int rango, procesadores;
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rango);
        MPI_Comm_size(MPI_COMM_WORLD, &procesadores);
        /*if(*argv[1] == '1'){
            cout << rango << endl;
        }
        */if(*argv[1] == '1'){
            if(rango == 0){
                imagen = imread(argv[2], -1);
                nueva_imagen.create(imagen.rows, imagen.cols, CV_8UC3);
                difuminarImagen(imagen, nueva_imagen, desviacion);
                enviarImagen(nueva_imagen, 1);
            }
            else{
                recibirImagen(nueva_imagen, 0);
                string salida = nombreImagenSalida("1");
                imwrite(salida, nueva_imagen);
            }
        }
        if(*argv[1] == '2'){
            if(rango == 0){
                imagen = imread(argv[2], -1);
                nueva_imagen.create(imagen.rows, imagen.cols, CV_8UC3);
                escalagrisImagen(imagen, nueva_imagen);
                enviarImagen(nueva_imagen, 1);
            }
            else{
                recibirImagen(nueva_imagen, 0);
                string salida = nombreImagenSalida("2");
                imwrite(salida, nueva_imagen);
            }
        }
        if(*argv[1] == '3'){
            if(rango == 0){
                imagen = imread(argv[2], -1);
                nueva_imagen.create(imagen.rows*escala, imagen.cols*escala, CV_8UC3);
                escalarImagen(imagen, nueva_imagen, escala);
                enviarImagen(nueva_imagen, 1);
            }
            else{
                recibirImagen(nueva_imagen, 0);
                string salida = nombreImagenSalida("3");
                imwrite(salida, nueva_imagen);
            }
        }
        if(*argv[1] != '1' && *argv[1] != '2' && *argv[1] != '3'){
            cout << "No has ingresado la opción 1, 2 o 3" << endl;
            return EXIT_FAILURE;
        }//*/
        MPI_Finalize();
    }
    else{
        cout<<"Cantidad de argumentos incorrecta.";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

