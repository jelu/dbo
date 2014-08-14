# libdbo

C library for database access using an object based abstraction layer

## About

This is a semi abstracted database layer meaning that the backend
implementations used by the layer and the objects using the layer actually know
a thing or two about each other. The layer is as abstract as it needs to be to
be able to support multiple backends that operates in completely different ways.

The basic idea is that you have database objects that represents the data model
of the database, these objects are used by the application logic and the object
them selves uses the database layer to describe them. The database layer is more
or less just a container for the description of the database objects and a
transporter for the information to the backend in use. The backend will perform
the operations coming from the database objects and deliver the result of them
using the classes of the database layer and that information will be interpreted
by the database objects and converted to be presentable as normal c value type
for the logic.


## Design

    +------------------------------+
    |         Application          |
    +------------------------------+
    |       Database Objects       |
    +------------------------------+
    |        Database Layer        |
    +------------------------------+
    |      Database Backends       |
    +------------------------------+

### Application

Application is the application/logic code that uses database objects to access
the data in the database.

### Database Objects

Database objects encapsulate the data from the database using the database
layer and provides functions for accessing and modifying the data. There are
also function for retrieving object from the database based on fields and
related objects.

### Database Layer

Database layer provides objects for encapsulating database values and for
describing database operations such as a clause list which contains a list of
fields, an operation and a value. CRUD operations can be executed through a
database connect to an underlying database backend.

### Database Backends

Database backends executes the given operations according to how that specific
database backend handles data.


## Object Revisions

All objects have a revision or something similar and when updating/deleting it
will use the revision to verify that the object has not been updated by anyone
else. If it has then the operation will fail with an error and the task that
performed the operation should be rerun immediately or rescheduled.
 

## Backend Support

Driver     | Support Status
-----------|---------------------
SQLite     | supported and tested
MySQL      | supported and tested
PostgreSQL | wip
CouchDB    | experimental support
LDAP       | wip
MongoDB    | wip


## Object/Module Descriptions

All following modules are documented in HTML or man(7).

### libdbo_configuration

Object holding a configuration value as a key and value, used for configuring
database connection and backends.

### libdbo_connection

Object holding a connection, used for connect/disconnect'ing to a backend and
handling transactions.

### libdbo_object_field

Object holding the definition of a database object field, used for describing
all the fields in a database object.

### libdbo_object

Object holding the definition of a database object and used for create/read/
update/delete the database object.

### libdbo_backend_handle

Object holding functions to a database backend, used by libdbo_backend.

### libdbo_backend

Object holding a backend, used by libdbo_connection.

### libdbo_join

SUBJECT FOR DEPRECATION!

Object holding the definition of a join, used to describe all joins in a query,
used in read/update and delete.

### libdbo_clause

Object holding the definition of a clause, used to describe all the clauses in
a query, used in read/update/delete.

### libdbo_result

Object holding the result of a row from a query, used to hold all the result
from a query.

### libdbo_value

Object holding a database value.

### libdbo_value_set

Object holding a fixed set of database values.

### libdbo_type

An enum describing all the different database value types.


## TODO

- Maybe remove clause for update/delete and set object id in libdbo_object_t
- Re-evaluate db backend meta data.
- Extend object with db value for id and use that in update/delete if no clause list is given.
- Create templates for setting objects by foreign key.
- Generate basic object cache for database objects.
