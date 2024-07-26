#include "../include/table.h"

int main() {
  int tableNumber, isBillInvalid = 0, numberOfCustomers;
  int tableId;
  struct Table *tablePtr;

  printf("Enter table number (from 1 to 100): ");
  scanf(" %d", &tableNumber);

  tableId = shmget(tableNumber, sizeof(struct Table), IPC_CREAT | PERMS);
  if (tableId == -1) {
    perror("Error in shmget");
    exit(1);
  }

  tablePtr = (struct Table *)shmat(tableId, NULL, 0);
  if (tablePtr == (void *)-1) {
    perror("Error in shmat");
    exit(1);
  }

  while (1) {
    tablePtr->isBillInvalid = 0;
    tablePtr->isBillReady = 0;
    tablePtr->isOrderPlaced = 0;

    if (!isBillInvalid) {
      tablePtr->tableNumber = tableNumber;
      printf(
          "Enter Number of Customers at the Table (maximum no. of customers "
          "can be %d): ",
          MAX_ORDERS);
      scanf(" %d", &(tablePtr->numberOfCustomers));

      if (tablePtr->numberOfCustomers == -1) {
        tablePtr->isOrderPlaced = 1;
        sleep(5);

        if (shmdt(tablePtr) == -1) {
          perror("Error in shmdt");
          exit(1);
        }

        if (shmctl(tableId, IPC_RMID, 0) == -1) {
          perror("Error in shmctl");
          exit(1);
        }

        printf("Customers left Table %d", tableNumber);
        exit(0);
      }

      displayMenu();
      printf("\n");
    } else {
      tablePtr->numberOfCustomers = numberOfCustomers;
    }

    isBillInvalid = 0;

    int customers = tablePtr->numberOfCustomers;
    int fd[MAX_ORDERS][2];
    while (customers--) {
      int currentNumber = tablePtr->numberOfCustomers - customers - 1;

      if (pipe(fd[currentNumber]) == -1) {
        perror("Error creating pipe");
        exit(1);
      }

      pid_t pid = fork();

      if (pid < 0) {
        perror("Error creating child process in fork");
        exit(1);
      }

      // Maximum 10 items can be ordered per person
      if (pid == 0) {
        printf(
            "Enter the serial number(s) of the item(s) to order from the menu. "
            "Enter -1 when done: ");
        struct Order order;
        int choice = 0, index = 0;
        while (choice != -1) {
          if (index == 10) {
            order.choices[index] = -1;
            break;
          } else {
            scanf(" %d", &choice);
            order.choices[index++] = choice;
          }
        }
        order.numberOfOrders = index;

        close(fd[currentNumber][0]);
        write(fd[currentNumber][1], &order, sizeof(order));
        close(fd[currentNumber][1]);

        while (!tablePtr->isBillReady)
          ;
        printf("Customer %d terminated successfully\n", currentNumber + 1);

        exit(0);
      }
      else {
        close(fd[currentNumber][1]);

        struct Order order;

        read(fd[currentNumber][0], &order, sizeof(order));
        close(fd[currentNumber][0]);

        tablePtr->numberOfOrders[currentNumber] = order.numberOfOrders;
        for (int i = 0; i < order.numberOfOrders; i++) {
          tablePtr->orders[currentNumber * MAX_ORDER_SIZE + i] =
              order.choices[i];
        }
      }
    }

    tablePtr->isOrderPlaced = 1;

    while (!tablePtr->isBillReady)
      ;

    sleep(2);

    if (tablePtr->isBillInvalid == 1) {
      printf("Invalid order placed\n");
      numberOfCustomers = tablePtr->numberOfCustomers;
      isBillInvalid = 1;

      continue;
    }

    printf("The total bill amount is: Rs.%d\n", tablePtr->total);
  }
}

void displayMenu() {
  char currentLine[100];

  FILE *file = fopen("menu.txt", "r");

  while (fgets(currentLine, sizeof(currentLine), file)) {
    printf("%s", currentLine);
  }

  fclose(file);
}