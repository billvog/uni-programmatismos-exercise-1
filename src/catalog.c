#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "catalog.h"
#include "definitions.h"

void initialize_catalog(product_t *catalog)
{
  char desc[50];
  for (int i = 0; i < NUM_PRODUCTS; i++)
  {
    sprintf(desc, "Product %d", i + 1);
    catalog[i].description = strdup(desc);
    catalog[i].price = 10.0 + (rand() % 90);
    catalog[i].item_count = INITIAL_STOCK;
    catalog[i].order_requests = 0;
    catalog[i].items_sold = 0;
  }
}