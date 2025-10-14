package com.example.grpc.chat.server;

import com.example.grpc.chat.*;
import io.grpc.stub.StreamObserver;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

public class ChatServiceImpl extends ChatServiceGrpc.ChatServiceImplBase {

    // El Map es el encargado de almacenar todos los clientes que se conecten al servidor, este va a solamente almacenar los userNames.
    // Es un ConcurrentMap, por cada cliente se ejecutara un hilo de ejecución distinto, posibilitando las conexiónes de más de un cliente.
    private static final ConcurrentMap<String, StreamObserver<ChatMessageFromServer>> connectedClients = new ConcurrentHashMap<>();

    // Aca se guardará el path del archivo de historial al recibir la solicitud de '/historial'.
    private static final Path HISTORY_FILE = Paths.get("chat_history.log");

    public ChatServiceImpl() {
        try {
            if (!Files.exists(HISTORY_FILE)) {
                Files.createFile(HISTORY_FILE);
            }
        } catch (IOException e) {
            System.err.println("Error creating history file: " + e.getMessage());
        }
    }

    // StreamObserver es la interfaz, representando el canal del servidor al cliente, o en este caso el chat.
    @Override
    public StreamObserver<ChatMessage> chat(StreamObserver<ChatMessageFromServer> responseObserver) {
        return new StreamObserver<ChatMessage>() {
            private String userName = null;

            // Se dispara cada vez que el cliente envia un mensaje... Siempre el primero va a ser el de userName.
            // Despues de que tenga asignado un userName, todos los demas mensajes seran mensajes normales que se
            // ubicaran en el chat.
            @Override
            public void onNext(ChatMessage request) {
                if (userName == null) {
                    userName = request.getFromUser();
                    System.out.println("--> " + userName + " se ha conectado.");

                    ChatMessageFromServer welcomeMessage = ChatMessageFromServer.newBuilder()
                            .setFromUser("Server")
                            .setContent("Bienvenido al chat, " + userName + "!")
                            .setTimestamp(getCurrentTimestamp())
                            .build();
                    responseObserver.onNext(welcomeMessage);

                    String joinNotification = userName + " se ha unido al chat.";
                    broadcastMessage(buildServerMessage(joinNotification));
                    connectedClients.put(userName, responseObserver);
                } else {
                    System.out.println("Mensaje de " + userName + ": " + request.getContent());
                    ChatMessageFromServer messageToBroadcast = ChatMessageFromServer.newBuilder()
                            .setFromUser(userName)
                            .setContent(request.getContent())
                            .setTimestamp(getCurrentTimestamp())
                            .build();
                    logMessage(messageToBroadcast);
                    broadcastMessage(messageToBroadcast);
                }
            }

            // En el caso de que se rompa la conexión, ó tiremos un "Ctrl+C", el handler se encargará de manejar todo.
            @Override
            public void onError(Throwable t) {
                System.err.println("Error para el cliente " + userName + ": " + t.getMessage());
                handleDisconnect();
            }

            // El onCompleted se encargará de finalizar la conexión cuando el cliente envie un "/exit".
            @Override
            public void onCompleted() {
                handleDisconnect();
                responseObserver.onCompleted();
            }

            // Metodo privado encargado de manejar las desconexiones, informando en el chat del servidor solo en el caso
            // de que el cliente haya estado registrado previamente en el chat.
            private void handleDisconnect() {
                if (userName != null) {
                    System.out.println("<-- " + userName + " se ha desconectado.");
                    connectedClients.remove(userName);
                    String leaveMessage = userName + " ha dejado el chat.";
                    broadcastMessage(buildServerMessage(leaveMessage));
                }
            }
        };
    }

    // Es una implemnetación de RPC unario. Recibe la petición de un cliente, lee todo el contenido del historial,
    // lo empaqueta en una Response y lo envia todo de vuelta con el .onNext()...
    @Override
    public void getHistory(HistoryRequest request, StreamObserver<HistoryResponse> responseObserver) {
        System.out.println("Solicitud de historial recibida.");
        try {
            byte[] fileContent = Files.readAllBytes(HISTORY_FILE);
            HistoryResponse response = HistoryResponse.newBuilder()
                    .setFileName("chat_history.txt")
                    .setFileContent(com.google.protobuf.ByteString.copyFrom(fileContent))
                    .build();
            responseObserver.onNext(response);
            responseObserver.onCompleted();
        } catch (IOException e) {
            System.err.println("Error al leer el archivo de historial: " + e.getMessage());
            responseObserver.onError(e);
        }
    }

    // Recorre todos los valores del mapa y usa el metodo .onNext par enviarles el mensaje a todos los clientes.
    private void broadcastMessage(ChatMessageFromServer message) {
        for (StreamObserver<ChatMessageFromServer> client : connectedClients.values()) {
            client.onNext(message);
        }
    }

    // Encargado de tomar un mensaje del cliente, interpretarlo, darle formato y guardarlo en el historial.
    private void logMessage(ChatMessageFromServer message) {
        String logEntry = String.format("[%s] %s: %s\n",
                message.getTimestamp(), message.getFromUser(), message.getContent());
        try {
            Files.write(HISTORY_FILE, logEntry.getBytes(), StandardOpenOption.APPEND);
        } catch (IOException e) {
            System.err.println("Error al escribir en el historial: " + e.getMessage());
        }
    }

    private ChatMessageFromServer buildServerMessage(String content) {
        ChatMessageFromServer message = ChatMessageFromServer.newBuilder()
                .setFromUser("Server")
                .setContent(content)
                .setTimestamp(getCurrentTimestamp())
                .build();
        logMessage(message);
        return message;
    }

    private String getCurrentTimestamp() {
        return new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date());
    }
}