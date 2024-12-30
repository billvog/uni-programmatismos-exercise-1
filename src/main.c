#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "types.h"
#include "catalog.h"
#include "customer.h"
#include "definitions.h"

int main()
{
  product_t catalog[NUM_PRODUCTS];
  initialize_catalog(catalog);

  // Create pipes for each customer
  int order_pipes[NUM_CUSTOMERS][2];
  int response_pipes[NUM_CUSTOMERS][2];

  for (int i = 0; i < NUM_CUSTOMERS; i++)
  {
    if (pipe(order_pipes[i]) < 0 || pipe(response_pipes[i]) < 0)
    {
      perror("Pipe creation failed");
      exit(1);
    }
  }

  // Create customer processes
  for (int i = 0; i < NUM_CUSTOMERS; i++)
  {
    pid_t pid = fork();
    if (pid < 0)
    {
      perror("Fork failed");
      exit(1);
    }
    if (pid == 0)
    {
      // Child process
      customer_process(i, order_pipes[i], response_pipes[i]);
    }
  }

  int orders_processed = 0;
  int successful_orders = 0;
  float total_revenue = 0;

  int failed_customers[NUM_PRODUCTS][NUM_CUSTOMERS] = {};
  int failed_customers_count[NUM_PRODUCTS] = {0};

  // Process orders until all customers finish
  while (orders_processed < NUM_CUSTOMERS * ORDERS_PER_CUSTOMER)
  {
    for (int i = 0; i < NUM_CUSTOMERS; i++)
    {
      int product_id;
      if (read(order_pipes[i][0], &product_id, sizeof(int)) > 0)
      {
        catalog[product_id].order_requests++;

        order_response_t response = {0, 0.0};

        if (catalog[product_id].item_count > 0)
        {
          catalog[product_id].item_count--;
          catalog[product_id].items_sold++;
          total_revenue += catalog[product_id].price;
          successful_orders++;

          response.success = 1;
          response.price = catalog[product_id].price;
        }
        else
        {
          failed_customers[product_id][failed_customers_count[product_id]] = i;
          failed_customers_count[product_id]++;
        }

        write(response_pipes[i][1], &response, sizeof(order_response_t));
        orders_processed++;

        // Processing time
        sleep(1);
      }
    }
  }

  // Print final report
  printf("\nFinal Report:\n\n");

  for (int i = 0; i < NUM_PRODUCTS; i++)
  {
    printf("Product #%d:\n", i);
    printf("  Description: %s\n", catalog[i].description);
    printf("  Order requests: %d\n", catalog[i].order_requests);
    printf("  Items sold: %d\n", catalog[i].items_sold);
    if (failed_customers_count[i] > 0)
    {
      printf("  Failed customers: ");
      for (int j = 0; j < failed_customers_count[i]; j++)
      {
        printf("%d ", failed_customers[i][j] + 1);
      }
      printf("\n");
    }
    printf("\n");
  }

  printf("\nSummary:\n");
  printf("  Total orders: %d\n", orders_processed);
  printf("  Successful orders: %d\n", successful_orders);
  printf("  Failed orders: %d\n", orders_processed - successful_orders);
  printf("  Total revenue: %.2f\n", total_revenue);

  // Cleanup
  for (int i = 0; i < NUM_PRODUCTS; i++)
  {
    free(catalog[i].description);
  }

  return 0;
}