#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>

#define SEGMENT_SIZE 0x6400

const char *data = "Hello there!";

int main(int argc, char *argv[])
{
  int status;
  int segment_id;

  segment_id = shmget(ftok("share.cpp", 20), SEGMENT_SIZE, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  char *sh_mem = (char *)shmat(segment_id, 0, 0);

  printf("Segment ID %d\n", segment_id);
  printf("Attached at %p\n", sh_mem);
  memmove(sh_mem, data, strlen(data) + 1);
  printf("%s\n", sh_mem);

  pid_t child_pid = fork();
  if (child_pid == -1)
    perror("fork");

  if (child_pid == 0)
  {
    strcpy(sh_mem, "NEW DATA Stored by Child Process\0");

    printf("child pid - %d\n", getpid());

    exit(EXIT_SUCCESS);
  }
  else
  {
    pid_t ret = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);
    if (ret == -1)
      perror("waitpid");

    if (WIFEXITED(status))
      printf("Child exited, status - %d\n", WEXITSTATUS(status));

    if (WEXITSTATUS(status) == 0)
      printf("%s\n", sh_mem);
  }

  shmdt(sh_mem);
  shmctl(segment_id, IPC_RMID, 0);
  exit(EXIT_FAILURE);
}