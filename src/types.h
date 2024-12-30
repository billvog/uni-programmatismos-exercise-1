#pragma once

typedef struct product
{
  char *description;
  float price;
  unsigned int item_count;
  // Add tracking fields for the final report
  unsigned int order_requests;
  unsigned int items_sold;
} product_t;

typedef struct order_response
{
  int success;
  float price;
} order_response_t;