#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
  int i = 0;
  for (; i < dbhdr->count; i++) {
    printf("Employee %d:\n", i + 1);
    printf("  Name: %s\n", employees[i].name);
    printf("  Address: %s\n", employees[i].address);
    printf("  Hours: %u\n", employees[i].hours);
  }
  return;
}

int update_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *updatestring) {
  if (NULL == dbhdr) return STATUS_ERROR;
  if (NULL == employees) return STATUS_ERROR;
  if (NULL == *employees) return STATUS_ERROR;
  if (NULL == updatestring) return STATUS_ERROR;

  char *name = strtok(updatestring, ",");
  if (NULL == name) return STATUS_ERROR;
  char *address = strtok(NULL, ",");
  if (NULL == address) return STATUS_ERROR;
  char *hoursStr = strtok(NULL, ",");
  if (NULL == hoursStr) return STATUS_ERROR;

  int i = 0;
  for (; i < dbhdr->count; i++) {
    if (strcmp((*employees)[i].name, name) == 0) {
      strncpy((*employees)[i].address, address, sizeof((*employees)[i].address) - 1);
      (*employees)[i].hours = atoi(hoursStr);
      break;
    }
  }

  return STATUS_SUCCESS;
}

int remove_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *removestring) {
  if (NULL == dbhdr) return STATUS_ERROR;
  if (NULL == employees) return STATUS_ERROR;
  if (NULL == *employees) return STATUS_ERROR;
  if (NULL == removestring) return STATUS_ERROR;

  char *name = strtok(removestring, ",");
  if (NULL == name) return STATUS_ERROR;

  bool resized = false;
  int i = 0;
  for (; i < dbhdr->count; i++) {
    if (strcmp((*employees)[i].name, name) == 0) {
      int j = i;
      for (; j < dbhdr->count - 1; j++) {
        (*employees)[j] = (*employees)[j + 1];
      }
      dbhdr->count--;
      resized = true;
    }
  }
  if (resized) {
    struct employee_t *e = realloc(*employees, dbhdr->count * sizeof(struct employee_t));
    if (e == NULL && dbhdr->count > 0) {
      perror("realloc");
      return STATUS_ERROR;
    }
    *employees = e;
  }
  return STATUS_SUCCESS;
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *addstring) {
  if (NULL == dbhdr) return STATUS_ERROR;
  if (NULL == employees) return STATUS_ERROR;
  if (NULL == *employees) return STATUS_ERROR;
  if (NULL == addstring) return STATUS_ERROR;

  char *name = strtok(addstring, ",");
  if (NULL == name) return STATUS_ERROR;
  char *address = strtok(NULL, ",");
  if (NULL == address) return STATUS_ERROR;
  char *hoursStr = strtok(NULL, ",");
  if (NULL == hoursStr) return STATUS_ERROR;

  struct employee_t *e = *employees;
  e = realloc(e, (dbhdr->count + 1) * sizeof(struct employee_t));
  if (e == NULL) {
    perror("realloc");
    return STATUS_ERROR;
  }

  dbhdr->count++;

  strncpy(e[dbhdr->count-1].name, name, sizeof(e[dbhdr->count-1].name) - 1);
  strncpy(e[dbhdr->count-1].address, address, sizeof(e[dbhdr->count-1].address) - 1);
  e[dbhdr->count-1].hours = atoi(hoursStr);

  *employees = e;
  return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
  if (fd < 0) {
    fprintf(stderr, "Error: Invalid file descriptor.\n");
    return STATUS_ERROR;
  }

  int count = dbhdr->count;

  struct employee_t *employees = calloc(count, sizeof(struct employee_t));
  if (employees == NULL) {
    perror("calloc");
    return STATUS_ERROR;
  }

  read(fd, employees, count * sizeof(struct employee_t));
  int i = 0;
  for (i = 0; i < count; i++) {
    employees[i].hours = ntohl(employees[i].hours);
  }

  *employeesOut = employees;
  return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
  if (fd < 0) {
    fprintf(stderr, "Error: Invalid file descriptor.\n");
    return STATUS_ERROR;
  }
  int employee_count = dbhdr->count;

  dbhdr->magic = htonl(dbhdr->magic);
  dbhdr->version = htons(dbhdr->version);
  dbhdr->count = htons(dbhdr->count);
  dbhdr->filesize = htonl(dbhdr->filesize) + employee_count * sizeof(struct employee_t);
  if (lseek(fd, 0, SEEK_SET) == -1) {
    perror("lseek");
    return STATUS_ERROR;
  }
  if (write(fd, dbhdr, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
    perror("write");
    return STATUS_ERROR;
  }
  int i = 0;
  for (i = 0; i < employee_count; i++) {
    employees[i].hours = htonl(employees[i].hours);
    write(fd, &employees[i], sizeof(struct employee_t));
  }
  return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
  if (fd < 0) {
    fprintf(stderr, "Error: Invalid file descriptor.\n");
    return STATUS_ERROR;
  }
  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (header == NULL) {
    perror("calloc");
    return STATUS_ERROR;
  }
  if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
    perror("read");
    free(header);
    return STATUS_ERROR;
  }
  header->magic = ntohl(header->magic);
  header->version = ntohs(header->version);
  header->count = ntohs(header->count);
  header->filesize = ntohl(header->filesize);

  if (header->magic != HEADER_MAGIC) {
    fprintf(stderr, "Error: Invalid database file (bad magic number).\n");
    free(header);
    return STATUS_ERROR;
  }
  if (header->version != 1) {
    printf("Improper header version\n");
    free(header);
    return STATUS_ERROR;
  }
  // if (header->count < 0 || header->count > MAX_EMPLOYEES)

  // if (header->filesize < sizeof(struct dbheader_t) || header->filesize > MAX_FILESIZE)

  *headerOut = header;

  return STATUS_SUCCESS;
}

int create_db_header(/*int fd,*/ struct dbheader_t **headerOut) {
  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (header == NULL) {
    perror("calloc");
    return STATUS_ERROR;
  }
  header->version = 0x1;
  header->count = 0;
  header->magic = HEADER_MAGIC;
  header->filesize = sizeof(struct dbheader_t);

  *headerOut = header;

  return STATUS_SUCCESS;
}
