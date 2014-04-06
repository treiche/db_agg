db_agg
======

join multiple datasources with a single query

db_agg is a commandline tool which processes a single postgres sql query consisting of multiple
common table expressions (WITH statements) that are referring to *different* databases.

Requirements
------------

* c++11 compatible compiler
* libpq
* pcre
* libxml2
* jansson
* libzip

Motivation
----------

comparing data across multiple databases can be cumbersome especially when sharding and multiple test of production environments
comes in play. 
most of the time this work consists of well defined steps that can be easily automatized.

db_agg only requires a single query and determines the concrete databases it needs to talk to, resolves the
dependencies that are needed to join the query results, and caches the results for a short development roundtrip.

Installation
------------
db_agg useds standard gnu autotools build system, so the single steps for installation are:
* ./configure
* make
* make install

Configuration
-------------
before db_agg can be used it needs to know where it can find databases to connect to.
this is configured in a file called database-registry.xml located in ~/.db_agg/etc.
a very simple example could look like this:
```xml
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE registry PUBLIC "database-registry.dtd" "/usr/local/db_agg/etc/database-registry.dtd">
<registry>
    <system name="local">
        <host name="host1">
            <server port="5432">
                <database-instance id="db1"/>
            </server>
        </host>
        <host name="host2">
            <server port="5432">
                <database-instance id="db2"/>
            </server>
        </host>
    </system>
    <database-definition name="db1">
        <namespace name="ns1"/>
    </database-definition>
    <database-definition name="db2">
        <namespace name="ns2"/>
    </database-definition>
</registry>
``` 
this tells db_agg to connect to host1 on port 5432 when it finds the namespace ns1 in a sub query.

Usage
-----
with the configuration given above db_agg could be used for a query like this:
```sql
with data_on_db1 as (
    select *
      from ns1.data
)
    select *
      from data_on_db1
      join ns2.data
        on column_x = column_y
```
with this query saved in a file test_query.sql db_agg could be run on the command line
using the follwing line:
```sh
/usr/local/bin/db_agg -e local test_query.sql
```
the results of running db_agg can be then found in the working directory under
./test_query/local
