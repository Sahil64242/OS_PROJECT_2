#ifndef UTILS_H
#define UTILS_H

#define PERMS 0644
#define PATHNAME "./src/manager.c"
#define MANAGER_ID 'A'
#define MAX_ORDERS 10
#define MAX_ORDER_SIZE 10
#define MAX_TABLES 10

struct Table {
  int tableNumber;
  int numberOfCustomers;
  int numberOfOrders[MAX_ORDERS];
  int orders[MAX_ORDERS * MAX_ORDER_SIZE];
  int isOrderPlaced;
  int isBillInvalid;
  int isBillReady;
  int total;
};

struct Waiter {
  int amountEarned;
  int total;
};

struct Manager {
  int terminate;
  int waiterStatus[MAX_TABLES];
};

struct Order {
  int numberOfOrders;
  int choices[MAX_ORDER_SIZE + 1];
};

#endif