#include "../include/waiter.h"

int main() {
  int waiterId, tableId, managerId;
  struct Waiter* waiterPtr;
  struct Manager* managerPtr;
  struct Table* tablePtr;

  int waiterNumber;
  printf("Enter Waiter ID: ");
  scanf(" %d", &waiterNumber);

  waiterId =
      shmget(waiterNumber + 20, sizeof(struct Waiter), IPC_CREAT | PERMS);
  if (waiterId == -1) {
    perror("Error in shmget");
    exit(1);
  }

  waiterPtr = (struct Waiter*)shmat(waiterId, NULL, 0);
  if (waiterPtr == (void*)-1) {
    perror("Error in shmat");
    exit(1);
  }

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

  managerPtr = (struct Manager*)shmat(managerId, NULL, 0);
  if (managerPtr == (void*)-1) {
    perror("Error in shmat");
    exit(1);
  }

  managerPtr->waiterStatus[waiterNumber - 1] = 1;

  tableId = shmget(waiterNumber, sizeof(struct Table), PERMS);
  if (tableId == -1) {
    perror("Error in shmget");
    exit(1);
  }

  tablePtr = (struct Table*)shmat(tableId, NULL, 0);
  if (tablePtr == (void*)-1) {
    perror("Error in shmat");
    exit(1);
  }

  while (1) {
    int total = 0, invalid = 0;

    while (!tablePtr->isOrderPlaced)
      ;

    if (tablePtr->numberOfCustomers == -1) {
      printf("Total earnings: %d\n", waiterPtr->amountEarned);
      managerPtr->waiterStatus[waiterNumber - 1] = -1;

      if (shmdt(tablePtr) == -1) {
        perror("Error in shmdt");
        exit(1);
      }

      if (shmdt(managerPtr) == -1) {
        perror("Error in shmdt");
        exit(1);
      }

      if (shmdt(waiterPtr) == -1) {
        perror("Error in shmdt");
        exit(1);
      }
      if (shmctl(waiterId, IPC_RMID, 0) == -1) {
        perror("Error in shmctl");
        exit(1);
      }

      exit(0);
    }

    for (int i = 0; i < tablePtr->numberOfCustomers; i++) {
      for (int j = 0; j < tablePtr->numberOfOrders[i]; j++) {
        int price =
            findPriceOfMenuItem(tablePtr->orders[i * MAX_ORDER_SIZE + j] - 1);
        if (invalid) {
          break;
        } else {
          if (price == -1) {
            invalid = 1;
            tablePtr->isBillInvalid = 1;
            printf("Invalid order placed, item %d does not exist in menu\n",
                   tablePtr->orders[i * MAX_ORDER_SIZE + j]);
          }
        }
        if (!invalid) {
          total += price;
        }
      }
    }

    if (!invalid) {
      waiterPtr->total += total;
      tablePtr->total = total;

      printf("Bill is ready for table %d\n", tablePtr->tableNumber);
      printf("The total bill amount is: Rs.%d\n", tablePtr->total);
    }

    tablePtr->isBillReady = 1;
    tablePtr->isOrderPlaced = 0;
  }

  exit(0);
}

int findPriceOfMenuItem(int orderNumber) {
  int numberOfLines = 0;
  char currentLine[100];

  FILE* file = fopen("menu.txt", "r");

  while (fgets(currentLine, sizeof(currentLine), file)) {
    numberOfLines++;
  }

  fclose(file);

  if (orderNumber >= numberOfLines) {
    return -1;
  }
  if (orderNumber + 1 == -1) {
    return 0;
  }

  FILE* filePtr = fopen("menu.txt", "r");
  int cost[numberOfLines];
  int index = 0;

  while (fgets(currentLine, sizeof(currentLine), filePtr)) {
    char charCost[5] = {'\0'};
    int j = 0, flag = 0;
    for (int i = 0; i < 100; i++) {
      if (currentLine[i] == ' ') {
        if (flag) {
          flag = !flag;
          cost[index++] = atoi(charCost);
          break;
        } else if (currentLine[i + 1] - '0' <= 9) {
          flag = 1;
          continue;
        }
      }
      if (flag) {
        charCost[j++] = currentLine[i];
      }
    }
  }

  fclose(filePtr);

  return cost[orderNumber];
}