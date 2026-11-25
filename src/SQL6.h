#ifndef LURAXDB_SQL6_H
#define LURAXDB_SQL6_H

/* Sprint 6 schema-based SQL extension. Returns 1 when handled, 0 for legacy SQL. */
int sql6_initialize(void);
int sql6_execute(const char *sql);

#endif
