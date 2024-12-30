#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "types.h"
#include "customer.h"
#include "definitions.h"

void customer_process(int customer_id, int order_pipe[], int response_pipe[])
{
  // Different seed for each customer
  srand(time(NULL) + customer_id);

  // Display 1-indexed customer id
  int display_customer_id = customer_id + 1;

  for (int i = 0; i < ORDERS_PER_CUSTOMER; i++)
  {
    int product_id = rand() % NUM_PRODUCTS;

    // Send order
    write(order_pipe[1], &product_id, sizeof(int));

    // Wait for response
    order_response_t response;
    read(response_pipe[0], &response, sizeof(order_response_t));

    if (response.success)
    {
      printf("Client %d: Purchase complete, your total is %.2f euro\n", display_customer_id, response.price);
    }
    else
    {
      printf("Client %d: Products unavailable, request failed\n", display_customer_id);
    }

    printf("\n");

    // Wait between orders
    sleep(1);
  }

  exit(0);
}