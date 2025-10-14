package com.example.grpc.chat.server;

import io.grpc.Server;
import io.grpc.ServerBuilder;
import java.io.IOException;

public class ChatServer {
    public static void main(String[] args) throws IOException, InterruptedException {
        int port = 9090;
        Server server = ServerBuilder.forPort(port)
                .addService(new ChatServiceImpl())
                .build();

        server.start();
        System.out.println("Servidor de Chat iniciado en el puerto " + port);

        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            System.out.println("Apagando servidor...");
            server.shutdown();
            System.out.println("Servidor apagado.");
        }));

        server.awaitTermination();
    }
}