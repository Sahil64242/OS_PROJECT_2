#include "../include/admin.h"

int main() {
  int managerId;
  struct Manager *managerPtr;
  key_t managerKey;

  if ((managerKey = ftok(PATHNAME, MANAGER_ID)) == -1) {
    perror("ftok");
    exit(1);
  }

  managerId = shmget(managerKey, sizeof(struct Manager), PERMS);
  if (managerId == -1) {
    perror("Error in shmget");
    exit(1);
  }

  managerPtr = (struct Manager *)shmat(managerId, NULL, 0);
  if (managerPtr == (void *)-1) {
    perror("Error in shmat");
    exit(1);
  }

  char input = 'N';
  while (input != 'Y' && input != 'y') {
    if (input != 'N' && input != 'n') {
      printf("Invalid input\n");
    }
    printf(
        "Do you want the restaurant to close? Press Y for Yes and N for No.\n");
    scanf(" %c", &input);
  }

  managerPtr->terminate = 1;

  if (shmdt(managerPtr) == -1) {
    perror("Error in shmdt");
    exit(1);
  }
}