package com.example.grpc.chat.client;

import com.example.grpc.chat.*;
import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.grpc.stub.StreamObserver;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Scanner;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class ChatClient {
    public static void main(String[] args) throws InterruptedException {
        // Representa la conexión física con el servidor, en este caso no usamos nada fancy como TLS/SSL
        // Nos conectaremos directamente con localhost para pruebas locales...
        ManagedChannel channel = ManagedChannelBuilder.forAddress("localhost", 9090)
                .usePlaintext()
                .build();

        // asyncStub es mas usando para el chat asincrono, no bloqueando el programa.
        ChatServiceGrpc.ChatServiceStub asyncStub = ChatServiceGrpc.newStub(channel);
        // el blockingStrub es mas para las tareas de sincronización, como por ejemplo para llamar al getHistory()..
        ChatServiceGrpc.ChatServiceBlockingStub blockingStub = ChatServiceGrpc.newBlockingStub(channel);

        Scanner scanner = new Scanner(System.in);
        System.out.print("Ingresa tu nombre: ");
        String userName = scanner.nextLine();

        CountDownLatch finishLatch = new CountDownLatch(1);

        // Es el oyente para los mensajes del servidor que recibirá cada cliente de los demás clientes.
        StreamObserver<ChatMessageFromServer> responseObserver = new StreamObserver<>() {

            // Se dispara cada vez que el server envía un mensaje. Su única tarea es imprimir el mensaje en la consola de usuario.
            @Override
            public void onNext(ChatMessageFromServer message) {
                System.out.printf("[%s] %s: %s\n", message.getTimestamp(), message.getFromUser(), message.getContent());
            }

            //Maneja las desconexiones junto con el onCompleted...
            @Override
            public void onError(Throwable t) {
                System.err.println("Conexión perdida: " + t.getMessage());
                finishLatch.countDown();
            }

            @Override
            public void onCompleted() {
                System.out.println("Desconectado del servidor.");
                finishLatch.countDown();
            }
        };

        // Esto es utilizado para enviar mensajes al servidor. Nuestra primera conexión necesitará que enviemos el primer mensaje
        // que sera el userName, sino no avanzaremos.
        StreamObserver<ChatMessage> requestObserver = asyncStub.chat(responseObserver);

        try {
            // Cuando el cliente escriba algo y presione Enter, se usará:
            requestObserver.onNext(ChatMessage.newBuilder().setFromUser(userName).setContent("CONNECT").build());

            while (true) {
                String line = scanner.nextLine();
                if (line.equalsIgnoreCase("/exit")) {
                    break;
                }

                if (line.equalsIgnoreCase("/historial")) {
                    try {
                        System.out.println("Solicitando historial...");
                        HistoryResponse response = blockingStub.getHistory(HistoryRequest.newBuilder().build());
                        Files.write(Paths.get(response.getFileName()), response.getFileContent().toByteArray());
                        System.out.println("Historial guardado en " + response.getFileName());
                    } catch (IOException e) {
                        System.err.println("Error al guardar el historial: " + e.getMessage());
                    }
                    continue;
                }

                // Esto es mas que nada el mensaje normal... Lo demás son los comandos especiales.
                ChatMessage chatMessage = ChatMessage.newBuilder()
                        .setFromUser(userName)
                        .setContent(line)
                        .build();
                requestObserver.onNext(chatMessage);
            }
        } catch (RuntimeException e) {
            requestObserver.onError(e);
            throw e;
        }

        requestObserver.onCompleted();
        finishLatch.await(1, TimeUnit.SECONDS);
        channel.shutdownNow().awaitTermination(5, TimeUnit.SECONDS);
    }
}