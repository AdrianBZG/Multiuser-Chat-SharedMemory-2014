/*
 * chatroom.cpp - Sala de chat multiusuario basada en memoria compartida
 *
 * Este programa de Adrián Rodríguez Bazaga <alu0100826456@ull.edu.es> está bajo una Licencia
 * Creative Commons Public Domain Dedication 1.0.
 *
 *     http://creativecommons.org/publicdomain/zero/1.0/deed.es
 */

#include <condition_variable>
#include <mutex>
#include <cstring>
#include <iostream>

//Librerias para las funciones que vamos a utilizar

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <thread>

//

#include "chatroom.h"

//
// Class ChatRoom::SharedMessage
//


struct ChatRoom::SharedMessage
{

    //
    char texto[1024*1024];
    int capacidad_texto;
    unsigned numero_mensaje;
    std::mutex mi_mutex;
    std::condition_variable  cond_var;
    //
    std::string chatRoomId;
    //

    SharedMessage();
};


ChatRoom::SharedMessage::SharedMessage()
{

}

//
// Class ChatRoom
//
// Clase de la sala de chat basada en memoria compartida.
//

ChatRoom::ChatRoom()
    : sharedMessage_(nullptr),
      messageReceiveCounter_(0),      
      isSharedMemoryObjectOwner_(false)
{

}

ChatRoom::~ChatRoom()
{

    //Si eres el creador del objeto de memoria compartida destruir el objeto
    if(isSharedMemoryObjectOwner_ == true)
    {
        //
        //Pasamos el puntero de String a puntero de Char para pasarselo a la función shm_unlink
        char CRI[1024];
        strcpy(CRI, chatRoomId_.c_str());
        CRI[sizeof(CRI) - 1] = 0;
        //
        shm_unlink(CRI);
    }
    //

    //Deshacer el mapeo
    munmap(NULL, sizeof(ChatRoom::SharedMessage));
    //

}
//Funcion que obtiene la salida de un comando
std::string exec(char* cmd) {
    std::string result = "";
    if(FILE* pipe = popen(cmd, "r"))
    {
        char buffer[128];
        while(!feof(pipe)) {
         if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
         }
        pclose(pipe);
        return result;
    }
    else
    {
        result = "";
        return result;
    }
}
//

