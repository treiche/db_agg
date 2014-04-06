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
db_agg used standard gnu autotools build system, so the single steps for installation are:
* ./configure
* make
* make install

Usage
-----

run db_agg -h to get help about the command line options

Examples
--------
db_agg -e local -S myquery.sql