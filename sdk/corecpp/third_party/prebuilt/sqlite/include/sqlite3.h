/*
** SQLite3 Header for LeafraSDK
** This is a simplified header containing essential SQLite API declarations
** Version: 3.49.2
*/

#ifndef SQLITE3_H
#define SQLITE3_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** SQLite Version Information
*/
#define SQLITE_VERSION        "3.49.2"
#define SQLITE_VERSION_NUMBER 3049002

/*
** Result Codes
*/
#define SQLITE_OK           0   /* Successful result */
#define SQLITE_ERROR        1   /* Generic error */
#define SQLITE_INTERNAL     2   /* Internal logic error in SQLite */
#define SQLITE_PERM         3   /* Access permission denied */
#define SQLITE_ABORT        4   /* Callback routine requested an abort */
#define SQLITE_BUSY         5   /* The database file is locked */
#define SQLITE_LOCKED       6   /* A table in the database is locked */
#define SQLITE_NOMEM        7   /* A malloc() failed */
#define SQLITE_READONLY     8   /* Attempt to write a readonly database */
#define SQLITE_INTERRUPT    9   /* Operation terminated by sqlite3_interrupt()*/
#define SQLITE_IOERR       10   /* Some kind of disk I/O error occurred */
#define SQLITE_CORRUPT     11   /* The database disk image is malformed */
#define SQLITE_NOTFOUND    12   /* Unknown opcode in sqlite3_file_control() */
#define SQLITE_FULL        13   /* Insertion failed because database is full */
#define SQLITE_CANTOPEN    14   /* Unable to open the database file */
#define SQLITE_PROTOCOL    15   /* Database lock protocol error */
#define SQLITE_EMPTY       16   /* Internal use only */
#define SQLITE_SCHEMA      17   /* The database schema changed */
#define SQLITE_TOOBIG      18   /* String or BLOB exceeds size limit */
#define SQLITE_CONSTRAINT  19   /* Abort due to constraint violation */
#define SQLITE_MISMATCH    20   /* Data type mismatch */
#define SQLITE_MISUSE      21   /* Library used incorrectly */
#define SQLITE_NOLFS       22   /* Uses OS features not supported on host */
#define SQLITE_AUTH        23   /* Authorization denied */
#define SQLITE_FORMAT      24   /* Not used */
#define SQLITE_RANGE       25   /* 2nd parameter to sqlite3_bind out of range */
#define SQLITE_NOTADB      26   /* File opened that is not a database file */
#define SQLITE_NOTICE      27   /* Notifications from sqlite3_log() */
#define SQLITE_WARNING     28   /* Warnings from sqlite3_log() */
#define SQLITE_ROW         100  /* sqlite3_step() has another row ready */
#define SQLITE_DONE        101  /* sqlite3_step() has finished executing */

/*
** Data Types
*/
#define SQLITE_INTEGER  1
#define SQLITE_FLOAT    2
#define SQLITE_BLOB     4
#define SQLITE_NULL     5
#define SQLITE_TEXT     3

/*
** Open Flags
*/
#define SQLITE_OPEN_READONLY         0x00000001  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_READWRITE        0x00000002  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_CREATE           0x00000004  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_DELETEONCLOSE    0x00000008  /* VFS only */
#define SQLITE_OPEN_EXCLUSIVE        0x00000010  /* VFS only */
#define SQLITE_OPEN_AUTOPROXY        0x00000020  /* VFS only */
#define SQLITE_OPEN_URI              0x00000040  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_MEMORY           0x00000080  /* Ok for sqlite3_open_v2() */

/*
** Forward Declarations
*/
typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;

/*
** Core API Functions
*/

/* Database Connection */
int sqlite3_open(const char *filename, sqlite3 **ppDb);
int sqlite3_open_v2(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs);
int sqlite3_close(sqlite3 *db);
int sqlite3_close_v2(sqlite3 *db);

/* Error Handling */
int sqlite3_errcode(sqlite3 *db);
int sqlite3_extended_errcode(sqlite3 *db);
const char *sqlite3_errmsg(sqlite3 *db);
const char *sqlite3_errstr(int rc);

/* SQL Execution */
int sqlite3_exec(sqlite3 *db, const char *sql, int (*callback)(void*,int,char**,char**), void *arg, char **errmsg);

/* Prepared Statements */
int sqlite3_prepare_v2(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
int sqlite3_step(sqlite3_stmt *pStmt);
int sqlite3_finalize(sqlite3_stmt *pStmt);
int sqlite3_reset(sqlite3_stmt *pStmt);

/* Parameter Binding */
int sqlite3_bind_blob(sqlite3_stmt *pStmt, int i, const void *zData, int nData, void(*xDel)(void*));
int sqlite3_bind_double(sqlite3_stmt *pStmt, int i, double rValue);
int sqlite3_bind_int(sqlite3_stmt *pStmt, int i, int iValue);
int sqlite3_bind_int64(sqlite3_stmt *pStmt, int i, long long iValue);
int sqlite3_bind_null(sqlite3_stmt *pStmt, int i);
int sqlite3_bind_text(sqlite3_stmt *pStmt, int i, const char *zData, int nData, void(*xDel)(void*));
int sqlite3_bind_parameter_count(sqlite3_stmt *pStmt);
int sqlite3_bind_parameter_index(sqlite3_stmt *pStmt, const char *zName);

/* Result Column Access */
const void *sqlite3_column_blob(sqlite3_stmt *pStmt, int iCol);
int sqlite3_column_bytes(sqlite3_stmt *pStmt, int iCol);
int sqlite3_column_count(sqlite3_stmt *pStmt);
double sqlite3_column_double(sqlite3_stmt *pStmt, int iCol);
int sqlite3_column_int(sqlite3_stmt *pStmt, int iCol);
long long sqlite3_column_int64(sqlite3_stmt *pStmt, int iCol);
const unsigned char *sqlite3_column_text(sqlite3_stmt *pStmt, int iCol);
int sqlite3_column_type(sqlite3_stmt *pStmt, int iCol);
const char *sqlite3_column_name(sqlite3_stmt *pStmt, int N);

/* Database Changes */
int sqlite3_changes(sqlite3 *db);
int sqlite3_total_changes(sqlite3 *db);
long long sqlite3_last_insert_rowid(sqlite3 *db);

/* Utility Functions */
void sqlite3_free(void *p);
char *sqlite3_mprintf(const char *zFormat, ...);
const char *sqlite3_libversion(void);
int sqlite3_libversion_number(void);

/* Transaction Control */
int sqlite3_get_autocommit(sqlite3 *db);

/* Memory Management */
#define SQLITE_STATIC      ((void(*)(void *))0)
#define SQLITE_TRANSIENT   ((void(*)(void *))-1)

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif /* SQLITE3_H */ 