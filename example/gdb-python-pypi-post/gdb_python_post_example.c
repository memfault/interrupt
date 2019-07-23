//! A boring singly linked list where each node holds a UUID used as an example of GDB Python APIs
//!
//! To compile, you can copy & paste the code into your favorite embedded development board or if you
//! gcc/gdb installed for desktop development you can also run everything locally, i.e:
//!
//! gcc -g3 gdb_python_post_example.c -o gdb_python_example
//! gdb gdb_python_example --ex="source custom_pretty_printers.py" --ex="break generate_fake_uuid_list" --ex="run" --ex="finish"

#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct Uuid {
  uint8_t bytes[16];
} Uuid;

typedef struct UuidListNode {
  struct UuidListNode *next;
  Uuid uuid;
} UuidListNode;

static UuidListNode *s_list_head = NULL;

void list_add_uuid(const Uuid *uuid) {
  UuidListNode *node_to_add = malloc(sizeof(UuidListNode));
  if (node_to_add == NULL) {
    return;
  }

  *node_to_add = (UuidListNode) {
    .uuid = *uuid,
  };

  // Add entry to the front of the list
  node_to_add->next = s_list_head;
  s_list_head = node_to_add;
}

//
// Code to populate the list with fake data
//

// A completely fake uuid-generator implementation
static void generate_fake_uuid(Uuid *uuid) {
  for (size_t i = 0; i < sizeof(*uuid); i++) {
    uuid->bytes[i] = rand() & 0xff;
  }
}

static void generate_fake_uuid_list(void) {
  for (int i = 0; i < 10; i++) {
    Uuid uuid;
    generate_fake_uuid(&uuid);
    list_add_uuid(&uuid);
  }
}

int main(void) {
  generate_fake_uuid_list();
  // Let's just spin loop so we can break at any point and dump the list
  while (1) { }
  return 0;
}
