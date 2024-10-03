#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

#define BUF_SIZE 1024

char *SenderPipe = "/tmp/SenderPipe";
char *ReceiverPipe = "/tmp/ReceiverPipe";

void cleanup() {
    unlink(SenderPipe);
    unlink(ReceiverPipe);
}

void signal_handler(int sig) {
    printf("\nUser exited the chat.\n");
    cleanup();
    exit(0);
}

int main() {
    char buffer[BUF_SIZE];

    // Signal handling to ensure cleanup on interruption
    signal(SIGINT, signal_handler);

    // Create FIFOs for communication
    if (mkfifo(SenderPipe, 0666) < 0 && errno != EEXIST) {
        perror("Error creating sender_fifo");
        exit(EXIT_FAILURE);
    }

    if (mkfifo(ReceiverPipe, 0666) < 0 && errno != EEXIST) {
        perror("Error creating receiver_fifo");
        unlink(SenderPipe);
        exit(EXIT_FAILURE);
    }

    printf("\n----------WELCOME TO MNH CHAT SYSTEM----------\n");
    printf("Instructions: Type 'exit' to quit.\n");
    printf("I am User 2 (📩)\n");

    while (1) {
        int Receiver = open(ReceiverPipe, O_RDONLY);
        if (Receiver < 0) {
            perror("Error opening receiver_fifo");
            cleanup();
            exit(EXIT_FAILURE);
        }

        read(Receiver, buffer, BUF_SIZE);
        printf("Sender (😊): %s\n", buffer);
        close(Receiver);

        // Check for exit message
        if (strcmp(buffer, "User 1 has left the chat. Goodbye! 😊") == 0) {
            break;
        }

        int Sender = open(SenderPipe, O_WRONLY);
        if (Sender < 0) {
            perror("Error opening sender_fifo");
            cleanup();
            exit(EXIT_FAILURE);
        }

        printf("You (📩): ");
        fgets(buffer, BUF_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;  // Remove trailing newline

        // Confirm exit command
        if (strcmp(buffer, "exit") == 0) {
            char exitMessage[] = "User 2 has left the chat. Goodbye! 😊";
            write(Sender, exitMessage, strlen(exitMessage) + 1);
            printf("Goodbye! Have a great day! 😊\n");
            break;
        }

        write(Sender, buffer, strlen(buffer) + 1);
        close(Sender);
    }

    cleanup();
    return 0;
}
