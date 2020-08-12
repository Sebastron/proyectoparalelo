# Título del Proyecto

**_Proyecto Paralelo_**

## Descripción 🚀

_Este proyecto consiste en un programa que está implementado con OpenCV y OpenMPI, hecho_
_con lenguaje C++, donde se recibe una imágen y contiene 3 funcionalidades_
_distintas para modificarlo, las cuales son difuminar, aplicar la escala de_ 
_gris y aplicar la escala del tamaño (X2 por defecto) a la misma imágen._
_Las limitaciones del programa es que no puede trabajar con más de 8 procesadores,_
_y no puede recibir una imagen que supere 1920 del largo y ancho (1920x1920)._

### Requisitos 📋

* Tener instalado y configurado OpenMPI, se podrá instalar ingresando el siguiente comando en el terminal:
```
sudo apt-get install libopenmpi-dev
```
* Tener instalado compiladores y bibliotecas GCC/g ++, se podrá instalar ingresando el siguiente comando en el terminal:
```
sudo apt-get install build-essential
```
* Tener instalado OpenCV, se podrá instalar ingresando el siguiente comando en el terminal:
```
sudo apt-get install libopencv-dev
```

### Ejecución del programa 🔧

_Para ejecutar una de las opciones que contiene el programa, primero se debe posicionar_
_en la carpeta **imagenParalelo** del proyecto. Luego, se debe ingresar el siguiente comando_
_que ejecuta cmake, con el fin de localizar las librerias que se encuentran instaladas:_

```
cmake CMakeLists.txt
```
_Después, se debe ingresar el comando make para construir el proyecto en base al archivo main.cpp:_

```
make
```

_Por último, se debe ingresar el comando que se verá a continuación para ejecutar el programa, de acuerdo las_
_opciones que el usuario ingrese, donde 1 si desea difuminar la imagen, 2 si desea aplicar la escala de gris,
_y 3 si desea escalar el tamaño, que por defecto del programa es el doble (X2):_

```
mpirun --hostfile maquinas.txt programa [opcion] [Ruta de la imagen que se desea modificar]
```
_Ejemplo, con una imagen de prueba disponible en la carpeta **imagenes**, para la opción 1 se ingresa:_

```
mpirun --hostfile maquinas.txt programa 1 ./imagenes/prueba.jpg
```
_Para la opción 2:_

```
mpirun --hostfile maquinas.txt programa 2 ./imagenes/prueba.jpg
```

_Para la opción 3:_

```
mpirun --hostfile maquinas.txt programa 3 ./imagenes/prueba.jpg
```

_Los resultados de nuevas imagenes generadas estarán almacenados en la carpeta **imagenes**._

_**No olvidar** tener escrito la(s) ip(s) de la(s) maquina(s) que se utiliza(n)_
_en el archivo **maquinas.txt**_

## Herramientas 🛠️

* [Netbeans 8.2](https://netbeans.org/downloads/8.2/rc/) - Entorno de desarrollo integrado libre

## Integrantes del proyecto ✒️

* **Sebastián Garrido Valenzuela** - [Sebastron](https://github.com/Sebastron)
* **Ramiro Uribe Garrido** - [RamiroUribe](https://github.com/RamiroUribe)
* **Javier Gálvez González** - [javerix](https://github.com/javerix)
