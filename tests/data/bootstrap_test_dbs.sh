#!/bin/sh

rm tests/data/local_unsharded_db.sqlite3

sqlite3 tests/data/local_unsharded_db.sqlite3 <<SQL

    drop table if exists article;
    
    create table article (
        code text,
        name text
    );
    
    insert
      into article
    values
        ('JEA', 'Jeans'),
        ('TSH', 'T-Shirt'),
        ('SHO', 'Shoes'),
        ('SCA', 'Scarf'),
        ('RIN', 'Ring'),
        ('BAL', 'Ball'),
        ('PAN', 'Pants'),
        ('HAT', 'Hat')
    ;
SQL

rm tests/data/local_sharded?_db.sqlite3

sqlite3 tests/data/local_sharded1_db.sqlite3 <<SQL

    drop table if exists price;
    
    create table price (
        article_code text,
        price float
    );
    
    insert
      into price
    values
        ('SHO', 10.3)
    ;
SQL

sqlite3 tests/data/local_sharded2_db.sqlite3 <<SQL

    drop table if exists price;
    
    create table price (
        article_code text,
        price float
    );
    
    /*
    insert
      into price
    values
        ('BR', 10.3),
        ('BI', 5.5 )
    ;
    */
SQL

sqlite3 tests/data/local_sharded3_db.sqlite3 <<SQL

    drop table if exists price;
    
    create table price (
        article_code text,
        price float
    );
    
    insert
      into price
    values
        ('JEA', 10.3),
        ('TSH', 5.5 ),
        ('HAT', 9.99 )
    ;
SQL

sqlite3 tests/data/local_sharded4_db.sqlite3 <<SQL

    drop table if exists price;
    
    create table price (
        article_code text,
        price float
    );
    
    insert
      into price
    values
        ('SCA', 10.34),
        ('RIN', 5.70 ),
        ('BAL', 15.70 ),
        ('PAN', 25.70 )
    ;
SQL
