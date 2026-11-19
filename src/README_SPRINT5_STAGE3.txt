LuraXDB - Sprint 5 Stage 3: Multiple-Column GROUP BY
====================================================

Files in this package
---------------------
SQLParser.c
SQLParser.h
TableRecordManager.c
TableRecordManager.h
Command.c

Implemented
-----------
1. GROUP BY supports ID, VALUE, or both columns.
2. Grouped SELECT projection supports:
   SELECT id, COUNT(*) ... GROUP BY id
   SELECT value, COUNT(*) ... GROUP BY value
   SELECT id, value, COUNT(*) ... GROUP BY id, value
3. Grouped aggregates continue to support COUNT, SUM, AVG, MIN and MAX.
4. WHERE filtering continues to work with grouped queries.
5. ORDER BY can sort by either grouping column when both columns are grouped.
6. Existing one-column GROUP BY queries remain supported.
7. Existing table-record file layout is unchanged; no migration is required.

Important current limitations
-----------------------------
- This storage model has only ID and VALUE columns. GROUP BY therefore supports
  only these two existing columns; it does not add department/salary/etc.
- At most two grouping columns are supported.
- For this stage, the SELECT grouping-column order must match the GROUP BY
  column order (for example SELECT id, value ... GROUP BY id, value).
- ORDER BY supports one grouping column at a time.
- SUM and AVG remain limited to numeric ID values, matching prior behavior.
- This stage does not add LIMIT to grouped queries.

Test SQL
--------
SELECT id, value, COUNT(*) FROM employees GROUP BY id, value;
SELECT id, value, COUNT(*) FROM employees GROUP BY id, value ORDER BY value DESC;
SELECT id, value, SUM(id) FROM employees WHERE id BETWEEN '101' AND '103' GROUP BY id, value ORDER BY id ASC;
SELECT value, COUNT(*) FROM employees GROUP BY value ORDER BY value ASC;
SELECT id, COUNT(*) FROM employees GROUP BY id ORDER BY id DESC;
SELECT value, AVG(id) FROM employees GROUP BY value ORDER BY value ASC;

Build command (run from the project root)
------------------------------------------
gcc -std=c17 -Wall -Wextra -Wpedantic src\Main.c src\Command.c src\Database.c src\storage.c src\PageManager.c src\BufferPool.c src\RecordManager.c src\SQLParser.c src\TableManager.c src\TableRecordManager.c -o Main.exe

Validation performed while preparing this package
--------------------------------------------------
- SQLParser.c compiled with GCC C17 using -Wall -Wextra -Wpedantic.
- TableRecordManager.c compiled with GCC C17 using -Wall -Wextra -Wpedantic
  against minimal dependency stubs because the other project headers were not
  included in this upload.
- Parser-to-group-manager integration harness passed for a two-column GROUP BY
  query and an existing one-column GROUP BY query.
- The complete Windows project build was not run because the remaining project
  source/header files were not part of this upload.
