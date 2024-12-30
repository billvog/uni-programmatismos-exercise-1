#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

#define NUM_PRODUCTS 20
#define NUM_CUSTOMERS 5
#define ORDERS_PER_CUSTOMER 10
#define INITIAL_STOCK 2

typedef struct
{
  char *description;
  float price;
  unsigned int item_count;
  // Add tracking fields for the final report
  unsigned int order_requests;
  unsigned int items_sold;
} product;

typedef struct
{
  int success;
  float price;
} order_response;

void initialize_catalog(product *catalog)
{
  char desc[50];
  for (int i = 0; i < NUM_PRODUCTS; i++)
  {
    sprintf(desc, "Product %d", i + 1);
    catalog[i].description = strdup(desc);
    catalog[i].price = 10.0 + (rand() % 90); // Random price between 10 and 100
    catalog[i].item_count = INITIAL_STOCK;
    catalog[i].order_requests = 0;
    catalog[i].items_sold = 0;
  }
}

void customer_process(int customer_id, int order_pipe[], int response_pipe[])
{
  // Different seed for each customer
  srand(time(NULL) + customer_id);

  for (int i = 0; i < ORDERS_PER_CUSTOMER; i++)
  {
    int product_id = rand() % NUM_PRODUCTS;

    // Send order
    write(order_pipe[1], &product_id, sizeof(int));

    // Wait for response
    order_response response;
    read(response_pipe[0], &response, sizeof(order_response));

    if (response.success)
    {
      printf("Client %d: Purchase complete, your total is %.2f euro\n", customer_id, response.price);
    }
    else
    {
      printf("Client %d: Products unavailable, request failed\n", customer_id);
    }

    printf("\n");

    // Wait between orders
    sleep(1);
  }
  exit(0);
}

int main()
{
  product catalog[NUM_PRODUCTS];
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
    { // Child process
      customer_process(i, order_pipes[i], response_pipes[i]);
    }
  }

  int orders_processed = 0;
  int successful_orders = 0;
  float total_revenue = 0;

  // Process orders until all customers finish
  while (orders_processed < NUM_CUSTOMERS * ORDERS_PER_CUSTOMER)
  {
    for (int i = 0; i < NUM_CUSTOMERS; i++)
    {
      int product_id;
      if (read(order_pipes[i][0], &product_id, sizeof(int)) > 0)
      {
        order_response response = {0, 0.0};

        catalog[product_id].order_requests++;

        if (catalog[product_id].item_count > 0)
        {
          catalog[product_id].item_count--;
          catalog[product_id].items_sold++;
          successful_orders++;
          total_revenue += catalog[product_id].price;

          response.success = 1;
          response.price = catalog[product_id].price;
        }

        write(response_pipes[i][1], &response, sizeof(order_response));
        orders_processed++;

        // Processing time
        sleep(1);
      }
    }
  }

  int products_requested = 0;
  int products_bought = 0;

  for (int i = 0; i < NUM_PRODUCTS; i++)
  {
    if (catalog[i].order_requests > 0)
    {
      products_requested++;
    }
    if (catalog[i].items_sold > 0)
    {
      products_bought++;
    }
  }

  // Print final report
  printf("Final Report:\n\n");

  for (int i = 0; i < NUM_PRODUCTS; i++)
  {
    printf("Product #%d:\n", i);
    printf("  Description: %s\n", catalog[i].description);
    printf("  Order requests: %d\n", catalog[i].order_requests);
    printf("  Items sold: %d\n", catalog[i].items_sold);
  }

  printf("Summary:\n");
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