#include <iostream>
#include <cstdlib>
#include <mpi.h>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <cmath>
#include <opencv4/opencv2/core/mat.hpp>
#include <opencv4/opencv2/opencv_modules.hpp>
#include <opencv4/opencv2/photo.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core/core.hpp>
#include <opencv4/opencv2/core.hpp>

using namespace cv;
using namespace std;

// Variables globales necesarias
const int maximo_bytes=8*1920*1920;
uchar buffer[maximo_bytes];
float escala = 2.0; // Valor para escalar una imagen, en este caso, escala de x2 
float desviacion = 0.99; // Se utiliza para difuminar una imagen

//Funciones utilizadas

void integrantes(){
    cout<<"-----------------------------------------"<<endl;
    cout<<"Sebastian Garrido Valenzuela."<<endl;
    cout<<"Javier Galvez Gonzalez."<<endl;
    cout<<"Ramiro Uribe Garrido."<<endl;
    cout<<"-----------------------------------------"<<endl;
    cout<<"Gracias por utilizar el programa y espero que le guste, no ha sido fácil construirla."<<endl;
    cout<<"-----------------------------------------"<<endl;
}


void enviarImagen(Mat imagen_enviar, int destino){
    int filas = imagen_enviar.rows;
    int columnas = imagen_enviar.cols;
    int tipo = imagen_enviar.type();
    int canales = imagen_enviar.channels();
    memcpy(&buffer[0 * sizeof(int)], (uchar*)&filas, sizeof(int));
    memcpy(&buffer[1 * sizeof(int)], (uchar*)&columnas, sizeof(int));
    memcpy(&buffer[2 * sizeof(int)], (uchar*)&tipo, sizeof(int));
    int bytes = filas*columnas*canales;
    if (imagen_enviar.isContinuous()) {
        imagen_enviar = imagen_enviar.clone();
    }
    memcpy(&buffer[3*sizeof(int)], imagen_enviar.data, bytes);
    MPI_Send(&buffer, bytes+3*sizeof(int), MPI_UNSIGNED_CHAR, destino, 0, MPI_COMM_WORLD);
}

Mat recibirImagen(int origen){
    MPI_Status status;
    int contador, filas, columnas, tipo;
    MPI_Recv(&buffer, sizeof(buffer), MPI_UNSIGNED_CHAR, origen, 0, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_UNSIGNED_CHAR, &contador);
    memcpy((uchar*)&filas, &buffer[0 * sizeof(int)], sizeof(int));
    memcpy((uchar*)&columnas, &buffer[1 * sizeof(int)], sizeof(int));
    memcpy((uchar*)&tipo, &buffer[2 * sizeof(int)], sizeof(int));
    return Mat(filas, columnas, tipo, (uchar*)&buffer[3*sizeof(int)]);
}


string nombreImagenSalida(char opcion){
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

Mat procesarImagen(char opcion, Mat parte_imagen, float escala){
    Mat parte_procesada = parte_imagen.clone(); // Se inicializa una parte de la imágen final
    if (opcion == '1') { // Se difumina a una parte de la imagen
        difuminarImagen(parte_imagen, parte_procesada, desviacion);
    }
    if (opcion == '2') { // Se aplica escala a gris a una parte de la imagen
        escalagrisImagen(parte_imagen, parte_procesada);
    }
    if (opcion == '3') { // Se aplica escala de tamaño a una parte de la imagen
        resize(parte_imagen, parte_procesada, Size(parte_imagen.cols*escala, parte_imagen.rows*escala), escala -1, escala -1); // Funcion nativa de opencv
    }
    return parte_procesada;
}

void unirImagen(Mat parte_imagen, Mat & final) {
    if (final.empty()) {
        final = parte_imagen.clone();
    }
    else {
        hconcat(final, parte_imagen, final);
    }
}


// Función Main y de MPI
int main(int argc, char** argv){
    if (argc > 2){ // Se requiere ingresa comando con mas de 2 argumentos, vease a README.md
        if(*argv[1] != '1' && *argv[1] != '2' && *argv[1] != '3'){ // En caso de no ingresar la opcion correcta
            cout << "No has ingresado la opción 1, 2 o 3" << endl;
            return EXIT_FAILURE;
        }
        int rango, procesadores;
        MPI_Init(&argc, &argv); // Se inicializa el MPI
        MPI_Comm_rank(MPI_COMM_WORLD, &rango); // Se identifica el rango
        MPI_Comm_size(MPI_COMM_WORLD, &procesadores); // Se identifica los procesadores
        Mat imagen = imread(argv[2], -1); // Se lee la imagen a la ruta ingresada
        if (imagen.empty()) { // En caso de que la imagen no sea valida o no se haya ingresado.
            cout << "Usted no ha ingresado la ruta de la imagen existente" << endl;
            return EXIT_FAILURE;
        }
        if(!imagen.data){ // En caso de que la imagen no sea valida el formato valido.
            cout << "Usted ha ingresado una imágen invalida." << endl;
            return EXIT_FAILURE;
        }
        int longitud_rebanada = imagen.cols/(procesadores-1); // Divido la imagen en base a la cantidad de procesadores disponibles
        if (rango == 0) { // En caso de ser el procesador maestro, se distribuye las tareas
            Mat nueva_imagen; // Se inicializa la nueva imágen
            int hilox = 1; // Se inicializa el contador para asignar tareas a los hilos
            for (int i=0; i< imagen.cols; i = i + longitud_rebanada){ // Empieza el proceso por partes 
                Mat imagen_dividida = imagen(Rect(i, 0, longitud_rebanada, imagen.rows)).clone(); // Se inicializa la porción de una imagen a procesar
                enviarImagen(imagen_dividida, hilox); // Se lo envio al hilo correspondiente
                Mat parte_imagen_procesada = recibirImagen(hilox); // Recibo porcion procesada
                unirImagen(parte_imagen_procesada, nueva_imagen);// Empiezo a generar la imagen final
                hilox++;// Se pasa otra tarea para el siguiente hilo
            }
            string salida = nombreImagenSalida(*argv[1]); // Se obtiene el nombre del archivo PNG de salida
            imwrite(salida, nueva_imagen); // Se procede a escribir la imágen final y procesada en su totalidad
        }
        else{ // En caso de ser un procesador esclavo
            Mat imagen_cortada = recibirImagen(0); // Se recibe una parte de imagen desde el procesador maestro
            Mat imagen_cortada_terminada = procesarImagen(*argv[1], imagen_cortada, escala);  // Se procesa dicha imagen de acuerdo la opción ingresada
            enviarImagen(imagen_cortada_terminada, 0); // Se envia la parte de imagen procesada al procesador maestro
        }
        if(rango == 0){
            cout<<"Nueva imagen generada con exito."<<endl;
            integrantes();
        }
        MPI_Finalize(); // Se finaliza MPI
        
    }
    else{
        cout<<"Cantidad de argumentos incorrecta."<<endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