int ChatRoom::connectTo(const std::string& chatRoomId)
{
    //Variables locales
    int shm_var;
    int error;
    //
    chatRoomId_ = chatRoomId;
    //Pasamos el puntero de String a puntero de Char para pasarselo a la función shm_open
    char CRI[1024];
    strcpy(CRI, chatRoomId_.c_str());
    CRI[sizeof(CRI) - 1] = 0;
    //


    shm_var = shm_open(CRI, O_EXCL | O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    if(shm_var != -1)
    {
        isSharedMemoryObjectOwner_ = true;
        if (ftruncate(shm_var, sizeof(ChatRoom::SharedMessage)) == -1)
        {
           //Error de ftruncate
            error = -1;
            return error;
        }

        sharedMessage_ = (SharedMessage*)mmap(NULL, sizeof(ChatRoom::SharedMessage), PROT_READ | PROT_WRITE, MAP_SHARED, shm_var, 0);

        if(sharedMessage_ == MAP_FAILED)
        {
           //Error al mapear siendo propietario
            error = -2;
            return error;
        }

        new(sharedMessage_) ChatRoom::SharedMessage;

    }
    else
    {
        shm_var = shm_open(CRI, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

        isSharedMemoryObjectOwner_ = false;

        sharedMessage_ = (SharedMessage*)mmap(NULL, sizeof(ChatRoom::SharedMessage), PROT_READ | PROT_WRITE, MAP_SHARED, shm_var, 0);

        if(sharedMessage_ == MAP_FAILED)
        {
           //Error al mapear siendo no propietario
            error = -3;
            std::cout << "Error de mapeo de cliente" << std::endl;
            return error;
        }

        send("\033[1;32m::: '"+userName_+"' ha entrado al chat\033[0m ");

    }

    //Devolver el entero correspondiente al error
    return 0;
    //


}
void ChatRoom::run()
{
    std::thread t1 (&ChatRoom::runSender, this);
    std::thread t2 (&ChatRoom::runReceiver, this);

    if ( isSharedMemoryObjectOwner_ ) {
        system("clear");
        std::cout << "\033[1;34mERES <PROPIETARIO> DE LA SALA DE CHAT\033[0m" << std::endl << std::endl << std::endl;

    }
    else {
        system("clear");
        std::cout << "\033[1;34mERES <USUARIO NORMAL> DE LA SALA DE CHAT\033[0m " << std::endl << std::endl << std::endl;
    }

    //Metemos los hilos
    t1.join();
    t2.join();
    //

    // Volver si no estamos conectados a una sala de chat
    if ( sharedMessage_ == nullptr )
    {
        std::cout << "ERROR: PUNTERO NULO" << std::endl;
        return;
    }
}

void ChatRoom::runSender()
{
    while(true)
    {
        std::string mensaje;
        std::getline(std::cin,mensaje);

        if(mensaje[0] != '!' && mensaje[0] != '@' && mensaje.compare("--help") != 0 && mensaje.compare("--dibujos") != 0)
        {
            if(mensaje != ":quit")
            {
                mensaje = "\033[1;36m"+userName_+":\033[0m "+mensaje;
                send(mensaje);
            }
            else
            {
                system("clear");
                if(isSharedMemoryObjectOwner_ == true)
                {
                    //
                    //Pasamos el puntero de String a puntero de Char para pasarselo a la función shm_unlink
                    char CRI[1024];
                    strcpy(CRI, chatRoomId_.c_str());
                    CRI[sizeof(CRI) - 1] = 0;
                    //

                    send("\033[1;31m::: El propietario '"+userName_+"' ha salido del chat y el chat ha sido eliminado\033[0m ");
                    shm_unlink(CRI);
                    std::cout << "Eras el propietario de la sala, por lo tanto la sala ha sido borrada." << std::endl;
                }
                else
                {
                    send("\033[1;31m::: '"+userName_+"' ha salido del chat\033[0m ");
                }

                std::cout << "Has cerrado la aplicacion." << std::endl;
                exit(0);
            }

        }
        if(mensaje[0] == '!' && mensaje.length() > 1)
        {
            execAndSend(mensaje);
        }
        if(mensaje[0] == '@' && mensaje.length() > 1)
        {
            if(mensaje == "@vaca")
            {
                std::cout << "\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+exec("apt-get moo")+"\033[0m" << std::endl;
                send("\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+exec("apt-get moo")+"\033[0m");
            }
            if(mensaje == "@cara1")
            {
                std::string dibujo = "  \\|||||/\n ( ~   ~ )\n@( 0   0 )@\n (   C   )\n  \\ \\_/ / \n   |___|\n";
                std::cout << "\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m" << std::endl;
                send("\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m");
            }
            if(mensaje == "@cara2")
            {
                std::string dibujo = "          ,xXXXXXXXXXx_\n        ,XXXXXXXXXXXXXXx_\n      ,XXf ~~~[XX]  ~~~XX\n     ,XXX <@> [XX]  <@>XXX,\n    ,XXXX_____[XX]_____XXXX,\n    XXXXXXXXXXX/~\\XXXXXXXXXX\n   XXXXXXXXXXX/   \\XXXXXXXXX\n   XXXXXXXXXXX_____XXXXXXXXX\n   XXXXXXXXXXXXXXXXXXXXXXXXX\n   XXXXXX XXXXXXXXXXXXX XXXX\n    XXXXX \\XXXXXXXXXXX/ XXX!\n    'XXXX_______________XXX\n     'XXXXXXXXXXXXXXXXXXXX\n      'XXXXXXXXXXXXXXXXXf\n        'XXXXXXXXXXXXXX~   \n          '~XXXXXXXX´~\n";
                std::cout << "\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m" << std::endl;
                send("\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m");
            }
            if(mensaje == "@cara3")
            {
                std::string dibujo = "          ,--.\n         /  . `----...__ \n       ,'  (            `.\n       ;    `-            \\ \n       |   ___..----.._____:-""-,\n      ,+-""             .-\"     |\n      ;           _..--\"  _.+\"  |\n      |     _..--\".-\" _.-\"  |_.-'\n      L..--\"   .-\"_.-\"      |\n     /      .-""""-.  ,--,  :\n    /    .-\"-.      l ;  | .'\n   /  .-\" `--'  /    \"   |'|\n  /.-\":        :       _.' :\n      ; ,--,    \\      ;    \\__\n      :  .-'`.   `.    |     \\ \"-.\n      '-';    \\        :      )   \\ \n         )_.-, ,  /   .'     /     \\ \n         `,         .'      /       \\ \n          :-._.'  .'   :   /         \\ \n          ;    .-'\\    |  /           `.\n          '---\"    )   ; /              `.\n                   \  / /            .-\"  `.\n                   /`. /           .'       \\ \n                  /  :/           /          \\ \n";
                std::cout << "\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m" << std::endl;
                send("\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m");
            }
            if(mensaje == "@conejo")
            {
                std::string dibujo = "    (\\   /)\n    .\\\\_//.\n     )0 0(\n    ( (_) )\n     `'\"'`\n";
                std::cout << "\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m" << std::endl;
                send("\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m");
            }
            if(mensaje == "@:o")
            {
                std::string dibujo = "(⨀_⨀)\n";
                std::cout << "\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m" << std::endl;
                send("\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m");
            }
            if(mensaje == "@:(")
            {
                std::string dibujo = "（︶︿︶）\n";
                std::cout << "\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m" << std::endl;
                send("\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m");
            }
            if(mensaje == "@:D")
            {
                std::string dibujo = "( ͡° ͜ʖ ͡°)\n";
                std::cout << "\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m" << std::endl;
                send("\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m");
            }
            if(mensaje == "@>_<")
            {
                std::string dibujo = "(⋟﹏⋞)\n";
                std::cout << "\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m" << std::endl;
                send("\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m");
            }
            if(mensaje == "@$")
            {
                std::string dibujo = "[̲̅$̲̅(̲̅ιοο̲̅)̲̅$̲̅]\n";
                std::cout << "\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m" << std::endl;
                send("\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m");
            }
            if(mensaje == "@:/")
            {
                std::string dibujo = "【ツ】\n";
                std::cout << "\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m" << std::endl;
                send("\n\033[1;36m::: '"+userName_+"' HA ENVIADO UN DIBUJO ASCII\n\n"+dibujo+"\033[0m");
            }
        }
        if(mensaje.compare("--help") == 0)
        {
            std::cout << "\n\033[1;32mAYUDA:\n\n- Escribe ! seguido de un comando para mandar el resultado del comando por el chat (Ejemplo: !ls)\n\n- Escribe @ seguido de un nombre de dibujo para mandar un dibujo ascii por el chat (Usa --dibujos. Ejemplo: @:D)\n\n- Escribe :quit para salir del chat\n\033[0m" << std::endl;
        }
        if(mensaje.compare("--dibujos") == 0)
        {
            std::cout << "\n\033[1;32mDIBUJOS:\n\n- Escribe @ seguido de uno de los siguientes nombres:\n\n:/, $, >_<, :D, :(, :o, conejo, cara1, cara2, cara3, vaca\n\033[0m" << std::endl;
        }


    }

}

void ChatRoom::send(const std::string& message)
{

    sharedMessage_->mi_mutex.lock();

    //Notificar que hay nuevo mensaje con condition_variable
    strcpy(sharedMessage_->texto, message.c_str());
    sharedMessage_->capacidad_texto = message.size();
    sharedMessage_->numero_mensaje = sharedMessage_->numero_mensaje+1;
    messageReceiveCounter_ = sharedMessage_->numero_mensaje;
    sharedMessage_->cond_var.notify_all();
    //
    sharedMessage_->mi_mutex.unlock();



}

void ChatRoom::runReceiver()
{
    bool infinito = true;

    while(infinito == true)
    {
        std::string mensaje_recibido;
        receive(mensaje_recibido);
        std::cout << mensaje_recibido << std::endl;
    }

}

void ChatRoom::receive(std::string& message)
{
    std::unique_lock<std::mutex> lock(sharedMessage_->mi_mutex);

    while(messageReceiveCounter_ == sharedMessage_->numero_mensaje)
    {
        sharedMessage_->cond_var.wait(lock);
    }


    message = sharedMessage_->texto;
    messageReceiveCounter_ = sharedMessage_->numero_mensaje;
    sharedMessage_->cond_var.notify_all();
}

void ChatRoom::execAndSend(std::string& command)
{
    command.erase(0,1);
    std::string command_def(command);
    command += " 2> /dev/null";
    //Pasamos el puntero de String a puntero de Char para pasarselo a la función exec
    char cmdstr[1024];
    strcpy(cmdstr, command.c_str());
    cmdstr[sizeof(cmdstr) - 1] = 0;
    //
    std::string resultado = exec(cmdstr);

    if(!resultado.empty())
    {
        std::cout << "\n\033[1;33m::: '"+userName_+"' HA EJECUTADO EL COMANDO '"+command_def+"'\n\n "+resultado+"\033[0m " << std::endl;
        send("\n\033[1;33m::: '"+userName_+"' HA EJECUTADO EL COMANDO '"+command_def+"'\n\n "+resultado+"\033[0m ");
    }
    else
    {
        return;
        std::cout << "ola k ase" << std::endl;
    }
}

void ChatRoom::setUserName(std::string& usuario)
{
    userName_ = usuario;
}
