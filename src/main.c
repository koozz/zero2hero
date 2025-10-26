#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
  printf("Usage: %s [-n] -f <database>\n", argv[0]);
  printf("  -n            Create a new database file\n");
  printf("  -f <database> Path to the database file\n");
  return;
}

int main(int argc, char *argv[]) {
  bool newfile = false;
  char *filepath = NULL;
  char *addstring = NULL;

  int dbfh = -1;
  struct dbheader_t *dbhdr = NULL;
  struct employee_t *employees = NULL;

  int c = 0;

  while ((c = getopt(argc, argv, "nf:a:")) != -1) {
    switch(c) {
      case 'n':
        newfile = true;
        break;

      case 'f':
        filepath = optarg;
        break;

      case 'a':
        addstring = optarg;
        break;

      case '?':
        printf("Unknown option: %c\n", optopt);
        print_usage(argv);
        return -1;

      default:
        return -1;
    }
  }

  if (filepath == NULL) {
    printf("Error: Database file path is required.\n");
    print_usage(argv);
    return -1;
  }

  if (newfile) {
    dbfh = create_db_file(filepath);
    if (dbfh == STATUS_ERROR) {
      printf("Error creating database file.\n");
      return -1;
    }
    if (create_db_header(/*dbfh,*/ &dbhdr) == STATUS_ERROR) {
      printf("Error creating database header.\n");
      return -1;
    }
  } else {
    dbfh = open_db_file(filepath);
    if (dbfh == STATUS_ERROR) {
      printf("Error opening database file.\n");
      return -1;
    }
    if (validate_db_header(dbfh, &dbhdr) == STATUS_ERROR) {
      printf("Error validating database header.\n");
      return -1;
    }
  }

  if (read_employees(dbfh, dbhdr, &employees) != STATUS_SUCCESS) {
    printf("Error reading employees from database file.\n");
    return -1;
  }
  if (addstring != NULL) {
    if (add_employee(dbhdr, &employees, addstring) != STATUS_SUCCESS) {
      printf("Error adding employee to database file.\n");
      return -1;
    }
  }

  output_file(dbfh, dbhdr, employees);
  close(dbfh);
  return 0;
}
