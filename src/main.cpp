/*
 * main.cpp - Chat multiusuario basado en memoria compartida
 *
 * Este programa de Adrián Rodríguez Bazaga <alu0100826456@ull.edu.es> está bajo una Licencia
 * Creative Commons Public Domain Dedication 1.0.
 *
 *     http://creativecommons.org/publicdomain/zero/1.0/deed.es
 */

#include "chatroom.h"
#include <iostream>
#include <cstring>

//
// Función principal o punto de entrada del programa
//

int main(int argc, char* argv[])
{

    std::string chatRoomId;

    ChatRoom chatRoom;

    std::string nombre;

    if(argc < 2)
    {
        std::cout << "::: Uso del programa incorrecto, usa la ayuda con -h o --help" << std::endl;
        exit(0);
    }
    else
    {
        if(argc == 2)
        {
            if(std::strcmp(argv[1], "-h") == 0 || std::strcmp(argv[1], "--help") == 0)
            {
                nombre = std::getenv("USER");
                std::cout << "\n\033[1;32mUSO DEL PROGRAMA: \n\n1) ./ssoo-shmchat <NOMBRE_SALA> -u <NOMBRE_USUARIO>\n2) ./ssoo-shmchat <NOMBRE_SALA>\n\nUna vez conectado al chat, escribe --help para mas ayuda.\n\033[0m" << std::endl;
                exit(0);
            }
            else
            {
                std::cout << "::: Uso del programa incorrecto, usa la ayuda con -h o --help" << std::endl;
                exit(0);
            }
        }
        if(argc == 3)
        {
            std::cout << "::: Uso del programa incorrecto, usa la ayuda con -h o --help" << std::endl;
            exit(0);
        }
        if(argc == 4)
        {
            if(std::strcmp(argv[2], "-u") != 0)
            {
                std::cout << "::: Uso del programa incorrecto, usa la ayuda con -h o --help" << std::endl;
                exit(0);
            }
            else
            {
                std::string nombre_usuario(argv[3]);
                chatRoom.setUserName(nombre_usuario);
                chatRoomId = argv[1];
            }
        }
    }

    //

    // Conectar a la sala de chat usando como identificador el nombre de la
    // cuenta del usuario o el indicado en el parametro

    int result = chatRoom.connectTo(chatRoomId);

    if(result < 0)
    {
        std::cerr << result << std::endl;
        exit(result);
    }


    // Ejecutar el chat

    chatRoom.run();


    return 0;
}

