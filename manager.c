#include "../include/manager.h"

int main() {
  int managerId;
  struct Manager *managerPtr;
  key_t managerKey;

  int totalTables, totalProfit = 0, totalEarnings = 0, terminatedWaiters = 0;

  FILE *fptr;
  fptr = fopen("earnings.txt", "w");

  printf("Enter the Total Number of Tables at the Hotel: ");
  scanf(" %d", &totalTables);

  if ((managerKey = ftok(PATHNAME, MANAGER_ID)) == -1) {
    perror("ftok");
    exit(1);
  }

  managerId = shmget(managerKey, sizeof(struct Manager), IPC_CREAT | PERMS);
  if (managerId == -1) {
    perror("Error in shmget");
    exit(1);
  }

  managerPtr = (struct Manager *)shmat(managerId, NULL, 0);
  if (managerPtr == (void *)-1) {
    perror("Error in shmat");
    exit(1);
  }

  for (int i = 0; i < totalTables; i++) {
    managerPtr->waiterStatus[i] = 0;
  }

  managerPtr->terminate = 0;

  while (managerPtr->terminate != 1 || terminatedWaiters == 0) {
    for (int i = 0; i < totalTables; i++) {
      if (managerPtr->waiterStatus[i] == 1) {
        int waiterId;
        struct Waiter *waiterPtr;

        waiterId = shmget(i + 21, sizeof(struct Waiter), PERMS);
        if (waiterId == -1) {
          perror("Error in shmget");
          exit(1);
        }

        waiterPtr = (struct Waiter *)shmat(waiterId, NULL, 0);
        if (waiterPtr == (void *)-1) {
          perror("Error in shmat");
          exit(1);
        }

        if (waiterPtr->total > 0) {
          fprintf(fptr, "Earning from Table %d: Rs. %d.\n", i + 1,
                  waiterPtr->total);

          int waiterEarnings = (waiterPtr->total / 5) * 2;
          totalEarnings += waiterPtr->total;
          totalProfit += (waiterPtr->total - waiterEarnings);

          waiterPtr->total = 0;
          waiterPtr->amountEarned += waiterEarnings;

          printf("Paid %d to waiter %d\n", waiterEarnings, i + 1);
        }

        if (shmdt(waiterPtr) == -1) {
          perror("Error in shmdt");
          exit(1);
        }
      }
    }
    int count = 0;
    for (int i = 0; i < totalTables; i++) {
      if (managerPtr->waiterStatus[i] == -1) {
        count++;
      }
    }
    if (count == totalTables) {
      terminatedWaiters = 1;
    }
  }
  fprintf(fptr, "Total Earnings of Restaurant: Rs. %d.\n", totalEarnings);
  fprintf(fptr, "Total Wage of Waiters: Rs. %d.\n",
          totalEarnings - totalProfit);
  fprintf(fptr, "Total Profit: Rs. %d.\n", totalProfit);

  fclose(fptr);

  printf("Thank you for visiting the Hotel!\n");

  if (shmdt(managerPtr) == -1) {
    perror("Error in shmdt");
    exit(1);
  }

  if (shmctl(managerId, IPC_RMID, 0) == -1) {
    perror("Error in shmctl");
    exit(1);
  }

  exit(0);
}