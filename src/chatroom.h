/*
 * chatroom.h - Sala de chat multiusuario basada en memoria compartida
 *
 * Este programa de Adrián Rodríguez Bazaga <alu0100826456@ull.edu.es> está bajo una Licencia
 * Creative Commons Public Domain Dedication 1.0.
 *
 *     http://creativecommons.org/publicdomain/zero/1.0/deed.es
 */

#ifndef CHATROOM_H
#define CHATROOM_H

#include <string>



class ChatRoom
{
public:

    ChatRoom();
    ~ChatRoom();

    // Conectar a la sala de chat indicada
    int connectTo(const std::string& chatRoomId);

    // Ejecutar el chat
    void run();

    //Asignar nombre de usuario
    void setUserName(std::string& usuario);

private:
    struct SharedMessage;

    // Buffer en memoria compartida para el intercambio de mensajes
    SharedMessage* sharedMessage_;
    // Número de secuencia del último mensaje leido con receive()
    unsigned messageReceiveCounter_;

    // Indicador de si el objeto es el propietario del objeto de memoria
    // compartida. El propietario es el responsable de su destrucción
    bool isSharedMemoryObjectOwner_;

    std::string chatRoomId_;
    std::string userName_;

    // Leer mensajes desde la entrada estándar y enviarlos a la sala de chat
    void runSender();
    // Recibir mensajes de la sala de chat y mostrarlos por la salida estándar
    void runReceiver();

    // Enviar um mensaje a la sala de chat
    void send(const std::string& message);
    // Recibir un mensaje de la sala de chat
    void receive(std::string& message);

    // Ejecutar el comando indicado y enviar su salida estándar
    void execAndSend(std::string& command);
};

#endif // CHATROOM_H
